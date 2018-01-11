package jumpshot;

import java.awt.*;
import java.util.*;
import java.awt.event.*;

//This class creates the frame with buttons which stand for each
//state type. The buttons are pressed to produce histogram frames

public class StateButtons extends Frame
implements ActionListener {
  public ClogDisplay parent;
  
  public StateButtons (ClogDisplay pr, int numCols, int colorSz) {
    super ("Process States");
    parent = pr;
    
    Color bgColor = Color.black, fgColor = Color.white;
    setBackground (Color.white); 
    
    StringBuffer dummy = new StringBuffer ();
    for (int i = 0; i < colorSz; i++) dummy.append (" ");
  
    Panel p = new Panel ();
    p.setLayout (new GridBagLayout ());
    
    GridBagConstraints con = new GridBagConstraints ();
    con.fill = GridBagConstraints.BOTH;
    con.weightx = 1.0; con.weighty = 1.0;
    
    int row = 0, col = 0;
    
    Enumeration enum = pr.stateDefs.elements ();
    
    while (enum.hasMoreElements ()) {
      CLOG_STATE s = (CLOG_STATE)enum.nextElement ();
      
      if (s.stateVector.size () > 0) {
        con.gridx = col; con.gridy = row;
        
        Label label = new Label (dummy.toString ());
        label.setBackground (s.color);
        p.add (label, con); 
        
        con.gridx = col + 1; 
        
        MyButton button = new MyButton (s.description.desc, this);
        button.setBackground (bgColor);
        button.setForeground (fgColor);
        button.setFont (new Font ("SansSerif", Font.BOLD, 14));
        p.add (button, con);
        
        con.gridx = col + 2;
        
        p.add (s.checkbox = new Checkbox ("", true), con);
        
        if (((col / 3) + 1) == numCols) {col = 0; row ++;}
        else col += 3;
      }
    }
    
    //Now attach buttons to switch on / off all
    MyButton button = new MyButton ("All On", this);
    button.setBackground (fgColor); button.setForeground (bgColor);
    button.setFont (new Font ("SansSerif", Font.BOLD, 14));    
    con.gridx = col + 1; con.gridy = row;
    p.add (button, con);
    
    if (((col / 3) + 1) == numCols) {col = 0; row ++;}
    else col += 3;
    
    button = new MyButton ("All Off", this);
    button.setBackground (fgColor); button.setForeground (bgColor);
    button.setFont (new Font ("SansSerif", Font.BOLD, 14));
    con.gridx = col + 1; con.gridy = row;
    p.add (button, con);
    
    if (((col / 3) + 1) == numCols) {col = 0; row ++;}
    else col += 3;
    
    button = new MyButton ("Ok", this);
    button.setBackground (fgColor); button.setForeground (bgColor);
    button.setFont (new Font ("SansSerif", Font.BOLD, 14));
    con.gridx = col + 1; con.gridy = row;
    p.add (button, con);
    
    if (((col / 3) + 1) == numCols) {col = 0; row ++;}
    else col += 3;
     
    button = new MyButton ("Close", this);
    button.setBackground (fgColor); button.setForeground (bgColor);
    button.setFont (new Font ("SansSerif", Font.BOLD, 14));
    con.gridx = col + 1; con.gridy = row;
    p.add (button, con);
    
    add ("Center", p);
    pack ();
    setSize (getMinimumSize ());
  
    // Define, instantiate and register a WindowListener object.
    addWindowListener (new WindowAdapter () {
      public void windowClosing (WindowEvent e) {setVisible (false);}
    });
  }
  
  void switchAll (boolean val) {
    Enumeration enum = parent.stateDefs.elements ();
    
    while (enum.hasMoreElements ()) {
      CLOG_STATE s = (CLOG_STATE)enum.nextElement ();
      if (s.stateVector.size () > 0) s.checkbox.setState (val);
    }
  }
  //end of setup methods------------------------------------------------------

  //causes the histogram frame to be displayed for the given state name
  public void getHistogram (String name) {
    waitCursor ();
    parent.waitCursor ();
    parent.canvas.waitCursor ();
    parent.parent.waitCursor ();
    new Histwin (name, parent);
    parent.parent.normalCursor ();
    parent.canvas.normalCursor ();
    parent.normalCursor ();
    normalCursor ();
  }

  //The function prints out the key to the upshot data.
  public int drawStuff (Graphics g, int x, int y, int width, int height) {
    Font f = new Font ("SansSerif", Font.BOLD, 14);
    FontMetrics fm = getToolkit ().getFontMetrics (f);
    g.setFont (f);
    int charW = fm.stringWidth (" "), charH = fm.getHeight ();
    int hgap1 = charW, hgap2 = 2 * charW, vgap = fm.getAscent ();
    int w = 40, h = charH; //Dimensions of state rectangles
    int xcord = x, ycord = y;
    
    Enumeration enum = parent.stateDefs.elements ();
    while (enum.hasMoreElements ()) {
      CLOG_STATE s = (CLOG_STATE)enum.nextElement ();
      
      if (s.stateVector.size () > 0) {
        //This state type is not empty
        if ((xcord + w + hgap1 + fm.stringWidth (s.description.desc) + hgap2) > 
            (width + x)) {
          xcord = x; ycord += (charH + vgap);
        }
        
        g.setColor (s.color);
        g.fillRect (xcord, ycord, w, h);
        g.setColor (Color.white);
        g.drawRect (xcord, ycord, w, h);
        g.drawString (s.description.desc, xcord + w + hgap1, ycord + h);
        xcord += (w + hgap1 + fm.stringWidth (s.description.desc) + hgap2);
      }
    }
    return ycord + 2 * charH;
  }      

  //sets the cursor to the WAIT_CURSOR type
  void waitCursor () {setCursor (new Cursor (Frame.WAIT_CURSOR));} 
  
  //sets the cursor to the cursor associated with this frame
  void normalCursor () {setCursor (new Cursor (Frame.DEFAULT_CURSOR));}
  
  //event handler methods------------------------------------------------------
  public void actionPerformed(ActionEvent evt) {
    String command = evt.getActionCommand ();
    if (command.equals ("All On")) switchAll (true);
    else if (command.equals ("All Off")) switchAll (false);
    else if (command.equals ("Ok")) parent.canvas.Refresh ();
    else if (command.equals ("Close")) setVisible (false);
    else getHistogram (command);
  }
  //end of event handler methods----------------------------------------------
}






