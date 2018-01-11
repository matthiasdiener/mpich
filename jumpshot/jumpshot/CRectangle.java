package jumpshot;

import java.awt.*;

//this class is a superclass of both ProcessState and JProcess as essentially
//both of these are rectangles
public class CRectangle
{
  public Dimension dim;     //Dimension of the rectangle to be drawn
  public Point pt;          //coordinate to draw the rectangle from 
  public Color col;         //color to fill the rectangle with 

  //Constructor
  public CRectangle (Point w, Dimension x, Color y) {
    pt = w; dim = x; col = y;
  }

  //draw a filled rectangle in the specified graphics context
  //g = Graphics context to draw in
  public void displayRect (Graphics g) {
    g.setColor (col);
    int w = (dim.width == 0)? 1 : dim.width;
    int h = (dim.height == 0)? 1 : dim.height; 
    g.fillRect (pt.x, pt.y, w, h);
  }  
  
  //draw a filled rectangle in the specified graphics context along with a
  //white border
  //g = Graphics context to draw in
  //startflag = specifies whether the vertical line of the border is to be drawn
  //            at the start of the rectangle [see MyImage]
  //endflag = specifies whether the vertical line of the border is to be drawn
  //          at the end of the rectangle. [see MyImage]
  public void displayEtRect (Graphics g, boolean startflag, boolean endflag) {
    displayRect (g);
    g.setColor (Color.white);
    int w = (dim.width == 0)? 1 : dim.width;
    int h = (dim.height == 0)? 1 : dim.height;
    g.drawLine (pt.x, pt.y, pt.x + w, pt.y);
    g.drawLine (pt.x, pt.y + h, pt.x + w, pt.y + h);
    if (startflag) g.drawLine (pt.x, pt.y, pt.x, pt.y + h);
    if (endflag) g.drawLine (pt.x + w, pt.y, pt.x + w, pt.y + h);
  }
}


