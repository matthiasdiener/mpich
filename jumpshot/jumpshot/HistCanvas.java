package jumpshot;

import java.awt.*;
import java.util.Vector;
import java.util.Enumeration;
import java.awt.event.*;
import java.lang.Math;

/*
  On this canvas all drawing of the histograms takes place. An offscreen image
  is used as a double buffer. the bins and time scale are drawn onto this
  offscreen image which is then placed onto this canvas. The size of the image
  is that of this canvas.
*/
public class HistCanvas extends Canvas
implements ComponentListener
{public Histwin parent;
  CLOG_STATE stateDef;
  Vector stateVector;

  double xDensity;        //horizontal pixels / second
  double yDensity;        //vertical pixels / state count
  double leastT;          //length of the shortest state
  double maxT;            //length of the largest state
  double begT;            //current starting length in the canvas
  double endT;            //current ending length in the canvas  
  double dLen;            //time change in seconds / slider increment or decrement
  int _xPix;              //width of the offscreen image
  int _yPix;              //height of the offscreen image
  int widthCan;           //width of the canvas
  int heightCan;          //height of the canvas 
  int xgap;               //horizontal gap on the left
  int ygap;               //vertical gap on top
  Color canBColor = Color.black; //Background color for the canvas
  HistImage view;         //object containing the offscreen image
  
  boolean setupComplete = false;

  public int lineSize, dpi;
  public FontMetrics fm;
  int numBins;
  
  //Used for resizing the bars
  int ty;                 //previous vertical position when the mouse was pressed
  int eff_yPix;           
  public int topGap;      //gap to be subtracted from heightCan to get the appropriate
  //bin height when the command Resize to Fit is given Histwin
  
  public HistCanvas (Histwin p) {
    super ();
    waitCursor ();
    parent = p;
    setupData ();
    adjustCanvasStuff ();
    normalCursor ();
  }
  
  //setup methods---------------------------------------------------------------
  void setupData () {
    stateDef = parent.stateDef; 
    stateVector = stateDef.stateVector;
    sortLen ();
    leastT = ((StateInfo)stateVector.elementAt (0)).lenT;
    maxT = ((StateInfo)stateVector.elementAt (stateVector.size () - 1)).lenT;
    
    if (leastT == maxT) maxT += (1.0 / parent.maxLenCt); 
    begT = leastT; endT = maxT;
    dLen = (maxT - leastT) / parent.maxLenCt;
    numBins = 25; 
    this.addComponentListener (this);
  }
 
  void adjustCanvasStuff () {
    Font f = parent.frameFont;
    setFont (f);
    fm = getToolkit ().getFontMetrics (f);
    lineSize = fm.getHeight ();
    dpi = getToolkit ().getScreenResolution ();
    setCursor (new Cursor (Frame.CROSSHAIR_CURSOR));
    setBackground (canBColor);
    this.enableEvents (AWTEvent.MOUSE_MOTION_EVENT_MASK | 
                       AWTEvent.MOUSE_EVENT_MASK );
  }
  //end of setup methods----------------------------------------------------------
  public void paint (Graphics g) {g.drawImage (view.img, 0, 0, this);}

  //Allocate memory for the offscreen image
  void setupImg () {
    Image im = createImage (_xPix, _yPix);
    view = new HistImage (im, _xPix, _yPix, this);
  }
  
  //redraw the histograms
  void Redraw () {view.resetValues (begT, endT);}
  
  int getEvtXCord (double t) {
    double d = t * xDensity + xgap;
    return (int)(Math.rint (d));
  }

  double getTime (int pos) {
    double time = (1 / xDensity) * pos;
    return time;
  }
  
  int getW (double t1, double t2) {
    double d = (t2 - t1) * xDensity;
    return  (int)(Math.rint (d));
  }
  
  //Return the height in pixels of the given number of state count
  int getHistHeight(int ct) {return (int)Math.rint (yDensity * ct);}

  //This functions sorts the states in the vector in the order of their time lengths
  //Bubble sort is being used to do so.
  void sortLen () {
    for (int pass = 1; pass < stateVector.size (); pass++)
      for (int i = 0; i < stateVector.size () - 1; i++) {
        StateInfo currEvt = (StateInfo)stateVector.elementAt (i);
        StateInfo nextEvt = (StateInfo)stateVector.elementAt (i + 1);
          if (currEvt.lenT > nextEvt.lenT) {
            stateVector.setElementAt (currEvt, i + 1);
            stateVector.setElementAt (nextEvt, i);
          }
      }
    
  }
  
  //reset the heights of the bins in display proportionately
  public void reFit () {
    waitCursor ();
    yDensity = (_yPix - 3 * lineSize - topGap) / (double)view.highestCt;
    eff_yPix = (int)Math.rint (yDensity * stateVector.size ());
    Redraw ();
    repaint ();
    normalCursor ();
  }
   
  //change the number of bins in display
  public void changeNumBins (int x) {
    numBins = x;
    view.drawStuff ();
    repaint ();
  }
  
  //change the end state length by the given change in scrollbar value
  public void changeEndLen (int endCt) {
    endT = maxT - (parent.maxLenCt - endCt) * dLen;
    if (endT > begT) {
      xDensity = (widthCan * 1.0) / (endT - begT);
      view.resetValues (begT, endT);
      repaint ();
    } 
    else if (endT == begT) {
      endT = begT + dLen;
      xDensity = widthCan * 1.0 / dLen;
      view.resetValues (begT, endT);
      repaint ();
    }
  }
  
  //change the start state length by the given change in scrollbar value
  public void changeStartLen (int startCt) {
    begT = maxT - (parent.maxLenCt - startCt) * dLen;
    if (begT < endT) {
      xDensity = widthCan * 1.0 / (endT - begT);
      view.resetValues (begT, endT);
      repaint ();
    }
    else if (begT == endT) {
      begT = endT - dLen;
      xDensity = widthCan * 1.0 / dLen;
      view.resetValues (begT, endT);
      repaint ();
    }
  }
   
  //Event Handler methiods--------------------------------------------------------
  //events are generated when the mouse is pressed and dragged and when canvas is resized
 
  //handles events when the canvas is resized
  public void componentResized (ComponentEvent e) {Resize ();}
   
  public void Resize () {
    waitCursor ();
    Dimension dim = getSize ();
    widthCan = dim.width;
    topGap = parent.getInsets ().top;
    heightCan = dim.height;
    
    _xPix = widthCan;
    _yPix = heightCan;
    
    if (_yPix < 1) _yPix = 1;
    if (_xPix < 1) _xPix = 1;
    
    //will be moved from here
    if (!setupComplete) {
      setupComplete = true;
      eff_yPix = _yPix;
    }
    
    xDensity = widthCan / (endT - begT);
    yDensity = (_yPix - 3.0 * lineSize) / (double)stateVector.size ();
  
    setupImg ();
    Redraw ();
    repaint ();
    normalCursor ();
  }
  
  //handles events when the mouse is moved or dragged
  public void processMouseMotionEvent (MouseEvent e) {
    if (e.getID () == MouseEvent.MOUSE_MOVED)
      adjustTimeField (e.getX ());
    if (e.getID () == MouseEvent.MOUSE_DRAGGED) {
      int y = e.getY ();
      int dy = ty - y;
      eff_yPix += dy;
      yDensity = (eff_yPix - 3.0 * lineSize) * 1.0/ (double)stateVector.size ();
      Redraw (); repaint ();
      ty = y;
    }
    else super.processMouseMotionEvent (e);
  }
  
  //handles events when the mouse is pressed
  public void processMouseEvent (MouseEvent e) {
    if (e.getID () == MouseEvent.MOUSE_PRESSED) ty = e.getY ();
    else if (e.getID () == MouseEvent.MOUSE_RELEASED) ty = e.getY ();
    else super.processMouseMotionEvent (e);
  }

  //end of event handler methods---------------------------------------------------
  
  //This function takes the current jumpshot data being viewed in the veiwport and 
  //rescales it according to the parameter dimensions and renders it onto the
  //graphics context being supplied to it.
  //g =  Graphics context where u want the states, etc. to be drawn.
  //xcord = The x position to place the data
  //ycord = The y position to place the data
  //width = width of the new display
  //height = height of the new display
  public void print (Graphics g, int xcord, int ycord, int width, int height) {
    waitCursor ();
    //Variables xDensity, yDensity, xgap and ygap need to be changed during resizing the data
    //These variables will be temporarily stored in temp variables and then restored.
    //THIS IS A TEMPORARY FIX AS IN FUTURE THE HISTMAGE FUNCTIONS WILL BE CHANGED SO THAT
    //THESE VARIABLES CAN BE PARAMETERS
    
    //Make Copies
    double txDensity = xDensity, tyDensity = yDensity;
    int txgap = xgap, tygap = ygap, tdpi = dpi;
    FontMetrics tfm = fm;
   
    double yFac = height /  (double)heightCan, xFac = width / (double)widthCan;
    //New Values
    xDensity *= xFac; yDensity *= yFac;
    ygap = ycord; xgap = xcord;
    dpi = (int)Math.rint (((double)dpi) * xFac);
    fm = getToolkit ().getFontMetrics (g.getFont ());
    lineSize = fm.getHeight ();
    
    
    //Here we do not render offscreen images as we do not need them. we are only using
    //drawStuff (Graphics g) function that draws into the given Graphics context.
    HistImage outputImage = new HistImage (null, width, height, this);
    outputImage.resetTimes (begT, endT);
    
    outputImage.drawStuff (g);
    
    //Restore the original values 
    yDensity = tyDensity; xDensity = txDensity;
    xgap = txgap; ygap = tygap;
    fm = getToolkit ().getFontMetrics (getFont ());
    lineSize = fm.getHeight ();dpi = tdpi;
    normalCursor ();
  
    outputImage = null;
  }
  
  void adjustTimeField (int x) {parent.adjustTimeField (view.begT + getTime (x));}
  
  //sets the current cursor to the WAIT_CURSOR type
  void waitCursor () {setCursor (new Cursor (Frame.WAIT_CURSOR));} 
  
  //sets the WAIT_CURSOR to cursor associated with this canvas
  void normalCursor () {setCursor (new Cursor (Frame.CROSSHAIR_CURSOR));}

  /** Unused methods of the ComponentListener interface. */
  public void componentHidden (ComponentEvent e) {;}
  public void componentMoved (ComponentEvent e) {;}
  public void componentShown (ComponentEvent e) {;}
}
