package jumpshot;

import java.awt.*;
import java.util.Vector;
import java.util.Enumeration;

//This class draws the bins and the time scale onto a offscreen image
//associated with it.
class HistImage {
  HistCanvas parent;//The canvas from which this offscreen image is got
 
  double begT;      //starting time in seconds of bins in this image
  double endT;      //ending time in seconds of bins in this image   
  int _xPix;        //width of the image 
  int _yPix;        //height of the image

  Image img;        //offscreen image object
  
  int statesDrawn;  //current number of states drawn
  int highestCt;    //highest number of states in any bin
  
  //Constructor
  public HistImage (Image i, int x, int y, HistCanvas p) {
    img = i;
    _xPix = x; _yPix = y;
    parent = p;
  }
  
  //change the starting and ending times and 
  //re draw all the bins
  public void resetValues (double b, double e) {
    begT = b; endT = e;
    drawStuff ();
  }
  
  //Change the starting and ending times
  public void resetTimes (double b, double e) {
    begT = b; endT = e;
  }
  
  //methods to draw bins, time scale-------------------------------------------
  //this method draws the bins and the time scale to the specified graphics
  //context
  public void drawStuff (Graphics g) {
    if (g instanceof PrintGraphics) 
      g.setColor (parent.parent.parent.printImgBColor);
    else 
      g.setColor (parent.parent.parent.normImgBColor);
    
    g.fillRect (parent.xgap, parent.ygap, _xPix, _yPix - 3 * parent.lineSize);
    
    //draw the stuff
    drawBins (g);
    
    //Draw the Time Line
    drawTimeLine (g);
  }
  
  //  this method draws the bins and the time scale onto the offscreen image
  public void drawStuff () {
    Graphics g = img.getGraphics ();
    g.setColor (parent.parent.parent.normImgBColor);
    g.fillRect (0, 0, _xPix, _yPix - 3 * parent.lineSize);
    
    //Draw the bins
    drawBins (g);
    
    //Draw the time scale
    drawTimeLine (g);
  }

  //This function creates bins and causes them to be drawn
  void drawBins (Graphics g) {
    double dTime = (endT - begT) / parent.numBins;
    int j = 0; statesDrawn = 0; highestCt = 0;

    Vector v = parent.stateDef.stateVector;
    
    for (int i = 0; i < parent.numBins; i++) {
      //Limits of a bin
      double startTime = begT + (i * dTime);        
      double finishTime = begT + ((i + 1) * dTime);
      int ct = 0;
      
      StateInfo currState;
      while (j < v.size () &&
             ((currState = (StateInfo)v.elementAt (j)).lenT < finishTime ||
              (currState.lenT == finishTime && currState.lenT == endT))) {
        if (startTime <= currState.lenT) {
          ct++;
          statesDrawn ++;
        }
        j++;
      }
      if (highestCt < ct) highestCt = ct;
      
      //draw the bin
      if (ct > 0) drawBin (g, startTime, finishTime, ct);
    }
  }
  
  //This function draws a histogram bar for a bin.
  void drawBin (Graphics g, double startTime, double endTime, int ct) {
    int width = parent.getW (startTime, endTime);
    if (width == 0) width = 1; //The least value for width is 1

    int height = parent.getHistHeight (ct);
    if (height == 0 && ct > 0) height = 1;
    if (height > (_yPix - 3 * parent.lineSize)) height = _yPix - 3 * parent.lineSize;
    int x = parent.getEvtXCord (startTime - begT);
    
    g.setColor (parent.stateDef.color);
    g.fillRect (x, _yPix - 3 * parent.lineSize - height + parent.ygap, width, height);
    g.setColor (Color.white);
    g.drawRect (x, _yPix - 3 * parent.lineSize- height + parent.ygap, width, height);
  }
  
  //This function draws the timeLine for the image
  void drawTimeLine (Graphics g) {
    if (g instanceof PrintGraphics) g.setColor (parent.parent.parent.printFColor);
    else {
      g.setColor (parent.parent.frameBColor);
      g.fill3DRect (parent.xgap, _yPix - 3 * parent.lineSize + parent.ygap, 
                    _xPix, parent.lineSize * 3, true);
      g.setColor (parent.parent.frameFColor);
    }
    
    double inchT = parent.getTime (parent.dpi);
    int i = (int)Math.rint (begT / inchT);
    double t = i * inchT;
    while (t < endT) {
      int xcord = i * parent.dpi - parent.getEvtXCord (begT) - 
        (int)Math.rint (parent.fm.charWidth ('|') / 2.0);
      
      g.drawString ("|", xcord + 2 * parent.xgap, 
                    _yPix - 2 * parent.lineSize + parent.ygap);
      
      String t1 = (new Float (t)).toString ();
      
      g.drawString (t1,
                    2 * parent.xgap + xcord - (int)Math.rint (parent.fm.stringWidth (t1) / 2.0),
                    _yPix - parent.lineSize - 1 + parent.ygap);
      t = (++i * inchT);
    }
  }  
  //end of methods to draw bins, time scales-------------------------------------------------
}






