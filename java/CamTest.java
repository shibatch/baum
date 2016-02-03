import javax.swing.*;
import java.awt.*;
import java.awt.image.*;

import org.naokishibata.ocvimgio.*;

public class CamTest {
    final int width, height;
    final JFrame frame;
    final JPanel panel;
    BufferedImage bImg;
    Graphics2D g;

    CamTest(int width, int height) {
	this.width = width;
	this.height = height;

	bImg = new BufferedImage(width, height, BufferedImage.TYPE_3BYTE_BGR);

	panel = new JPanel() {
		public void paintComponent(Graphics g) {
		    super.paintComponent(g);
		    g.drawImage(bImg, 0, 0, null);
		}
	    };

	panel.setPreferredSize(new Dimension(width, height));

        frame = new JFrame("CamTest.java");

        frame.setResizable(false);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.getContentPane().add("Center", panel);
        frame.pack();
	frame.setLocation((int)((Toolkit.getDefaultToolkit().getScreenSize().getWidth()  - frame.getWidth ())/2),
			  (int)((Toolkit.getDefaultToolkit().getScreenSize().getHeight() - frame.getHeight())/2));

        frame.setVisible(true);

	g = bImg.createGraphics();
    }
    
    void execute(String fnin, String fnout) throws Exception {
	CvCapture c0 = null;
	CvVideoWriter vw = null;
	
	if (fnin == null) {
	    c0 = CvCapture.cvCreateCameraCapture(0);
	    c0.cvSetCaptureProperty(CvCapture.CV_CAP_PROP_FRAME_WIDTH , 1024);
	    c0.cvSetCaptureProperty(CvCapture.CV_CAP_PROP_FRAME_HEIGHT,  768);
	} else {
	    c0 = CvCapture.cvCreateFileCapture(fnin);
	}
	
	IplImage iimg = c0.cvQueryFrame();
	c0.cvGrabFrame();
	int iw = iimg.width(), ih = iimg.height();

	if (fnout != null) vw = CvVideoWriter.cvCreateVideoWriter(fnout, "PIM1", 30, iw, ih, 1);

	for(;;) {
	    if (vw != null) vw.cvWriteFrame(iimg);
	    
	    iimg = c0.cvRetrieveFrame();
	    c0.cvGrabFrame();
	    if (iimg == null) break;

	    g.drawImage(iimg.getImage(), 0, 0, null);
	    panel.repaint();
	}

	if (vw != null) vw.close();
    }

    public static void main(String[] args) throws Exception {
	System.out.println("OpenCV version : " + CvUtil.versionString());

	CamTest instance = new CamTest(1024, 768);
	String fnin = null, fnout = null;
	if (args.length >= 1 && !args[0].equals("-")) fnin = args[0];
	if (args.length >= 2) fnout = args[1];

	instance.execute(fnin, fnout);

	System.exit(0);
    }
}
