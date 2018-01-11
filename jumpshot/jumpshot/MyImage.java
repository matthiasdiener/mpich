package jumpshot;

import java.awt.*;
import java.util.Vector;
import java.util.Enumeration;
import java.awt.event.*;
import java.lang.Math;

/*
  this is the class that contains the offscreen images on which the jumpshot data
  is drawn. 2 offscreen images are used, one for states and arrows (img) and the
  other for time line (timeImg). Functions for drawing rectangles and arrows are
  present here. 
*/
class MyImage {
  ProgramCanvas parent;

  int imgIdH;     //id of this object 
  double begT;    //staring time of this object
  double endT;    //ending time of this object
  int _xPix;      //width of the entire image in pixels
  int _yPix;      //height of the entire image in pixels
  
  Image img;      //offscreen image for displaying the states and arrows
  Image timeimg;  //offscreen image for displaying the time scale
  
  int tx;         //Used to prevent drawing thin states repeatedly over one another.
  
  //Configuration variables for the arrows.
  public Color printLineColor, normLineColor;
  public Color circleColor;             //Color of the circle of arrow
  public Color hiColor = Color.red;     //Arrow ring Color
  public Color normColor = Color.white; //Arrow Color
  public int lrad = 1;                  //Radius of the little cirle
  public int brad = 3;                  //Radius of the bigger circle
  public int angle = 40;                //central angle of the arrow head
  public int ht = 20;                   //height of the arrow head
  public Color elTimeLineColor = Color.gray; //Color of elapsed time line
  public Color zoomLkLineColor = Color.lightGray;//Color of zoom lock line
  
  public int max = 6; //The maximum size of the time string in time scale
  
  //Constructor
  public MyImage (Image i, Image ti, int idH, int x, int y, ProgramCanvas p) {
    img = i;
    timeimg = ti;
    imgIdH = idH;
    _xPix = x; _yPix = y;
    parent = p;
    normLineColor = normColor; circleColor = hiColor; printLineColor = Color.black;
  }
  
  //reset time values and redraw all the stuff
  public void resetValues (double b, double e) {
    begT = b; endT = e;
    drawStuff ();
  }
  
  //reset the starting and ending time values
  public void resetTimes (double b, double e) {begT = b; endT = e;}
  
  //draw all the things in the offscreen images
  public void drawStuff () {
    Graphics g = img.getGraphics ();
    
    if (g instanceof PrintGraphics)
      g.setColor (parent.parent.printImgBColor);
    else 
      g.setColor (parent.parent.normImgBColor);
    
    g.fillRect (parent.xgap, parent.ygap, _xPix, _yPix);

    //Draw  the states.
    drawStates (begT, endT, g);
    
    //Draw the Time Line.
    
    drawTimeScale (timeimg.getGraphics ());
  
    //Draw the arrows.
    drawArrows (begT, endT, g);
    
    //Draw the elapsed Time Line 
    drawElTimeLine (begT, endT, g);
    
    //Draw the zoom lock Time Line
    drawZoomLkLine (begT, endT, g);

    //Always dispose () as it takes a lot of memory.
    g.dispose ();
  }

  //draw all the stuff in the given graphics context
  public void drawStuff (Graphics g) {
    if (g instanceof PrintGraphics)
      g.setColor (parent.parent.printImgBColor);
    else 
      g.setColor (parent.parent.normImgBColor);
    
    g.fillRect (parent.xgap, parent.ygap, _xPix, _yPix);

    //Draw the states.
    drawStates (begT, endT, g);
    
    //Draw the Time Line.
    drawTimeScale (g);
  
    //Draw the arrows.
    drawArrows (begT, endT, g);
    
    //When printing do not draw these lines
    if (!(g instanceof PrintGraphics)) {
      //Draw the elapsed Time Line 
      drawElTimeLine (begT, endT, g);
      
      //Draw the zoom lock Time Line
      drawZoomLkLine (begT, endT, g);
    }
  }

  //draw all the states
  void drawStates (double b, double e, Graphics g) {
    Vector procVector = parent.procVector;
    Enumeration enum1 = procVector.elements ();

    while (enum1.hasMoreElements ()) {
      JProcess currProc = (JProcess)enum1.nextElement ();
      
      if (currProc.dispStatus) {
        parent.adjustValues (currProc); //Put the y position and width of the time Line.
        currProc.dim.width = _xPix;
        draw (currProc, g); 
        tx = -1;
        
        Vector a = currProc.procStateVector;
        for (int i = a.size () - 1; i >= 0; i--) {
          ProcessState currEvt = (ProcessState)(a.elementAt (i));
          if (currEvt.info.stateDef.checkbox.getState ())
            if (!(currEvt.info.endT < b || currEvt.info.begT > e)) 
              draw (currEvt, g);
        }
      } 
    }
  }

