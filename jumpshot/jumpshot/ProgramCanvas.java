package jumpshot;

import java.awt.*;
import java.awt.image.*;
import java.util.Vector;
import java.util.Enumeration;
import java.awt.event.*;
import java.lang.Math;

/*
  This class is the canvas on which all drawing of Jumpshot data is done
  A new data structure containing a vector of JProcess objects resides here.
  Each of these JProcess objects represent a process and contain a vector
  'stateVector' which is a list of all states of that process, in ascending order of 
  their end timestamps. 
  
  Double Buffering is used for smooth scrolling. For an explanation see the documents.
*/

public class ProgramCanvas extends Canvas {
  public ClogDisplay parent;

  //Data Structures
  
  //Vector containing all displayed processes including those whose
  //dispStatus is turned off and which are not deleted.
  public Vector procVector;    
  public Vector dprocVector;    //Vector containing all deleted processes

  double xDensity;              //pixels / second
  double yDensity;              //pixels / process vertical thickness
  int _xPix;                    //width of each image
  int _yPix;                    //height of each image
  int widthCan;                 //height of the viewport 
  int heightCan;                //width of the viewport
  
  double maxT;                  //Maximum time in the display
  double tMaxT;                 //end time of the last state in all processes
  
  int maxH;                     //horizontal pixel limit on display
  //horizontal distance between the start of the viewport and the zoom lock line
  int zDist;

  int panePosx;                 //horizontal scroll postion of ScrollPane container
  public int sbPos;             
  int cursorPos;                //Horizontal cursor position in this canvas
  int tbegH;                    //previous horizontal scrollbar value
  boolean fflagH, bflagH;       //flags describing forward or backward scrolling (horizontally) 
  
  public double zoomH;          //double value describing the zoom
  double zF;                    //zoom Factor
  
  double begTime;               //time at start of the viewport
  double endTime;               //time at the end of the viewport
  double currTime;              //time at the current cursor position
  double zXTime;                //time at which the zoom lock line was set
  double elapsedPoint;          //time at which the elapsed time line was set
  
  public boolean elTLineDispStatus = true;    //elapsed time line display status
  public boolean zoomLkLineDispStatus = true; //zoom lock line display status
  
  int proct;                    //process count
  int dproct;                   //deleted process count
  
  int ygap = 0;                 //Distance left unused at the top of drawing
  int xgap = 0;                 //Distance left unused at the left side of drawing
  int endGap = 10;              //horizontal gap at the end of data
  
  MyImage [] img;               //array of 3 offscreen images
  MyImage currImg;              //current image placed in the viewport

  
  public int lineSize;          //vertical distance of the font
  public int dpi;               //number of pixels / inch on the screen
  public FontMetrics fm;        //FontMetrics object describing the curren font
 
  int maxLevel = 0;             //maximum level of nesting
  double nestFactor;            //value describing the height difference between levels
  //nestFactor * yDensity will be the difference in height between two consecutive levels
  
  boolean message = false;      //value decribing whether an arrow circle is present ??
  boolean Windows = false;      //catering for the extra scrollbar
  boolean arrowDispStatus = true; 
  public boolean setupComplete; //setup status 

  //Constructor
  public ProgramCanvas () {super ();}
  
  //Setup methods -----------------------------------------------------------
  public void init (ClogDisplay p) {
    setupComplete = false;
    parent = p;
    setupData ();
    adjustCanvasStuff ();
    setupStates ();
    setupNestedStates ();
    drawStates ();
  }
  
  void setupData () {
    proct = 0; dproct = 0;
    procVector = new Vector ();
    dprocVector = new Vector ();
    nestFactor = 0.10;
    zoomH = 1.0;
    zF = 2.0;
    begTime = 0.0;
  }
 
  void adjustCanvasStuff () {
    Font f = parent.frameFont;
    setFont (f);
    fm = getToolkit ().getFontMetrics (f);
    lineSize = fm.getHeight ();
    dpi = getToolkit ().getScreenResolution ();
  }

