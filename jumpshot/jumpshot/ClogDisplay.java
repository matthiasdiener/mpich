package jumpshot;

import java.awt.*;
import java.awt.event.*;
import java.util.*;

/*
  This class supervises all display operations and printing for the given
  Jumpshot data. 
  
  It shows a frame consisting of buttons (Options), (Print), (Refresh), 
  (Close), (In), (Out), etc. and textFields including time cursor and
  elapsed time. It also contains the canvas showing the different states
  
  Once a clog file is read by ClogReader, the data structures are passed
  onto this class which then produces the display.
*/
public class ClogDisplay extends Frame 
implements AdjustmentListener, ComponentListener, ActionListener {
  public Mainwin parent;
 
  //Data Structures
  public Vector stateDefs;      //List of state definitions
  public Vector data;           //List of all states of all process's
  public CLOG_ARROW quiver;     //Object containing list of all arrows

  Properties printPrefs;        //stores the default settings from printing 
  
  //Dialogs and frames
  ZoomDlg zoomDlg;              //dialog containing zoom options
  PrintDlg printDlg;            //dialog containing print options 
  StateButtons stateButtons;    //frame containing state buttons
  CanOptions optionsDlg;        //dialog containing options
  public ProcDlg procDlg;       //dialog containing process options
  public StateDlg stateDlg;     //dialog containing state information
  public MsgDlg msgDlg;         //dialog containing message information

  //canvas's
  public ProgramCanvas canvas;  //canvas where states are drawn
  VertScaleCanvas vcanvas1, vcanvas2; //canvas's where process numbers are drawn
  
  ScrollPane paneC;             //object that contains the canvas for smooth scrolling 
  Scrollbar hbar;               //scrollbar that controls all scrolling
  Panel mainPanel;              
  MyTextField timeField, elTimeField;
  
  Font frameFont, printFont;
  Color frameBColor, frameFColor, printBColor, printFColor;
  Color printImgBColor, normImgBColor;

  boolean setupComplete = false;
  boolean printStatus;

  //constructor
  public ClogDisplay (Mainwin p, RecordHandler mainTool) {
    super ("TimeLines");
    parent = p;
    
    //Getting data structures from ClogReader
    stateDefs = mainTool.stateDefs;
    data = mainTool.data;
    quiver = mainTool.a;
    
    setup ();
  }
  
  //Setup methods------------------------------------------------------------------------------
  public void setup () {
    adjustFrameStuff ();
    setupCanvas ();
    setupPanels ();
    setupScrollPanes ();
    setupScrollbars ();
    setupBugMsg ();
    setupDlgs ();
    setupStateTypes ();
    setupPrintPrefs ();
    setVisible (true);
    
    //Before we draw the states on to the image, which is on the canvas the
    //frame needs to be made visible so that we get a graphics context to
    //draw upon. If the frame was not made visible a null graphics context
    //was returned
    waitCursor ();  
    drawData ();
    setupEventHandlers ();
    setupComplete = true;
    normalCursor ();
  }
  
  void adjustFrameStuff () {
    setBackground (frameBColor = parent.frameBColor);
    setForeground (frameFColor = parent.frameFColor);
    setFont (frameFont = parent.frameFont);
    printBColor = new Color (35, 129, 174);            //A lighter shade of blue
    printFColor = Color.white; 
    printImgBColor = Color.lightGray;
    normImgBColor = Color.black;
    printFont = new Font ("SansSerif", Font.BOLD, 14);
  }
  
  //Initialize the canvas's. canvas has to be non-null to be placed in a 
  //panel
  void setupCanvas () {
    canvas = new ProgramCanvas ();
    vcanvas1 = new VertScaleCanvas (this);
    vcanvas2 = new VertScaleCanvas (this);
  }
  
  public void setupPanels () {
    Panel controlPanel = new Panel ();
    controlPanel.setLayout (new GridBagLayout ());
    
    GridBagConstraints con = new GridBagConstraints (); 
    con.fill = GridBagConstraints.BOTH;
    con.weightx = 1.0; con.weighty = 1.0;
    
    con.gridx = 0; con.gridy = 0; 
    controlPanel.add (new Label ("Pointer:", Label.LEFT), con);
    con.gridx = 1; controlPanel.add (timeField = new MyTextField ("", 20, false), con); 
    con.gridx = 2; controlPanel.add (new MyButton ("Options", this), con);
    con.gridx = 3; controlPanel.add (new MyButton ("Print", this), con);
    con.gridx = 4; controlPanel.add (new MyButton ("States", this), con);
    con.gridx = 5; controlPanel.add (new MyButton ("Refresh", this), con);
    con.gridx = 6; controlPanel.add (new MyButton ("Close", this), con);

    
    con.gridx = 0; con.gridy = 1; controlPanel.add (new Label ("Elapsed Time:"), con);
    con.gridx = 1; controlPanel.add (elTimeField = new MyTextField ("", 20, false), con);
    con.gridx = 2; controlPanel.add (new Label ("Horizontal Zoom:"), con);
    con.gridx = 3; controlPanel.add (new MyButton ("In", this), con);
    con.gridx = 4; controlPanel.add (new MyButton ("Out", this), con);
    con.gridx = 5; controlPanel.add (new MyButton ("Configure Zoom", this), con);
    con.gridx = 6; controlPanel.add (new MyButton ("ResetView", this), con);

    add ("North", controlPanel);
    
    mainPanel = new Panel ();
    mainPanel.setLayout (new GridBagLayout ());
    
    GridBagConstraints cg = new GridBagConstraints ();
    cg.gridx = 1; cg.gridy = 0;
    cg.weightx = 1.0; cg.weighty = 1.0;
    cg.fill = GridBagConstraints.BOTH; cg.gridwidth = GridBagConstraints.RELATIVE;
    mainPanel.add (paneC = new ScrollPane (ScrollPane.SCROLLBARS_AS_NEEDED), cg);
    
    GridBagConstraints vg = new GridBagConstraints ();
    vg.gridx = 0; vg.gridy = 0; vg.fill = GridBagConstraints.BOTH;
    vg.weightx = 0; vg.weighty = 1.0;
    mainPanel.add (vcanvas1, vg);
    
    vg.gridx = 2; mainPanel.add (vcanvas2, vg);
    vg.gridwidth = GridBagConstraints.REMAINDER;
    add ("Center", mainPanel);
  }

  void setupScrollPanes () {
    paneC.setBackground (Color.black); //to hide scrollbars
    setSize (parent.dimPG);
    paneC.add (canvas);
  }
  
  public void setupScrollbars () {
    hbar = new Scrollbar (Scrollbar.HORIZONTAL); 
    hbar.setBackground (Color.black);
    
    GridBagConstraints hbg = new GridBagConstraints ();
    hbg.gridx = 1; hbg.gridy = 1; 
    hbg.weightx = 1.0; hbg.weighty = 0;
    hbg.fill = GridBagConstraints.BOTH;
    hbg.gridx = 1; hbg.gridy = 1; 
    hbg.weightx = 1.0; hbg.weighty = 0;
    hbg.fill = GridBagConstraints.BOTH;
    mainPanel.add (hbar, hbg);
  }
  
  public void setupBugMsg () {
    add ("South", new Label("Use scrollbar above to move forward or backward in time", Label.CENTER));  
  }

  public void setupEventHandlers () {
    hbar.addAdjustmentListener (this);
    this.addComponentListener (this);
    
    // Define, instantiate and register a WindowListener object.
    addWindowListener (new WindowAdapter () {
      public void windowClosing (WindowEvent e) {free ();}
    });
  }
  
  void setupDlgs () {
    zoomDlg = new ZoomDlg (this);
    optionsDlg = new CanOptions (this);
    printDlg = new PrintDlg (this);
    stateDlg = new StateDlg ((Frame)this);
    msgDlg = new MsgDlg ((Frame)this);
  }   
  
  void setupStateTypes () {stateButtons = new StateButtons (this, 4, 4);}
   
  //printPrefs is the property set that stores the default settings for
  //printing.
  void setupPrintPrefs () {
    printPrefs = new Properties ();
    printPrefs.put ("awt.print.fileName", "jumpshot.ps");
    printPrefs.put ("awt.print.numCopies", "1");
    printPrefs.put ("awt.print.paperSize", "letter");
    printPrefs.put ("awt.print.destination", "file");
    printPrefs.put ("awt.print.orientation", "landscape");
  }
  
  //End of Setup methods--------------------------------------------------------------------------
  
  //Event Handler methods---------------------------------------------------------------------
  //Events may be generated when the scrollbar is modified, the component is 
  //resized, or if one of the buttons is pressed
  
  //Handles the event when the scrollbar is modified
  public void adjustmentValueChanged (AdjustmentEvent e) {setPosition (hbar.getValue ());}
  
  void setPosition (int x) {
    canvas.adjustImgH (x); 
    paneC.setScrollPosition (canvas.getPanePosX (), 0);
  }
  
  //Handle the event when the frame is resized
  public void componentResized (ComponentEvent e) {
    waitCursor (); 
    if (setupComplete) canvas.Resize (); 
    normalCursor ();
  } 
  
  //Handler the event when one of the buttons is pressed 
  public void actionPerformed (ActionEvent evt) {
    String command = evt.getActionCommand ();
    if (setupComplete) {
      if (command.equals ("States")) {stateButtons.show (); stateButtons.toFront ();}
      else if (command.equals ("In")) zoomInH ();
      else if (command.equals ("Out")) zoomOutH ();
      else if (command.equals ("ResetView")) resetView ();
      else if (command.equals ("Refresh")) canvas.Refresh ();
      else if (command.equals ("Options")) {
        optionsDlg.reset ();
        optionsDlg.show (); 
        optionsDlg.toFront ();
      }
      else if (command.equals ("Configure Zoom")) {
        zoomDlg.show (); 
        zoomDlg.toFront ();
      }
      else if (command.equals ("Print")) Print ();
      else if (command.equals ("Close")) free ();
    }
  }

  //End of event Handler methods--------------------------------------------
  
  //draw the states on to the canvas
  void drawData () {
    canvas.init (this);
    vcanvas1.repaint ();
    vcanvas2.repaint ();
    procDlg = new ProcDlg (this);
  }  
  
  //Zoom methods------------------------------------------------------------
  //zoom in horizontally using the current zoom factor along zoomlock if any.
  void zoomInH () {
    waitCursor ();
    canvas.zoomInH ();
    zoomH ();
    normalCursor ();
  }
  
  //zoom out horizontally using the current zoom factor along zoomlock if any.
  void zoomOutH () {
    waitCursor ();
    canvas.zoomOutH ();
    zoomH ();
    normalCursor ();
  }   

  //Reset view so that the entire data occupies one screen
  void resetView () {
    waitCursor ();
    canvas.resetView ();
    zoomH (); 
    normalCursor ();
  }
  
  //Method called whenever a horizontal zoom takes place
  void zoomH () {
    setHoriz (hbar);
    paneC.doLayout ();
    hbar.setValue (canvas.sbPos);
    paneC.setScrollPosition (canvas.getPanePosX (), 0);
  }
  
  //Reset scrollbar values when horizontal extent of data changes (e.g during zoom)
  public void setHoriz (Scrollbar hbar) {
    hbar.setMaximum (canvas.maxH);
    hbar.setMinimum (0);
    hbar.setVisibleAmount ((int)Math.rint (canvas._xPix / 3.0));
    hbar.setUnitIncrement ((int)Math.rint (1));
    hbar.setBlockIncrement ((int)Math.rint (canvas._xPix / 3.0));
  }
  //--------------------------------------------------------------------------
  
  //modify the cursor value in the time field
  public void adjustTimeField (double d) {
    timeField.setText ((new Float (d)).toString ());
  }
  
  //modify time values of start and end positions of the screen in
  //the zoom Dialog
  public void adjustExTimes (double beg, double end) {
    zoomDlg.fromTime (beg);
    zoomDlg.toTime (end);
  }

  //Modify the elapsed time field value
  public void adjustElTimeField (double d) {
    elTimeField.setText ((new Float (d)).toString ());
  }
   
  //change the zoom factor
  public void adjustZF (double d) {zoomDlg.zoomF (d);}
  
  // Prints out jumpshot data
  void Print () {
    waitCursor ();
    
    String jobTitle = "Jumpshot:" + parent.logFileName;
    
    //Obtain a PrintJob Object. This posts a Print dialog
    //printprefs stores user printing prefrences
    PrintJob pjob = getToolkit ().getPrintJob (this, jobTitle, printPrefs);
    
    //If the user clicked Cancel in the print dialog, then do nothing
    if (pjob != null) {
      printDlg.reset (pjob, this);
      canvas.Refresh ();
    }

    normalCursor ();
  }
  
  //sets the current cursor to WAIT_CURSOR type
  public void waitCursor () {setCursor (new Cursor (Frame.WAIT_CURSOR));} 
  
  //sets the WAIT_CURSOR to cursor associated with this frame
  public void normalCursor () {setCursor (new Cursor (Frame.DEFAULT_CURSOR));}
  
  //destructor
  void free () {
    canvas = null;
    vcanvas1 = null;
    vcanvas2 = null;
    if (stateButtons != null) {stateButtons.dispose (); stateButtons = null;}
    if (zoomDlg != null) {zoomDlg.dispose (); zoomDlg = null;}
    stateDefs = null;
    this.dispose ();
  }
  
  //Unused methods of the ComponentListener interface
  public void componentHidden (ComponentEvent e) {;}
  public void componentMoved (ComponentEvent e) {;}
  public void componentShown (ComponentEvent e) {;}
}
















