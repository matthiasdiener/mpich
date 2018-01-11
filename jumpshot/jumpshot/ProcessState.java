package jumpshot;

import java.awt.*;
import java.util.*;

//This class specifies each state
public class ProcessState extends CRectangle
{
  public StateInfo info;     //object storing all relevent info. on a state
  JProcess parent;           //JProcess that this state belongs to
  
  //Constructor
  public ProcessState (StateInfo evt, JProcess p) { 
    super (new Point (), new Dimension (), evt.stateDef.color);
    info = evt;
    parent = p;
  }

  //method that draws the state at the specified place
  //x and y = These specify the point the Graphics context's coordinate place
  //width and height = These specify the dimensions of the rectangle
  //startflag = tells whether starting line is to be drawn. [see MyImage]
  //endflag = tells whether ending line is to be drawn. [see MyImage]
  public void display (int x, int y, int width, int height, Graphics g, boolean startflag,
                       boolean endflag) {
    pt.x = x; pt.y = y; dim.width = width; dim.height = height;
    displayEtRect (g, startflag, endflag);
  }
} 