  void setupStates () {
    Enumeration enum = parent.data.elements (); 
    while (enum.hasMoreElements ()) {
      StateInfo evt = (StateInfo)enum.nextElement ();
      addState (evt);
    }
    
    enum = procVector.elements ();
    while (enum.hasMoreElements ()) {
      JProcess p = (JProcess)enum.nextElement ();
      p.begT = ((ProcessState)p.procStateVector.firstElement ()).info.begT;
    }
  }

  //This method goes trough the stateVector for each process and assigns the 
  //appropriate level to each state. If no nesting is present then a level of
  //0 will be assigned
  void setupNestedStates () {
    waitCursor ();
    maxLevel = 0;

    Enumeration enum = procVector.elements ();
    while (enum.hasMoreElements ()) {
      //Scan each process
      StateInfo prev = null;
      
      JProcess p = (JProcess)enum.nextElement ();
      Vector a = p.procStateVector;
      
      //For each process assign levels to all states 
      for (int i = a.size () - 1; i >= 0; i--) {
        ProcessState currState = (ProcessState)a.elementAt (i);
        if (currState.info.stateDef.checkbox.getState ()) {
          if (prev != null) {
            if (prev.begT <= currState.info.begT) {
              //Entering here should mean that this state is nested inside the 
              //previous state and hence shoud be smaller.
              currState.info.level = prev.level + 1;
              currState.info.higher = prev;
            }
            else {
              //Entering here means that the this state is independent of the previous
              //state. however there may be higher states that contain both this and 
              //the previous state. An EFFICIENT WAY FOR THIS HAS TO BE DETERMINED.
              StateInfo temp = prev.higher;
              currState.info.level = prev.level;
              
              while (temp != null && temp.begT > currState.info.endT) {
                //Means that the current State is definately not containted in temp..
                currState.info.level --;
                temp = temp.higher;
              }
              currState.info.higher = temp;
            }
          }
          prev = currState.info;
        }
        
        if (currState.info.level > maxLevel) maxLevel = currState.info.level;
      }
    }
    normalCursor ();
  }
  
  //When the setup is complete draw all the states
  public void drawStates () {
    waitCursor ();
    Dimension dimVP = parent.paneC.getViewportSize ();
    widthCan = dimVP.width; 
    heightCan = dimVP.height - parent.paneC.getHScrollbarHeight ();
    
    _xPix = 3 * widthCan;
    _yPix = heightCan - 3 * lineSize;
    xDensity = widthCan / maxT; tMaxT = maxT;
    maxT += (endGap * 1.0) /(2 * xDensity);
    xDensity = widthCan / maxT;
    yDensity = _yPix / (proct + ((proct + 1) / 5.0));
    updateH ();
    
    setupImg ();
    Redraw ();
    repaint ();
    panePosx = _xPix;
    parent.zoomH ();
    
    setupComplete = true;
    setupEventHandlers ();
    
    adjustStartEndTimes (); parent.adjustZF (zF);
    normalCursor ();
  }
  //end of setup methods-----------------------------------------------------------
  
  public void paint (Graphics g) {
    Point p = parent.paneC.getScrollPosition ();
    int x = parent.hbar.getValue ();
    
    //If the ScrollPane's horizontal scrollbar was accidently pressed
    if (p.x != panePosx && x == tbegH) {
      getToolkit ().beep ();
      parent.paneC.setScrollPosition (panePosx, 0); 
      
      //There is  a bug in JDK 1.1.2. If I run the program in Windows for some reason
      //the above function call takes place sometimes and sometimes not. So, it is 
      //best to redraw the entire screen by calling Refresh (). It is commented out
      //so if you are running on a Windows platform remove the comments. Event with this
      //there is not a gaurentee that it will work always but, at least the frequency of
      //it working was found to be more

      if (Windows) Refresh (); 
      return;
    }
    //We request keyboard focus out here so that we can catch events for locking zoom, etc.
    requestFocus ();
    
    //3 Images are painted each time. The reason we use images rather than directly drawing
    //onto the graphics context is that the drawings of one image can be taking place
    //concurrently with the other using threads into offscreen graphic contexts assoc.
    //with the respective images. While printing we do the opposite
    
    if (setupComplete && currImg != null){
      int prevH = getPrev (currImg.imgIdH);
      int nextH = getNext (currImg.imgIdH);
      
      g.drawImage (img [prevH].img, 0, 0, this);
      g.drawImage (img [prevH].timeimg, 0, _yPix, this);
      
      g.drawImage (currImg.img, _xPix, 0, this);
      g.drawImage (currImg.timeimg, _xPix, _yPix, this);
      
      g.drawImage (img [nextH].img, _xPix + _xPix, 0, this);
      g.drawImage (img [nextH].timeimg, _xPix + _xPix, _yPix, this);
    }
  }
  