  //draw all the arrows
  void drawArrows (double b, double e, Graphics g) {
    if (!parent.arrowDispStatus) return;
    Enumeration enum = parent.parent.quiver.arrowVector.elements ();
    while (enum.hasMoreElements ()) {
      ArrowInfo arrow = (ArrowInfo)enum.nextElement ();
      if (check (arrow, b, e)) draw (arrow, g);
    }  boolean setupComplete = false;
    boolean printStatus;
  }
  
  //Draws the given process state in this image.
  //ps = process state to be drawn
  void draw (ProcessState ps, Graphics g) {
    double lenT = endT - begT;
    double timeDiff = ps.info.begT - begT;
    boolean startflag = (ps.info.begT >= begT)? true : false;
    boolean endflag = (ps.info.endT <= endT)? true : false;
    double effT = (timeDiff > 0)? timeDiff : 0.0;
    int maxW = parent.getW (effT, lenT);
    int width = parent.getW (effT, ps.info.endT - begT);
    if (width > maxW) width = maxW;
    int x = parent.getEvtXCord (effT);
    
    if (!(x == tx && width == 0)) {
      double diff = ps.info.level * parent.yDensity * parent.nestFactor;
      double height = parent.yDensity - diff; 
      double ycord = 
        parent.getProcYCord (parent.getIndex (parent.procVector, ps.info.procId)) 
        - (height / 2.0);
      
      ps.display (x, (int)Math.rint (ycord), width, (int)Math.rint (height), g,
                  startflag, endflag);
    }
    tx = x;
  }
  
  //Checks whether the given process should be drawn in this image or not.
  //p = Process being considered to be drawn.
  boolean check (JProcess p) {return p.dispStatus;}

  //Draws the Process time Line
  //p = Process to be drawn.
  void draw (JProcess p, Graphics g) {p.displayRect (g);}
  
  //Check whether the given arrow should be drawn in this image or not.
  //arrow = The arrow being considered to be drawn.
  boolean check (ArrowInfo arrow, double b, double e) {
    if (arrow.endT >= b && arrow.begT <= e) {
      int startIndex = parent.getIndex (parent.procVector, arrow.begProcId);
      int endIndex = parent.getIndex (parent.procVector, arrow.endProcId);
      int size = parent.procVector.size ();
      if (startIndex < size && endIndex < size) {
        if (((JProcess)parent.procVector.elementAt (startIndex)).dispStatus &&
            ((JProcess)parent.procVector.elementAt (endIndex)).dispStatus) 
          return true;
      }
    }
    return false;
  }

  //Draw the arrow representing a message in the given image.
  //arrow = The arrow to draw in this image
  void draw (ArrowInfo arrow, Graphics g) {
    int x1 = parent.getEvtXCord (arrow.begT - begT);
    int y1 = (int)Math.rint 
      (parent.getProcYCord (parent.getIndex (parent.procVector, arrow.begProcId)));
    int x2 = parent.getEvtXCord (arrow.endT - begT);
    int y2 = (int)Math.rint 
      (parent.getProcYCord (parent.getIndex (parent.procVector, arrow.endProcId)));
    
    //Drawing the arrow line
    if (g instanceof PrintGraphics) g.setColor (printLineColor);
    else g.setColor (normLineColor);
    
    g.drawLine (x1, y1, x2, y2);
    
    //Drawing the circle which gives info. about the message
    g.setColor (circleColor);
    g.fillOval (x1 - lrad, y1 - lrad, lrad * 2, lrad * 2);
    brad = (int)Math.rint (parent.yDensity / 2);
    if (brad > 10) brad = 10;
    if (brad < 3) brad = 3;
    g.drawOval (x1 - brad, y1 - brad, brad * 2, brad * 2);
    
    //Drawing the arrowhead
    double halfangle = (Math.PI / 360.0) * angle;
    double dx = (double)Math.abs(x2 - x1);
    double dy = (double)Math.abs (y2 - y1);
    double a1 = Math.atan (dy / dx);
    double radtodeg = 180.0 / Math.PI;
    int startangle = 0;
    
    if (y1 < y2 && x2 > x1)      //Forward : Downward
      startangle = (int)Math.rint (radtodeg * (Math.PI  - a1 - halfangle));
    else if (y1 > y2 && x2 > x1) //Forward : Upward
      startangle = (int)Math.rint (radtodeg * (Math.PI + a1 - halfangle));
    else if (y1 > y2 && x1 > x2) //Backward : Upward
      startangle = (int)Math.rint (radtodeg * ((2.0 * Math.PI) - a1 - halfangle));
    else if (y2 > y1 && x1 > x2) //Backward : Downward
      startangle = (int)Math.rint (radtodeg * (a1 - halfangle));
    else if (y1 == y2) {
      startangle = (int)Math.rint (radtodeg * (Math.PI - halfangle));
      if (x1 > x2) startangle += 180;
    }
    else if (x1 == x2) {
      startangle = (int)Math.rint (radtodeg * (Math.PI / 2.0 - halfangle));
      if (y1 > y2) startangle += 180;
    }

    if (g instanceof PrintGraphics) g.setColor (printLineColor);
    else g.setColor (normLineColor);
    ht = (int)Math.rint (parent.yDensity);
    if (ht > 20) ht = 20;
    g.fillArc (x2 - ht, y2 - ht, ht * 2, ht * 2, startangle, angle);
  }
  
