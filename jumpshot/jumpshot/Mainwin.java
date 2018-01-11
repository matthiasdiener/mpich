package jumpshot;

import java.awt.*;
import java.util.*;
import java.awt.event.*;
import java.io.*;

/*
  This class supervises the reading and display of a clog file. 
  
  It shows a Frame consisting of (New Frame), (Select File) and (Exit) buttons 
  along with a text field showing the current filename. 
  
  1. The log file to be read is selected via FileDialog object (openFileDlg)
  2. An instance of ClogReader class is created with this filename which traverses
     the log file and supervises the creation of data structures.
  3. The data structures created reside in RecordHandler object which is returned.
  4. This class (Mainwin) then takes the RecordHandler object along with the 
     data structures creates an instance of ClogDisplay class which handles all
     drawing, printing, manipulation, etc.
*/
public class Mainwin extends Frame 
implements ActionListener {
  ClogDisplay disp;                          //Frame displaying all data                 
  
  String settingsFile = "Jumpshot.setup";    //file containing default settings          
  String distributionName = "jumpshot-1.0";  //Name of the distribution
  String distributionDir = ".";              //Directory where the distribution is
  String dataDir = "data";                   //Directory name where the ASCII file's are
  
  GridBagConstraints con;
  FileDialog openFileDlg;
  MyTextField logFileField;
  String logFileName;

  Font frameFont; 
  Color frameBColor, frameFColor;
  public Dimension dimPG;
  
  HelpDlg helpDlg;
  File helpFile;

  //This boolean value tells us if this instance of Mainwin is a child or not
  //If it is a child then when u exit it only its descendents will be terminated
  //If however, u exit the father all children will also be exited
  boolean child;                           
  
  // Constructor
  public Mainwin (String fileName, boolean c) {
    child = c;
    logFileName = fileName;
    if (logFileName == null) logFileName = new String ("No Name");
    setup ();
  }
  
  //setup methods-------------------------------------------------------------------
  void setup () {
    adjustFrameStuff ();
    setupData ();
    setupDlgs ();
    setupPanels ();
    setupEventHandlers ();
    pack (); 
    setVisible (true);
    if (!(logFileName.equals ("No Name"))) readLogFile (logFileName);
  }
  
  void adjustFrameStuff () {
    if (child) setTitle ("Jumpshot: (Child)");
    else setTitle ("Jumpshot");
    Toolkit toolkit = getToolkit ();
    Dimension dimScreen = new Dimension (toolkit.getScreenSize ());
    dimPG = new Dimension ((int)Math.rint (dimScreen.width * 0.75), 
                           (int)Math.rint (dimScreen.height * 0.5));
    setBackground (frameBColor = new Color (0, 74, 130));
    setForeground (frameFColor = Color.white);
    setFont (frameFont = new Font ("SansSerif", Font.BOLD, 14));
  }
  
  void setupData () {
    //Obtain the settings specified by the user.
    Properties settings = new Properties ();
    File setFile = getSettingsFile ();
    try {
      
      FileInputStream in = new FileInputStream (setFile);
      settings.load (in);
    }
    catch (FileNotFoundException e) {
      new ErrorDiag (this, "Settings File " + setFile + " cannot be found."
                     + " Proceeding without it.");
    }
    catch (IOException e) {
      new ErrorDiag (this, "Error reading Settings File " + setFile);
    }
    
    COLOR_UTIL.readColors (this, 
                           getDataFile (settings.getProperty ("COLORFILE", "Jumpshot.colors")));
    helpFile = getDataFile (settings.getProperty ("HELPFILE", "Jumpshot.help"));
  }
  
  void setupDlgs () {
    //A FileDialog is being created here. We can't set the filename filter
    //using the function setFilenameFilter as it does not work in 1.1
    openFileDlg = new FileDialog (this, "select file", FileDialog.LOAD);
    helpDlg = new HelpDlg ((Frame)this, readLines (helpFile));
  }
  
  void setupPanels () {
    setLayout (new GridBagLayout ());
    con = new GridBagConstraints ();
    con.fill = GridBagConstraints.BOTH;
    con.weightx = 1.0; con.weighty = 1.0;
    
    con.gridx = 0; con.gridy = 0; add (new MyButton ("New Frame", this), con);
    con.gridx = 1; add (new MyButton ("Select File", this), con);
    con.gridx = 2; add (new MyButton ("Exit", this), con);
    con.gridx = 3; add (new MyButton ("Help", this), con);
    
    con.gridx = 0; con.gridy = 1; 
    con.gridheight = GridBagConstraints.REMAINDER; 
    add (new Label ("Logfile:"), con);
    
    con.gridx = 1; con.gridwidth = GridBagConstraints.REMAINDER;
    add (logFileField = new MyTextField (logFileName, false), con);
  }
  
  void setupEventHandlers () {
    this.enableEvents (AWTEvent.ACTION_EVENT_MASK);
    
    // Define, instantiate and register a WindowListener object.
    addWindowListener (new WindowAdapter () {
      public void windowClosing (WindowEvent e) {close ();}
    });
  }
  
  //End of setup methods---------------------------------------------------------
  
  //Event Handler methods--------------------------------------------------------
  //Only events from buttons are generated
  
  //Handles action events generated from buttons
  public void actionPerformed (ActionEvent evt) {
    String command = evt.getActionCommand ();
    if (command.equals ("New Frame")) new Mainwin (null, true);
    else if (command.equals ("Select File")) selectFile ();
    else if (command.equals ("Exit")) close ();
    else if (command.equals ("Help")) helpDlg.setVisible (true);
  }
  //End of event Handler methods-------------------------------------------------

  //selects the log file to be read
  void selectFile () {
    openFileDlg.show ();
    waitCursor ();
    String dir = openFileDlg.getDirectory (), file = openFileDlg.getFile ();
    if (dir != null && file != null) {
      logFileName = dir + file;
      remove (logFileField);
      add (logFileField = new MyTextField (logFileName, false), con);
      pack (); //This repaints the updated logFileField on the Frame.
      readLogFile (logFileName);
      normalCursor ();
     }
    else normalCursor ();
   }
  
  //reads a log file by creating a ClogReader instance 
  void readLogFile (String inFile) {
    waitCursor ();
    free ();
    RecordHandler mainTool = (new ClogReader (this)).ReadClogFile (this);
    if (mainTool != null)
      disp = new ClogDisplay (this, mainTool);
    normalCursor ();
  }
  
  //Destructor
  void close () {
    free (); 
    this.dispose ();
    if (!child) System.exit (0);
  }
  
  //frees up the memory
  void free () {
    if (disp != null) {disp.free (); disp = null;}
    System.gc ();
  } 
  
  //sets the current cursor to WAIT_CURSOR type 
  void waitCursor () {setCursor (new Cursor (Frame.WAIT_CURSOR));}
  
  //sets the WAIT_CURSOR to cursor associated with this frame 
  void normalCursor () {setCursor (new Cursor (Frame.DEFAULT_CURSOR));}

  //method reads all the lines of text from the given file. All lines are
  //stored in one string seperated by '/n's.
  String readLines (File file) {
    DataInputStream in = null;
    try{in = new DataInputStream (new FileInputStream (file));}
    catch (IOException x) {
      new ErrorDiag (this, "File " + file.toString () + " could not be opened");
      return null;
    }
    
    String line, text = "";
    try {while ((line = in.readLine ()) != null) text += (line + "\n");}
    catch (IOException x) {
      new ErrorDiag (this, "IO Exception occured while reading " + file.toString ());
      return text;
    }
    return text;
  }

  //This method will get an handle to the Settings file. The settings file usually
  //Jumpshot.setup is in the directory ...jumpshot-1.0/data. From the CLASSPATH
  //environmental variable I hope to extract the absolute path of Jumpshot.setup
  File getSettingsFile () {
    Properties systemStuff = System.getProperties ();
    
    String CLASSPATH = systemStuff.getProperty ("java.class.path");
    
    StringTokenizer tokens = new StringTokenizer (CLASSPATH, File.pathSeparator);
    
    int ct = tokens.countTokens ();

    for (int i = 0; i < ct; i++) {
      String dir = tokens.nextToken ();
      if (dir.endsWith (distributionName)) {
        distributionDir = dir;
        break;
      }
    }
    return new File (distributionDir + File.separator + dataDir, settingsFile);
  }
  
  File getDataFile (String s) {
    return new File (distributionDir + File.separator + dataDir, s);
  } 
}