  //methods for handling smooth scrolling, double buffereing------------------------
  
  //Allocate memory for the 3 offscreen images
  void setupImg () {
    img = new MyImage [3];
    for (int i = 0; i < 3; i++) {
      Image im = createImage (_xPix, _yPix);
      Image tim = createImage (_xPix, 3 * lineSize); 
      img [i] = new MyImage (im, tim, i, _xPix, _yPix, this);
    }
  }
  
  //Get the appropriate display for the given scroll bar position
  void adjustImgH (int x) {
    MyImage prev = img [getPrev (currImg.imgIdH)];
    MyImage next = img [getNext (currImg.imgIdH)];
    
    int epos1 = getEvtXCord (currImg.begT);
    int epos2 = getEvtXCord (currImg.endT);

    int beg = x, end = x + widthCan, tendH = tbegH + widthCan; 
    
    int gap1 = epos1 - beg;
    int gap2 = end - epos2;
   
    panePosx += (beg - tbegH); //Move the scroll position according to the increment
    
    if ((beg < epos1 && tbegH > epos1)) bflagH = true;
    else if (beg > epos1 && tbegH < epos1) bflagH = false;
    if ((end > epos2 && tendH < epos2)) fflagH = true;
    else if (end < epos2 && tendH > epos2) fflagH = false;
    
    int gr = getCurrGridH (x);
    boolean change = ((bflagH && gap1 >= widthCan) || (fflagH && gap2 >= widthCan) ||
                      gap1 > _xPix || gap2 > _xPix)?
      true : false;
    
    if (change) {
      getImgH (gr);
      panePosx = _xPix + (x - (gr * _xPix));
      bflagH = false; fflagH = false;
    }    
    tbegH = beg; begTime = getTime (tbegH); adjustStartEndTimes ();
    zXTime = getTime (tbegH); 
  }
  
  int getGridValue (int val) {
    return val - (getCurrGridH (val) * _xPix);
  }
  
  //Get the appropriate display for the given grid
  public void getImgH (int grid) {
    double begT = getTime (grid * _xPix);
    double endT = begT + getTime (_xPix);
    MyImage i = searchImg (begT, endT);
    if (i != null) currImg = i;
    else currImg.resetValues (begT, endT);
    centralizeH (); repaint ();
  }
 
  //search for a image with the given starting and ending times
  public MyImage searchImg (double begT, double endT) {
    for (int i = 0; i < 3; i++) {
      MyImage cImg = img [i];
      if (cImg != null && cImg.begT == begT && cImg.endT == endT) return cImg;
    }
    return null;
  }
  
  //Redraw all the images
  public void Redraw () {
    double begT = begTime;
    for (int i = 0; i < 3; i++) {
      double endT = begT + getTime (_xPix);
      img [i].resetValues (begT, endT);
      if (endT >= maxT) break;
      begT = endT;
    }
    currImg = img [0];
    panePosx = _xPix;
  }
  
  //Return the appropriate value from the circular buffer
  int getNext (int t) {return (t + 1) % 3;}
  
  int getPrev (int t) {
    int ret = (t - 1) % 3;
    if (t == 0) ret = 2;
    return ret;
  }

  //Draw the image data which is before and after the current Image
  void centralizeH () {
    double t = currImg.endT - currImg.begT;
    if (currImg.endT < maxT) {
      int nextId = getNext (currImg.imgIdH);
      if (img [nextId].begT != currImg.endT ||
          img [nextId].endT != (currImg.endT + t)) renderNextH (1);
    }
    if (currImg.begT > 0.0) {
      int prevId = getPrev (currImg.imgIdH);
      if (img [prevId].endT != currImg.begT ||
          img [prevId].begT != (currImg.begT - t))renderPrevH (1);
    }
  }
  