  //checks whether the given time is valid for this image
  boolean check (double time, double b, double e) 
  {return (time >= b && time <= e)? true : false;}
  
  //draw the elapsed time line
  void drawElTimeLine (double b, double e, Graphics g) {
    if (parent.elTLineDispStatus && check (parent.elapsedPoint, b, e)) {
      int xcord = parent.getEvtXCord (parent.elapsedPoint - begT);
      g.setColor (elTimeLineColor);
      g.drawLine (xcord, 0, xcord, _yPix);
      g.drawString ("Elapsed Time", xcord, _yPix);
    }
  } 
  
  //This function is supposed to redraw the area where the previous 
  //ElTimeLine was and then draw the new one. It would boost up 
  //speed as the entire area will not have to be drawn. Since, 
  //it is not implemented yet we will redraw the entire area..
  void reDrawElTimeLine () {
    //These are the starting and ending times of the area where
    //the prev. ElTimeLine stood and that has to be drawn
    //so that the prev. ElTimeLine is not seen.
    //double b, e;
    //reDrawArea (b, e);
    Graphics g = img.getGraphics ();
    reDrawArea (begT, endT, g);
    drawElTimeLine (begT, endT, g);
    g.dispose ();
  }
  
  //draw the zoom lock line
  void drawZoomLkLine (double b, double e, Graphics g) {
    if (parent.zoomLkLineDispStatus && check (parent.zXTime, b, e)) {
      int xcord = parent.getEvtXCord (parent.zXTime - begT);
      g.setColor (zoomLkLineColor);
      g.drawLine (xcord, 0, xcord, _yPix);
      g.drawString ("Zoom Lock", xcord, _yPix);
    }
  } 
  
  //This function is supposed to redraw the area where the previous 
  //ZoomLkLine was and then draw the new one. It would boost up 
  //speed as the entire area will not have to be drawn. Since, 
  //it is not implemented yet we will redraw the entire area..
  void reDrawZoomLkLine () {
    //These are the starting and ending times of the area where
    //the prev. ZoomLkLine stood and that has to be drawn
    //so that the prev. ZoomLkLine is not seen.
    //double b, e;
    //reDrawArea (b, e);
    Graphics g = img.getGraphics (); 
    reDrawArea (begT, endT, g);
    drawZoomLkLine (begT, endT, g);
    g.dispose ();
  }
  
  //redraw a given area in the image
  void reDrawArea (double b, double e, Graphics g) {
    g.setColor (parent.parent.normImgBColor);
    g.fillRect (parent.getEvtXCord (b - begT), 0,
                parent.getEvtXCord (e - begT), _yPix);
    drawStates (b, e, g);
    drawArrows (b, e, g);
    drawElTimeLine (b, e, g);
    drawZoomLkLine (b, e, g);
  }
  
  //Draw the Scale for the image in the specified graphics context
  void drawTimeScale (Graphics g) {
    //Make the filled rectangle
    if (g instanceof PrintGraphics) {
      g.setColor (parent.parent.printBColor);
      g.fill3DRect (parent.xgap, parent.ygap + _yPix, _xPix, parent.lineSize * 3, true);
    }
    else {
      g.setColor (parent.parent.frameBColor);
      g.fill3DRect (0, 0, _xPix, parent.lineSize * 3, true);
    }
    
    g.setColor (parent.parent.frameFColor);
    
    double inchT = parent.getTime (parent.dpi);
    int i = (int)Math.rint (begT / inchT);
    //Start time
    double t = i * inchT;

    while (t < endT && t < parent.maxT) {
      int xcord = 2 * parent.xgap + i * parent.dpi - parent.getEvtXCord (begT);
     
      String t1 = (new Float (t)).toString ();
      int index = max;
      if (index >= t1.length ()) index = t1.length () - 1;
      String t2 = t1.substring (0, index);
      
      if (g instanceof PrintGraphics) {
        g.drawString ("|", xcord, parent.lineSize + parent.ygap + _yPix);
        g.drawString (t2, xcord - (int)Math.rint (parent.fm.stringWidth (t2) / 2.0),
                      2 * parent.lineSize - 1 + parent.ygap + _yPix);
      }
      else { 
        g.drawString ("|", xcord, parent.lineSize);
        g.drawString (t2,
                      xcord - (int)Math.rint (parent.fm.stringWidth (t2) / 2.0),
                      2 * parent.lineSize - 1);
      }
      t = (++i * inchT);
    }
  }  

  public String toString () {
    return "MyImage [imgIdH = " + Integer.toString (imgIdH) + ", begT = " + 
      Double.toString (begT) + ", endT = " + Double.toString (endT) +
      ", _xPix = " + Integer.toString (_xPix) + ", _yPix = " + Integer.toString (_yPix) + 
      ", Image = " + img.toString () + "]";
  }
}
