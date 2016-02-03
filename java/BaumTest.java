import java.io.*;
import java.nio.*;
import java.awt.*;
import java.awt.image.*;
import javax.imageio.*;

import org.naokishibata.baum.*;

public class BaumTest {
    public static void main (String[] args) throws Exception {
	System.setProperty("java.awt.headless", "true"); 

	if (args.length < 2) {
	    System.out.println("Usage : java BaumTest <device id> <image file>");
	    System.out.println("Available devices :");
	    for(int i=0;;i++) {
		String s = Baum.getDeviceName(i);
		if (s == null) break;
		System.out.println("Device ID " + i + " : " + s);
	    }
	    System.exit(-1);
	}
	
	BufferedImage bimg = ImageIO.read(new File(args[1]));
	int iw = bimg.getWidth(), ih = bimg.getHeight();
	
	Baum baum = new Baum(Integer.parseInt(args[0]), iw, ih);
	Baum.Result result = baum.createResult(1024);

	System.out.println(baum.getDeviceName());

	ByteBuffer bb = baum.malloc(iw * ih * 3);

	Baum.bgrimg2bb(bb, bimg);

	if (baum.loadPlan("BaumTestPlan.txt") != 0) {
	    System.out.println("creating plan");
	    baum.createPlan("BaumTestPlan.txt", result, bb, iw, ih);
	}

	baum.enqueueTask(result, bb, iw, ih);

	baum.poll(true);

	int nDecode = result.size();

	Graphics2D g = bimg.createGraphics();
	g.setColor(Color.red);

	for(int i=0;i<nDecode;i++) {
	    float[] d = result.get(i);
	    g.drawLine((int)d[0], (int)d[1], (int)d[2], (int)d[3]);
	    g.drawString("" + (int)d[4], (int)(d[0] + d[2])/2, (int)(d[1] + d[3])/2);
	}

	ImageIO.write(bimg, "png", new File("output.png"));

	baum.free(bb);
	baum.dispose();
    }
}