  //Draw the image data for the nth image before the current one
  void renderPrevH (int n) {
    int prevId = getPrev  (currImg.imgIdH);
    double tL = getTime (_xPix);
    img [prevId].resetValues (currImg.begT - n * tL, currImg.begT - (n - 1) * tL);
  }
  
  //Draw the image data for the nth image after the current one
  void renderNextH (int n) {
    int nextId = getNext (currImg.imgIdH);
    double tL = getTime (_xPix);
    img [nextId].resetValues (currImg.endT + (n - 1) * tL, currImg.endT + n * tL);
  }
 
  //Return the image # where the given scrollbar position points to
  int getCurrGridH (int x) {return x / _xPix;}
  //--------------------------------------------------------------------------------

  //place the given StateInfo object in the stateVector of corresponding process
  void addState (StateInfo event) {
    int pid = event.procId;
    
    if (event.endT > maxT) maxT = event.endT; 

    if ((proct - 1) < pid) addProcess (pid);
    
    JProcess currProc = (JProcess)procVector.elementAt (pid);
    ProcessState ps = new ProcessState (event, currProc);
    currProc.procStateVector.addElement (ps);
  }

  //Give values to the process before drawing on image
  public void adjustValues (JProcess p) {
    p.pt.y = (int)Math.rint (getProcYCord (getIndex (procVector, p.procId)));
    p.pt.x = xgap;
  }
  
  //Create a new JProcess object if needed
  void addProcess (int pid) {
    for (int x = proct; pid > proct - 1; x++) {
      JProcess currProc = new JProcess (x);
      procVector.addElement (currProc);
      proct ++;
    }
  }

  //Methods describing horizontal and vertical position-------------------------------
  double  getProcYCord (int pid) {
    return (ygap + ((2.0 * pid + 1.0) * yDensity / 2.0) + ((pid + 1) * yDensity / 5.0));
  }
  
  double getEvtYCord (int procId) {
    //This method calculates the y co-ordinate of the given event w.r.t the
    //y co-ordinate of the time line for the related processor
    //yDensity contains pixels / eventHeight
    return getProcYCord (procId) - yDensity  / 2.0;
  }
  
  int getEvtXCord (double begT) {
    //This method calculates the x co-ordinate of the givent event from its
    //starting time
    //xDensity contains the value in pixels / millisecond.
    double d = begT * 1.0 * xDensity + xgap;
    return (int)(Math.rint (d));
  }

  double getTime (int pos) {
    double time = (1 / xDensity) * pos;
    return time;
  }
  
  int getW (double begT, double endT) {
    //This method gets the length for the given event based on its starting and
    //ending timestamps
    double d = (endT - begT) * 1.0 * xDensity;
    return  (int)(Math.rint (d));
  }
  
  void updateH () {
    maxH = getW (0, maxT);
  }
 
  //---------------------------------------------------------------------------------------
  
  //Return index of the process where the given y position is placed
  public int findIndex (int y) {
    int index = -1;
    for (int i = 0; i < proct; i++) {
      int a = (int)Math.rint (getEvtYCord (i));
      int b = a + (int)Math.rint (yDensity);
      if (y >= a && y <= b) {index = i; break;}
    }
    return index;
  }
  
  //Return the state upon which the given x and y coordinates are placed
  public ProcessState findState (int x, int y) {
    int index = findIndex (y);
    if (index != -1) {
      JProcess currProc = (JProcess)(procVector.elementAt (index));
      if (currProc.dispStatus) {
        Enumeration e = currProc.procStateVector.elements ();
        while (e.hasMoreElements ()) {
          ProcessState currState = (ProcessState)(e.nextElement());
          //Determining using position (less accurate)
          if (currState.info.stateDef.checkbox.getState () &&
              (getEvtXCord (currState.info.begT - currImg.begT) <= cursorPos) &&
              (cursorPos <= getEvtXCord (currState.info.endT - currImg.begT)) &&
              checkLevel (currState, y))
            return currState;
        }
      }
    }
    return null;
  }

  //Method checks whether the given y coordinate is valid above the given state  
  boolean checkLevel (ProcessState currState, int y) {
    //This check is not very efficient as for small nestFactors differentiating between
    //nested states may not yield correct results. Here we give preference to the boundry
    //of the inner state over the outer state --> y => currState.pt.y && y <= (curr.......
    //A choice had to be made and we chose this.
    if (y >= currState.pt.y && y <= (currState.pt.y + currState.dim.height))
      return true;
    return false;
  }
  
