import java.io.*;
import java.nio.*;
import javax.swing.*;
import java.awt.*;
import java.awt.image.*;

import org.naokishibata.baum.*;
import org.naokishibata.ocvimgio.*;

public class BaumCamTest {
    final int width, height;
    final JFrame frame;
    final JPanel panel;
    final BufferedImage bImg;
    final Graphics2D g;

    BaumCamTest(int width, int height) {
	this.width = width; this.height = height;
	bImg = new BufferedImage(width, height, BufferedImage.TYPE_3BYTE_BGR);

	panel = new JPanel() {
		public void paintComponent(Graphics gx) {
		    super.paintComponent(gx);
		    gx.drawImage(bImg, 0, 0, null);
		}
	    };

	panel.setPreferredSize(new Dimension(width, height));
        frame = new JFrame("BaumCamTest.java");
        frame.setResizable(false);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.getContentPane().add("Center", panel);
        frame.pack();
	frame.setLocation((int)((Toolkit.getDefaultToolkit().getScreenSize().getWidth()  - frame.getWidth ())/2),
			  (int)((Toolkit.getDefaultToolkit().getScreenSize().getHeight() - frame.getHeight())/2));
        frame.setVisible(true);

	g = bImg.createGraphics();
    }
    
    void execute(int did, String fnin, String fnout) throws Exception {
	CvCapture c0 = null;
	CvVideoWriter vw = null;
	
	if (fnin == null) {
	    c0 = CvCapture.cvCreateCameraCapture(0);
	    c0.cvSetCaptureProperty(CvCapture.CV_CAP_PROP_FRAME_WIDTH , 1024);
	    c0.cvSetCaptureProperty(CvCapture.CV_CAP_PROP_FRAME_HEIGHT,  768);
	} else {
	    c0 = CvCapture.cvCreateFileCapture(fnin);
	}
	
	IplImage iimg = c0.cvQueryFrame(), oimg = null;
	c0.cvGrabFrame();
	int iw = iimg.width(), ih = iimg.height(), resultPage = 0;

	Baum baum = new Baum(did, iw, ih);
	Baum.Result[] result = new Baum.Result[] { baum.createResult(1024), baum.createResult(1024) };
	ByteBuffer pinnedMem = baum.malloc(iw * ih * 3);
	BufferedImage image = iimg.getImage();
	
	System.out.println(baum.getDeviceName());
		
	if (baum.loadPlan("BaumTestPlan.txt") != 0) System.out.println("Could not load a plan.");

	if (fnout != null) {
	    vw = CvVideoWriter.cvCreateVideoWriter(fnout, "PIM1", 30, width, height, 1);
	    oimg = new IplImage(width, height);
	}

	baum.enqueueTask(result[resultPage++ & 1], iimg.getDirect(), iw, ih);
	
	for(;;) {
	    image = iimg.getImage();

	    iimg = c0.cvRetrieveFrame();
	    c0.cvGrabFrame();
	    if (iimg == null) break;

	    pinnedMem.rewind(); pinnedMem.put(iimg.getDirect());
	    baum.enqueueTask(result[resultPage++ & 1], pinnedMem, iw, ih);

	    baum.poll(true);

	    int nDecode = result[resultPage & 1].size();
	    g.setColor(Color.red);

	    g.drawImage(image, 0, 0, null);
	    
	    for(int i=0;i<nDecode;i++) {
		float[] d = result[resultPage & 1].get(i);
		g.drawLine((int)d[0], (int)d[1], (int)d[2], (int)d[3]);
		g.drawString("" + (int)d[4], (int)(d[0] + d[2])/2, (int)(d[1] + d[3])/2);
	    }

	    if (vw != null) {
		oimg.setImage(bImg);
		vw.cvWriteFrame(oimg);
	    }

	    panel.repaint();
	}

	baum.free(pinnedMem);
	result[0].dispose();
	result[1].dispose();
	baum.dispose();
	if (vw != null) vw.close();
    }

    public static void main(String[] args) throws Exception {
	if (args.length == 0) {
	    System.out.println("Usage : java BaumCamTest <device id> [<input movie file>] [<output movie file>]");
	    System.out.println("Available devices :");
	    for(int i=0;;i++) {
		String s = Baum.getDeviceName(i);
		if (s == null) break;
		System.out.println("Device ID " + i + " : " + s);
	    }
	    System.exit(-1);
	}

	BaumCamTest instance = new BaumCamTest(1024, 768);
	String fnin = null, fnout = null;
	
	int did = Integer.parseInt(args[0]);
	if (args.length >= 2 && !args[1].equals("-")) fnin  = args[1];
	if (args.length >= 3 && !args[2].equals("-")) fnout = args[2];

	instance.execute(did, fnin, fnout);
	System.exit(0);
    }
}
