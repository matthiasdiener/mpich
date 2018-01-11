package jumpshot;
     
import java.awt.*;
import java.awt.event.*;
import java.util.*;
/*
  This class supervises all display operations and printing for the given
  state's histogram 
  It shows a frame consisting of buttons (Resize to Fit), (Close), etc and 
  textFields like numBins, start state length, etc.
  when a particular state is selected from StateButtons dialog the name is
  passed onto this object which searches through the vector of all state
  difinitions and the matching CLOG_STATE object is retrieved
*/
public class Histwin extends Frame 
implements ActionListener, AdjustmentListener {
  ClogDisplay parent;                 
  HistCanvas canvas;           //HistCanvas object to draw bins in        
  public CLOG_STATE stateDef;  //pointer to the corresponding state definition
  
  Scrollbar binSlider;         //Scrollbar controlling the number of bins 
  Scrollbar startSlider;       //Scrollbar controlling start state length
  Scrollbar endSlider;         //Scrollbar controlling end state length
  MyTextField statesInViewField, startLenField, endLenField; 
  MyTextField cursorField, numBinField, percentInViewField;
  Panel mainPanel, controlPanel;
  
  Dimension dimScreen;         //dimension of the screen
  Dimension dimFrame;          //Initial dimension of this Frame 
  Font frameFont;                        
  public Color frameBColor;
  public Color frameFColor;
  
  int maxNumBins = 200;        //maximum number of bins
  int tNumBins = 25;           //previous number of bins
  int maxLenCt = 200;          //total divisions in start and end sliders 
  int tStartCt = 0;            //previous startSlider value
  int tEndCt = maxLenCt;       //previous endSlider value
  //Each increment or decrement in start and end scrollbars will result in
  //a change of (maximum length - minimum length) / maxLenCt seconds.
    
  //Constructor
  public Histwin (String name, ClogDisplay p) {
    super ("state " + name + " histogram");
    parent = p;
    
    //Get the CLOG_STATE
    stateDef = getState (name);
    if (stateDef == null) {
      System.out.println ("Invalid state selected");
      this.dispose ();
    }
    
    setup ();
  }
  
  //setup methods---------------------------------------------------------------
  void setup () {
    adjustFrameStuff ();
    setupSliders ();
    setupPanels ();
    pack ();
    setSize (dimFrame);
    setVisible (true);
    setupFields ();
    // Define, instantiate and register a WindowListener object.
    addWindowListener (new WindowAdapter () {
      public void windowClosing (WindowEvent e) {dispose ();}
    });
  }
  
  //This function returns the CLOG_STATE associated with the given name string.
  CLOG_STATE getState (String stateName) {
    Enumeration enum = parent.stateDefs.elements ();
    while (enum.hasMoreElements ()) {
      CLOG_STATE s = (CLOG_STATE)enum.nextElement ();
      if  (s.description.desc.compareTo (stateName) == 0) {
        return s;
      }
    }
    return null;
  }
 
  void adjustFrameStuff () {
    Toolkit toolkit = getToolkit ();
    dimScreen = new Dimension (toolkit.getScreenSize ());
    dimFrame = new Dimension ((int)Math.rint (dimScreen.width/2.0), 
                           (int)Math.rint (dimScreen.height/2.0));
    setBackground (frameBColor = parent.frameBColor);
    setForeground (frameFColor = parent.frameFColor);
    setFont (frameFont = parent.frameFont);
  }
  
  //Sets up the scrollbars controlling the starting and ending time lengths and
  //the number of bins.
  void setupSliders () {
    binSlider = new Scrollbar (Scrollbar.HORIZONTAL, 24, 0, 0, maxNumBins);
    binSlider.addAdjustmentListener (this);
    
    //For starting and ending times
    startSlider = new Scrollbar (Scrollbar.HORIZONTAL, 0, 0, 0, maxLenCt + 1);
    startSlider.addAdjustmentListener (this);
    endSlider = new Scrollbar (Scrollbar.HORIZONTAL, maxLenCt, 0, 0, maxLenCt + 1);
    endSlider.addAdjustmentListener (this);
  }

  public void adjustTimeField (double d) {
    cursorField.setText ((new Float (d)).toString ());
  }
 
  //Lays out the entire panel
  void setupPanels () {
    setLayout (new GridBagLayout ());
    
    //Define controlPanel
    Panel controlPanel = new Panel ();
    controlPanel.setLayout (new GridBagLayout ());

    GridBagConstraints con = new GridBagConstraints ();
    con.fill = GridBagConstraints.BOTH;
    con.weightx = 0; con.weighty = 1.0;
    
    con.gridx = 0; con.gridy = 0;
    controlPanel.add (new Label ("State: " + stateDef.description.desc), con);
    
    con.gridy = 1;
    controlPanel.add (new Label ("Total number of states: " + stateDef.stateVector.size ()), con);
    
    con.gridy = 2;
    controlPanel.add (new Label ("states in view: "), con);
    
    con.gridx = 1;
    controlPanel.add (statesInViewField = new MyTextField ("", 12, false), con);
    
    con.gridy = 3; con.gridx = 0;
    controlPanel.add (new Label ("%: "), con);
    
    con.gridx = 1;
    controlPanel.add (percentInViewField = new MyTextField ("", 12, false), con);
    
    con.gridx = 0; con.gridy = 4; 
    controlPanel.add (new Label ("Start state length"), con);
    
    con.gridx = 1;
    controlPanel.add (startLenField = new MyTextField ("", 12, false), con);
    
    con.gridx = 0; con.gridy = 5; con.gridwidth = 2;
    controlPanel.add (startSlider, con);
    
    con.gridy = 6; con.gridwidth = 1;
    controlPanel.add (new Label ("End state length"), con);
    
    con.gridx = 1;
    controlPanel.add (endLenField = new MyTextField ("", 12, false), con);
    
    con.gridx = 0; con.gridy = 7; con.gridwidth = 2;
    controlPanel.add (endSlider, con);
    
    con.gridy = 8; con.gridwidth = 1;
    controlPanel.add (new Label ("Number of bins"), con);
    
    con.gridx = 1;
    controlPanel.add (numBinField = new MyTextField ("", 3, false), con);
    
    con.gridx = 0; con.gridy = 9; con.gridwidth = 2;
    controlPanel.add (binSlider, con);
    
    con.gridy = 10; con.gridwidth = 1;
    controlPanel.add (new Label ("cursor"), con);
    
    con.gridx = 1;
    controlPanel.add (cursorField = new MyTextField ("", 12, false), con);
    
    con.gridx = 0; con.gridy = 11;
    controlPanel.add (new MyButton ("Resize to fit", this), con);
    
    con.gridx = 0; con.gridy = 12;
    controlPanel.add (new MyButton ("Print", this), con);
    
    con.gridx = 1;
    controlPanel.add (new MyButton ("Close", this), con);
    
    con.gridx = 0; con.gridy = 0; con.gridwidth = GridBagConstraints.RELATIVE;
    con.gridheight = GridBagConstraints.RELATIVE;
    add (controlPanel, con);

    //Define mainPanel
    con.gridwidth = con.gridheight = GridBagConstraints.REMAINDER;
    con.weightx = 1.0;
    mainPanel = new Panel ();
    mainPanel.setLayout (new GridBagLayout ());
    
    mainPanel.add (canvas  = new HistCanvas (this), con);
  
    con.gridx = 1; add (mainPanel, con);
  }
  
   
  void setupFields () {
    percentInViewField.setText ("100.0");
    startLenField.setText ((new Float (canvas.leastT)).toString ());
    endLenField.setText ((new Float (canvas.maxT)).toString ());
    statesInViewField.setText (Integer.toString (canvas.stateVector.size ()));
    cursorField.setText ((new Float (canvas.leastT)).toString ());
    numBinField.setText ("25");
  }
  //end of setup methods---------------------------------------------------------
  //Prints out jumpshot data
  void Print () {
    waitCursor ();
    String jobTitle = "Jumpshot:" + parent.parent.logFileName;
    
    //Obtain a PrintJob Object. This posts a Print dialog
    PrintJob pjob = getToolkit ().getPrintJob (this, jobTitle, parent.printPrefs);
    
    //If the user clicked Cancel in the print dialog, then do nothing
    if (pjob != null) parent.printDlg.reset (pjob, this);

    normalCursor ();
  }

  //sets the current cursor to the WAIT_CURSOR type
  void waitCursor () {setCursor (new Cursor (Frame.WAIT_CURSOR));} 

  //sets the WAIT_CURSOR to cursor associated with this frame
  void normalCursor () {setCursor (new Cursor (Frame.DEFAULT_CURSOR));}  
  
  //event handler methods-------------------------------------------------------
  //events generated from the scrollbars, buttons are caught
  
  //Handles the event whenever one of the scrollbars is modified
  public void adjustmentValueChanged (AdjustmentEvent e) {
    //Number of bins
    int numBins = binSlider.getValue () + 1;
    if (tNumBins != numBins) {
      numBinField.setText (Integer.toString (numBins));
      canvas.changeNumBins (numBins);
    }
    tNumBins = numBins;
  
    //Start state length
    int startCt = startSlider.getValue ();
    if (tStartCt != startCt) {
      canvas.changeStartLen (startCt);
      startLenField.setText ((new Float (canvas.begT)).toString ());
      statesInViewField.setText (Integer.toString (canvas.view.statesDrawn));
      percentInViewField.setText ((new Float (canvas.view.statesDrawn * 100.0 / 
                                              (double)canvas.stateVector.size ())).toString ());
    }
    tStartCt = startCt;
    
    //End state length
    int endCt = endSlider.getValue ();
    if (tEndCt != endCt) {
      canvas.changeEndLen (endCt);
      endLenField.setText ((new Float (canvas.endT)).toString ());
      statesInViewField.setText (Integer.toString (canvas.view.statesDrawn));
      percentInViewField.setText ((new Float (canvas.view.statesDrawn * 100.0 / 
                                              canvas.stateVector.size ())).toString ());
    }
    tEndCt = endCt;
  }
  
  //Handles the event whenever one of the buttons is used
  public void actionPerformed (ActionEvent evt) {
    String command = evt.getActionCommand ();
    if (command.equals ("Close")) {this.dispose ();}
    else if (command.equals ("Resize to fit")) {canvas.reFit ();}
    else if (command.equals ("Print")) {Print ();}
  }
  //end of event handler methods------------------------------------------------------
}