  //This method returns the index in the given vector of the process with the
  //given procId
  public int getIndex (Vector v, int procId) {
    Enumeration enum = v.elements ();
    int index = 0;
    while (enum.hasMoreElements ()) {
      JProcess currProc = (JProcess)(enum.nextElement ());
      if (currProc.procId == procId) break;
      index ++;
    }
    return index;
  }
  
  //Return the ArrowInfo object upon which the given x and y coordinates are placed
  public ArrowInfo findMsg (int x, int y) {
    if (arrowDispStatus) {
      int index = findIndex (y);
      if (index != -1) {
        JProcess currProc = (JProcess)(procVector.elementAt (index));
        Enumeration enum = parent.quiver.arrowVector.elements ();
        while (enum.hasMoreElements ()) {
          ArrowInfo arrow = (ArrowInfo)enum.nextElement ();
          if (arrow.begProcId == currProc.procId) {
            int x1 = getEvtXCord (arrow.begT - currImg.begT);
            int y1 = currProc.pt.y;
            if (((x1 - img [0].brad) <= (x - _xPix)) && 
                ((x - _xPix) <= (x1 + img [0].brad)) &&
                ((y1 - img [0].brad) <= y) && (y <= (y1 + img [0].brad))) 
              return arrow;
          }
        }
      }
    }
    return null;
  }

  //Event Handler methods----------------------------------------------------------------
  //Events may be generated when the mouse is pressed, released or moved and also if
  //keyboard keys 'Z' or 'T' are pressed

  void setupEventHandlers () {
    this.enableEvents  (AWTEvent.MOUSE_MOTION_EVENT_MASK | 
                        AWTEvent.MOUSE_EVENT_MASK |
                        AWTEvent.KEY_EVENT_MASK);
  }

  //Handles the event when the mouse is moved
  public void processMouseMotionEvent (MouseEvent e) {
    if (e.getID () == MouseEvent.MOUSE_MOVED) {
      adjustTimeField (e.getX ());
      adjustElTimeField ();
    }
    else super.processMouseMotionEvent (e);
  }
  
  //Handles the event when the mouse is pressed or released
  public void processMouseEvent (MouseEvent e) {
    if (e.getID () == MouseEvent.MOUSE_PRESSED) {
      message = handleMsgDlg (e);
      if (!message) handleEventDlg (e);
    }
    else if (e.getID () == MouseEvent.MOUSE_RELEASED) {
      if (message) parent.msgDlg.setVisible (false);
      parent.stateDlg.setVisible (false);
    }
    else super.processMouseEvent (e);
  }
  
  //Check if a state dialog is to be displayed??
  public void handleEventDlg (MouseEvent e) {
    int x = e.getX (), y = e.getY ();
    ProcessState currEvt = findState (x, y);
    if (currEvt != null) {
      parent.stateDlg.reset (currEvt);
      Point p = getLocationOnScreen ();
      int tx = x, ty = y;
      tx += p.x; ty += p.y;
      parent.stateDlg.setLocation (tx, ty);
      parent.stateDlg.show ();
    }
  }
  
  //Check if a message dialog is to be displayed??
  public boolean handleMsgDlg (MouseEvent e) {
    int x = e.getX (), y = e.getY ();
    ArrowInfo arrow = findMsg (x, y);
    if (arrow != null) {
      parent.msgDlg.reset (arrow);
      Point p = getLocationOnScreen ();
      int tx = x, ty = y;
      tx += p.x; ty += p.y;
      parent.msgDlg.setLocation (tx, ty);
      parent.msgDlg.show ();
      return true;
    }
    return false;
  }
  
  //Handles the event when the key is pressed
  public void processKeyEvent (KeyEvent e) {
    int keyCode = e.getKeyCode ();
    
    if (e.getID () == KeyEvent.KEY_PRESSED) {
      if (keyCode == KeyEvent.VK_Z) lockZoom ();
      else if (keyCode == KeyEvent.VK_T) 
        fixElTimePointer (currTime);
    }
    else super.processKeyEvent (e);
  }
  //---------------------------------------------------------------------------------------
  
