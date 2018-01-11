package jumpshot;

import java.awt.*;
import java.util.Enumeration;
import java.awt.event.*;

//This class is the canvas on which the process ids are displayed. It
//is located vertically on the left side of ProgramCanvas in mainPanel of
//ClogDisplay object.
public class VertScaleCanvas extends Canvas {
  ClogDisplay parent;
  ProgramCanvas canvas;

  public int lineSize;
  int maxH;
  public Color vCanBColor, vCanFColor;
  public Font vCanFont;
  
  JProcess currProc;
  
  public VertScaleCanvas (ClogDisplay p) {
    super ();
    parent = p;
    setup ();
  }
  //setup methods------------------------------------------------------------
  void setup () {
    adjustCanvasStuff ();
    canvas = parent.canvas;
    setSize (maxH, canvas._yPix);
    this.enableEvents (AWTEvent.MOUSE_EVENT_MASK);
  }
  
  void adjustCanvasStuff () {
    setCursor (new Cursor (Cursor.HAND_CURSOR));
    setBackground (vCanBColor = parent.frameBColor);
    setForeground (vCanFColor = parent.frameFColor);
    setFont (vCanFont = parent.frameFont);
    FontMetrics fm = getToolkit ().getFontMetrics (vCanFont);
    lineSize = fm.getHeight ();
    maxH = fm.stringWidth (new String ("0000"));
  }
  //--------------------------------------------------------------------------
  
  public void paint (Graphics g) {drawStuff (g, 0, 2, canvas.heightCan);}
  
  //This method is draws the vertical scale containing process Ids. This method
  //is used by paint () and also PrintDlg class to print.
  public void drawStuff (Graphics g, int x, int y, int height) {
    if (g instanceof PrintGraphics) g.setColor (parent.printFColor);
    
    double yFac = height / (double)canvas.heightCan;
    
    for (int i = 0; i < canvas.procVector.size (); i++) {
      int xcord = x;
      int ycord = (int)Math.rint ((canvas.getProcYCord (i)) * yFac + lineSize / 4.0 + y);
      int procId = ((JProcess)canvas.procVector.elementAt (i)).procId;
      g.drawString (getNumString (procId, 4), xcord, ycord);
    }
  }
  
  //method used to format process Id
  public String getNumString (int num, int ct) {
    String numStr = Integer.toString (num);
    int fill = ct - numStr.length ();
    
    for (int i = 0; i < fill; i++) numStr = " " + numStr;
    return numStr;
  }
  
  //event handler methods-------------------------------------------------------
  
  //Only MouseEvents will be caught
  
  //Event handler method for MouseEvents
  public void processMouseEvent (MouseEvent e) {
    if (e.getID () == MouseEvent.MOUSE_CLICKED) {
      if (e.getClickCount () == 2) handleProcDlg (e);
    }
    else super.processMouseEvent (e);
  }
  
  //if the mouse is clicked upon some process Id then display the procDlg for
  //that process.
  void handleProcDlg (MouseEvent e) {
    int y = e.getY ();
    int index = canvas.findIndex (y);
    if (index > -1) {
      currProc = (JProcess)(canvas.procVector.elementAt (index));
      parent.procDlg.reset (currProc);
      parent.procDlg.show ();
    }
  }
  //----------------------------------------------------------------------------
}