  //lock zoom at the current cursor position
  void lockZoom () {
    if (zoomLkLineDispStatus) {
      zXTime = currTime;
      Refresh ();
      //  currImg.reDrawZoomLkLine (); //Not yet implemented
      repaint ();
    }
  }
  
  //fix the elapsed time line to the given time
  void fixElTimePointer (double time) {
    if (elTLineDispStatus) {
      elapsedPoint = time;
      Refresh ();
      // currImg.reDrawElTimeLine (); //Not yet implemented
      repaint ();
    }
  }
  
  //Methods controlling zooming of data--------------------------------------------------

  //zoom in horizontally
  public void zoomInH () {waitCursor (); zoomH (zF); normalCursor ();}
  
  //zoom in Vertically
  public void zoomOutH () {waitCursor (); zoomH (1 / zF); normalCursor ();} 
    
  //reset view so that all the data fits in a viewport
  public void resetView () {
    waitCursor ();
    
    changeConst (1 / zoomH);
    
    if (maxH != widthCan) {
      xDensity = widthCan / maxT; tMaxT = maxT;
      maxT += (endGap * 1.0) /(2 * xDensity);
      xDensity = widthCan / maxT;      
      updateH ();
    }
    adjustZoomImg (0);
    normalCursor ();
  }
  
  //method called whenever a horizontal zoom is performed
  public void zoomH (double z) {
    //Calculations required whenever a horizontal zoom is needed.
    int tmaxH = maxH;

    zDist = getW (getTime (tbegH), zXTime);
    
    changeConst (z);
    
    //Get the scrollbar Position in the zoomed Image
    int xcord = getEvtXCord (zXTime);
    sbPos = ((tmaxH > widthCan || maxH > widthCan) && xcord > zDist)? xcord - zDist : 0;
     
    if (sbPos + parent.hbar.getVisibleAmount () >= maxH)
      sbPos =  maxH - parent.hbar.getVisibleAmount ();
    if (sbPos < 0) sbPos = 0;
    
    //Get the appropriate image
    adjustZoomImg (sbPos);
  }
  
  public void changeConst (double z) {
    zoomH *= z; xDensity *= z;
    updateH ();
 
  }
  
  public void adjustZoomImg (int sbPos) {
    getImgH (getCurrGridH (sbPos)); 
    bflagH = false; fflagH = false; 
    tbegH = sbPos;
    if (sbPos > 0) {
      int extra = getW (currImg.begT, zXTime) - zDist;
      int rem = getW (currImg.begT, maxT) - widthCan;
      if (extra > rem) extra = rem;
      panePosx = _xPix + extra;
    }  
    else panePosx = _xPix;
    begTime = getTime (tbegH);
    adjustStartEndTimes ();
  }
  
  public void changeZF (double z) {
    if (z <= 0) return;
    zF = z;
  }
  //---------------------------------------------------------------------------------
  
  //This function renders the view corresponding the given start and end times
  //in the viewport.
  //start = starting time (sec)
  //end = ending time (sec)
  public void changeExTime (double start, double end) {
    double diff = end - start; 
    if (diff <= 0.0) return;
    
    int pos = getW (0, start);
    parent.hbar.setValue (pos);  //Position the scrollbar at the start time
    parent.setPosition (pos);    //render the view with this start time

    //Now zoom in/out so that the end time corresponds with the given one.
    zoomH ((endTime - begTime) / diff); 
    parent.zoomH ();
  }

  //Method called whenver the canvas has to be resized
  public void Resize () {
    waitCursor ();
    Dimension dimVP = parent.paneC.getViewportSize ();
    widthCan = dimVP.width;
    heightCan = dimVP.height;
    
    _xPix = 3 * widthCan;
    _yPix = heightCan  - 3 * lineSize;
    
    if (_yPix < 1) _yPix = 1;
    if (_xPix < 1) _xPix = 1;
    
    maxT = tMaxT;
    xDensity = (widthCan / maxT) * zoomH;
    maxT += (endGap * 1.0) / (2 * xDensity);
    xDensity = (widthCan / maxT) * zoomH;
    yDensity = _yPix / (proct + ((proct + 1) / 5.0));
    updateH ();
    
    setupImg ();
    sbPos = getEvtXCord (begTime);
    currImg = img [0];
    adjustZoomImg (sbPos);
    panePosx = _xPix + getW (currImg.begT, begTime);
    parent.zoomH ();
    normalCursor ();
  }
 
  //Redraw all the images so that changes in options can take effect
  public void Refresh () {
    waitCursor ();
    int prevH = getPrev (currImg.imgIdH);
    int nextH = getNext (currImg.imgIdH);
    img [prevH].drawStuff ();
    img [nextH].drawStuff ();
    currImg.drawStuff ();
    repaint ();
    parent.vcanvas1.repaint (); parent.vcanvas2.repaint ();
    parent.paneC.setScrollPosition (panePosx, 0);
    normalCursor ();
  }

  //Methods to handle deleting, swapping and horizontally translating process display--
  //Delete the process at the given index of procVector
  public void DeleteProc (int index) {
    JProcess p = (JProcess)(procVector.elementAt (index));
    procVector.removeElementAt (index);
    dprocVector.addElement (p);
    proct --; dproct ++;
    yDensity = _yPix / (proct + ((proct + 1) / 5.0));
  }
  
  //Delete process at the given index from dprocVector and insert it on top
  //of the process with the given pid
  //dindex = index into dProcVector which specifies the JProcess object to be removed
  //pid = pid of the process before which the removed process is to be inserted
  public void InsertProc (int dindex, int pid) {
    JProcess dp = (JProcess)(dprocVector.elementAt (dindex));
    dprocVector.removeElementAt (dindex);
    procVector.insertElementAt (dp, getIndex (procVector, pid));
    proct++; dproct--;
    yDensity = _yPix / (proct + ((proct + 1) / 5.0));
  }

  //push the timeLine and all states of the given process.
  //p = process whose time line is to be moved
  //time = this specifies the time in seconds the time line is to be moved
  //       (time > 0) => right, (time < 0) => left
  public void pushTimeLine (JProcess p, double time) {
    Enumeration enum = p.procStateVector.elements ();
    ProcessState currEvt = (ProcessState)(p.procStateVector.firstElement ());
    if (currEvt != null) {
      currEvt.info.begT += time;
      if (currEvt.info.begT < 0) {
        time -= currEvt.info.begT;
        currEvt.info.begT = 0;
      }
      currEvt.info.endT += time;
      enum.nextElement ();
    }
    
    while (enum.hasMoreElements ()) {
      currEvt = (ProcessState)(enum.nextElement ());
      currEvt.info.begT += time;
      currEvt.info.endT += time;
    }
    
    //Push the arrows
    pushArrows (p, time);
    
    //position the image appropriately
    adjustPosition ();
    
    //Redraw the image
    Refresh ();
  }
  
  //push the time line and all states of the given states
  //p = process whose time line is to be moved
  //amount = displacement in pixels to move
  //dir = direction. true => right, false => left
  public void pushTimeLine (JProcess p, int amount, boolean dir) {
    double time = getTime (amount);
    if (!dir) time *= (- 1);
    pushTimeLine (p, time);
  }
  
  void pushArrows (JProcess p, double time) {
    Enumeration enum = parent.quiver.arrowVector.elements ();
    while (enum.hasMoreElements ()) {
      ArrowInfo arrow = (ArrowInfo)enum.nextElement ();
      if (arrow.begProcId == p.procId) arrow.begT += time;
      if (arrow.endProcId == p.procId) arrow.endT += time;
    }
  }
  
  //This method adjusts the scroll position of scrollPane container when
  //the time line is being moved
  void adjustPosition () {
    double max = 0;
    Enumeration enum = procVector.elements ();
    
    while (enum.hasMoreElements ()) {
      JProcess p = (JProcess)enum.nextElement ();
      ProcessState ps = (ProcessState)p.procStateVector.lastElement ();
      if (ps.info.endT > max) max = ps.info.endT;
    }
    
    boolean end = false;
    
    if (max < tMaxT || max > tMaxT) {
      if (max < tMaxT) end = true; 
      double diff = tMaxT - max;
      tMaxT = max;
      maxT -= diff;
      updateH ();
      parent.setHoriz (parent.hbar);
      centralizeH (); 
    }
    if (end) {
      if ((parent.hbar.getValue () + parent.hbar.getVisibleAmount ()) >= parent.hbar.getMaximum ()) {
        parent.hbar.setValue (maxH - widthCan);
        adjustImgH (parent.hbar.getValue ());
        parent.paneC.setScrollPosition (getPanePosX (), 0);
      }
    }
  }
  
  //This method returns the maximum value in pixels that the time line can be moved
  //to the left before reaching the 0 time position
  //p = the process whose time line is to be moved
  public int getMaxDiff (JProcess p) {
    if (p.procStateVector.size () > 0) {
      ProcessState ps = (ProcessState)(p.procStateVector.firstElement ());
      return getEvtXCord (ps.info.begT - 0.0);
    }
    return 0;
  }
  
  //Repostions the time line to its original position
  //p = the process whose time line is to be repositioned
  public void resetTimeLine (JProcess p) {
    ProcessState ps = (ProcessState)p.procStateVector.firstElement ();
    parent.canvas.pushTimeLine (p, p.begT - ps.info.begT);
  }
  //end of methods dealing with process position manipulation------------------------------

  
  //Printing methods-----------------------------------------------------------------------
  //This function takes the current jumpshot data being viewed in the veiwport and 
  //rescales it according to the parameter dimensions and renders it onto the
  // graphics context being supplied to it.
  //g =  Graphics context where u want the states, etc. to be drawn.
  //xcord = The x position to place the data
  //ycord = The y position to place the data
  //width = width of the new display
  //height = height of the new display
  public void print (Graphics g, int xcord, int ycord, int width, int height) {
    waitCursor ();
    //Variables xDensity, yDensity, xgap and ygap need to be changed during resizing the data
    //These variables will be temporarily stored in temp variables and then restored.
    //THIS IS A TEMPORARY FIX AS IN FUTURE THE MYIMAGE FUNCTIONS WILL BE CHANGED SO THAT
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
    MyImage outputImage = new MyImage (null, null, 0, width, height - 3 * lineSize, this);
    outputImage.resetTimes (begTime, endTime);
    
    //to print we directly draw into the PrintGraphics context that we get. The reason this
    //is done is to speed up things as java has built in converters for each of its 
    //objects.
    outputImage.drawStuff (g);
    
    //Restore the original values 
    yDensity = tyDensity; xDensity = txDensity;
    xgap = txgap; ygap = tygap;
    fm = getToolkit ().getFontMetrics (getFont ());
    lineSize = fm.getHeight ();dpi = tdpi;
    normalCursor ();
  
    outputImage = null;
  }
  //end of printing methods-----------------------------------------------------------------
  
  //change the nest factor to the given value
  public void changeNestFactor (double n) {nestFactor = n;}
  
  //Recalculates the most appropriate nest factor by dividing the total thickness 
  //amongst all levels
  public void getAppropNestFac () {
    double diff = yDensity / (maxLevel + 1.0);
    parent.optionsDlg.nFacTxField.setText ((new Float (diff / yDensity)).toString ());
  }
  
  //returns the desired scroll position in the scrollpane
  public int getPanePosX () {return panePosx;}
  
  //Method required by the scrollpane container to adjust values
  public Dimension getPreferredSize () {return new Dimension (_xPix * 3, 
                                                              _yPix + 3 * lineSize);}
  //Adjust field values
  void adjustTimeField (int x) {
    cursorPos = (x - _xPix);
    parent.adjustTimeField (currTime = currImg.begT + getTime (cursorPos));
  }
  
  void adjustElTimeField () {
    parent.adjustElTimeField (currTime - elapsedPoint);
  }
  
  void adjustStartEndTimes () {
    endTime = begTime + getTime (widthCan);
    parent.adjustExTimes (begTime, endTime);
  }
  
  //sets the current cursor to WAIT_CURSOR type
  public void waitCursor () {setCursor (new Cursor (Frame.WAIT_CURSOR));}
  
  //sets the WAIT_CURSOR to cursor associated with this canvas
  public void normalCursor () {setCursor (new Cursor (Frame.CROSSHAIR_CURSOR));}
}















