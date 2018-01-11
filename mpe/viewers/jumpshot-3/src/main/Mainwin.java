import java.awt.*;
import java.util.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import java.rmi.*;
import javax.swing.*;
import javax.swing.border.*;

/*
  This class supervises the reading and display of a slog file. 
  
  It shows a Frame consisting of (New Frame), (Select File) and (Exit) buttons 
  along with a text field showing the current filename. 
  
  1. The log file to be read is selected via FileDialog object (openAppFileDlg)
  2. An instance of FrameReader class is created with this filename which
     traverses the log file and supervises the creation of data structures.
  3. The data structures created reside in RecordHandler object which is
     returned.
  4. This class (Mainwin) then takes the RecordHandler object along with the 
     data structures creates an instance of FrameDisplay class which handles all
     drawing, printing, manipulation, etc.
*/

public class Mainwin extends JPanel 
                     implements ActionListener
{
          SLOG_ProxyInputStream   slog = null;
          ViewFrameChooser        frame_chooser;

  private SwingWorker readWorker;           //Thread doing reading
  
  // Relative Path to the files for the GUI system's configuration 
  private String       configDefaultFile = "etc/jumpshot.conf";
  private String       colorDefaultFile = "share/jumpshot.colors";
  private String       tourDefaultFile = "doc/html/index.html";
  private String       buttonDefaultFile = "doc/jumpshot.def";
  private String       configFile = configDefaultFile;
  private String       colorFile;
  private String       tourHTMLFile;
  private String       btnsDefnFile;
  //Directory to the sample logfiles
  private String       logFileDir = "logfiles";
  
  // private Object       openAppFileDlg;
  private JFileChooser openAppFileDlg;
  private ApltFileDlg  openApltFileDlg;
  private MyTextField  logFileField;
  private JMenuItem    metalMenuItem, selectFileMenuItem;
          String       logFileName;
  
          Font         frameFont; 
          Color        frameBColor, frameFColor;
  public  Dimension    dimPG;
  
  private HTMLviewer   btns_viewer;
  private HTMLviewer   tour_viewer;
  private JMenuBar     menuBar;
          Mainwin      startwin;
  
  //This boolean value tells us if this instance of Mainwin is a child or not
  //If it is a child then when u exit it only its descendents will be terminated
  //If however, u exit the father all children will also be exited
  private boolean      isChild;
  
          Component    parent;
  private String       parent_superclass;
  
  private boolean      isApplet;
          int          dtype;
  
  private String       about = "Jumpshot, the SLOG viewer, Version 3.0\n"
                             + "Questions: mpi-maint@mcs.anl.gov";
  
  private MyButton     read_btn;
  private boolean      reader_alive;
  

  public Mainwin( Component origin, boolean is_child,
                  String filename, int frame_idx )
  {
      parent = origin;
      parent_superclass = parent.getClass().getSuperclass().getName();
      isApplet = parent_superclass.equals( "javax.swing.JApplet" );
      isChild = is_child;
      startwin = this;
      logFileName = filename;

      slog = null;
      /*
      if ( frame_idx >= 0 )
          slog_frame_idx = frame_idx;
      else
          slog_frame_idx = 0;
      */

      if ( logFileName == null )
          logFileName = new String( "No Name" );

      setup();
  }
  
  public boolean getIsApplet()
  {   return isApplet;  }

  //setup methods-----------------------------------------------------------------
  private void setup () 
  {
    setupUI();
    adjustFrameStuff();
    setupData();
    // setupDlgs();
    setupPanels();
    disableRead();
    setupEventHandlers();
    // setSize( getMinimumSize() ); setResizable( false );
    setVisible(true);
    if ( ! logFileName.equals( "No Name" ) ) {
        readLogFile();
    }
  }
  
  private void setupUI () {
    // Force SwingSet to come up in the Cross Platform L&F
    try {
      UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
      // If you want the System L&F instead, comment out the above line and
      // uncomment the following:
      // UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
    } catch (Exception exc) {
      new ErrorDialog (null, "Error loading L&F: " + exc);
    }
  }
  
  private void adjustFrameStuff () {
    if ( isChild )
        ( (MainFrame)parent ).setTitle( "Jumpshot-3: Child" );
    Dimension dimScreen = Toolkit.getDefaultToolkit().getScreenSize();
    dimPG = new Dimension ((int)Math.rint (dimScreen.width * 0.75), 
                           (int)Math.rint (dimScreen.height * 0.7));
    frameBColor = Color.lightGray;
    frameFColor = Color.black;
    frameFont = new Font ("Serif", Font.PLAIN, 10);
  }
  
private void setupData()
{
    //Obtain the settings specified by the user.
    Properties settings = new Properties();
    URL setup_URL, color_URL;
    boolean useDefaultSettings;
    InputStream setin, colorin;
    
    setup_URL = getURL( configFile );
    if ( setup_URL != null ) {
        try {
            setin = setup_URL.openStream();
            settings.load( setin );
            setin.close(); 
        }
        catch ( IOException err ) {
            new WarnDialog( this, "IO Error:" + err.getMessage() + " "
                          + "in reading the setup file. Use default settings" );
            useDefaultSettings = true;
        }
        useDefaultSettings = false;
    }
    else {
        new WarnDialog( this, "Cannot locate setup file " + configFile + ". "
                            + "Use default settings." );
        useDefaultSettings = true;
    }
    
    // Load the settings if possible
    if ( useDefaultSettings ) {
        colorFile    = colorDefaultFile;
        tourHTMLFile = tourDefaultFile;
        btnsDefnFile = buttonDefaultFile;
    }
    else {
        colorFile    = settings.getProperty( "COLORFILE", colorDefaultFile );
        tourHTMLFile = settings.getProperty( "HTMLFILE", tourDefaultFile );
        btnsDefnFile = settings.getProperty( "BUTTONFILE", buttonDefaultFile );
    }

    color_URL = getURL( colorFile );
    if ( color_URL == null ) {
        new ErrorDialog( this, "Cannot locate " + colorFile + ". Exiting!" );
        close();
    }

    colorin = null;
    try { colorin = color_URL.openStream(); }
    catch ( IOException err ) {
        new ErrorDialog( this, "IO Error:" + err.getMessage() + ". "
                       + "Cannot open " + colorFile + ". Exiting!" );
        close();
    }
    ColorUtil.readColors( this, colorin );
    
    reader_alive = false;
}
    
  
/*
private void setupDlgs()
{
    URL init_URL = getURL( tourHTMLFile );
    if ( init_URL != null ) 
        tour_viewer = new HTMLviewer( init_URL );
    else
        tour_viewer = null;

    URL btns_URL = getURL( btnsDefnFile );
    if ( btns_URL != null )
        btns_viewer = new HTMLviewer( btns_URL );
    else
        btns_viewer = null;
}
*/
  
private void setupPanels()
{
    setLayout( new GridBagLayout() );
    GridBagConstraints con = new GridBagConstraints();
    con.anchor = GridBagConstraints.WEST; 
    con.fill = GridBagConstraints.HORIZONTAL;
    
    add( setupMenuBar(), con );
    
    con.fill = GridBagConstraints.NONE;
    //JPanels are used now. But, if the problem with using menus is solved
    //they should be used instead
    
    Border border1, border2 = BorderFactory.createLoweredBevelBorder();
    
    //Logfile field
    JPanel pl = new JPanel( new FlowLayout() );
    border1 = BorderFactory.createEmptyBorder( 4, 4, 2, 4 );
    pl.setBorder( BorderFactory.createCompoundBorder( border1, border2 ) );

    pl.add( new JLabel( "Logfile" ) ); 
    
    logFileField = new MyTextField( logFileName, 35, false );
    pl.add( logFileField );
    
    con.gridy = 1; 
    add( pl, con );
    
    con.gridy = 2; con.gridx = 0;
    read_btn = new MyButton( "Read", "Reading the selected logfile", this );
    add( read_btn, con );
}
  
  private JMenuBar setupMenuBar () {
    menuBar = new JMenuBar();
    
    //File Menu
    JMenu file = (JMenu) menuBar.add(new JMenu("File")); 
    
    JMenuItem menuItem = file.add (new JMenuItem ("New Frame"));
    menuItem.addActionListener (this);
    menuItem.setHorizontalTextPosition (JButton.RIGHT);
    
    selectFileMenuItem = new JMenuItem ("Select Logfile");
    file.add (selectFileMenuItem);
    selectFileMenuItem.addActionListener (this);  
    selectFileMenuItem.setHorizontalTextPosition (JButton.RIGHT);
    
    menuItem = file.add (new JMenuItem ("Exit"));
    menuItem.addActionListener (this); 
    menuItem.setHorizontalTextPosition (JButton.RIGHT);
    
    menuBar.add (file);
    
    //Display Menu
    JMenu dispM = (JMenu) menuBar.add(new JMenu("Display"));
    
    ButtonGroup dGroup = new ButtonGroup();
    DispListener dispListener = new DispListener();
    
    JRadioButtonMenuItem rbM;
    rbM = (JRadioButtonMenuItem)
          dispM.add(new JRadioButtonMenuItem("Time Lines"));
    dGroup.add(rbM); rbM.addItemListener(dispListener);
    rbM.setSelected (true);
    dtype = CONST.TIMELINES; //Setting dtype to time lines
    
    rbM = (JRadioButtonMenuItem)
          dispM.add(new JRadioButtonMenuItem("Mountain Ranges"));
    dGroup.add(rbM); rbM.addItemListener(dispListener);
    //  disable the MountainRanges option
    rbM.setEnabled( false );
    
    menuBar.add (dispM);
    
    // Sys Options Menu
    JMenu options = (JMenu) menuBar.add(new JMenu("System"));
    
    // Look and Feel Radio control
    ButtonGroup group = new ButtonGroup();
    
    ToggleUIListener toggleUIListener = new ToggleUIListener();
    
    JMenuItem windowsMenuItem, motifMenuItem, macMenuItem;
    
    windowsMenuItem = (JRadioButtonMenuItem)
          options.add(new JRadioButtonMenuItem("Windows Style Look and Feel"));
    windowsMenuItem.setSelected(
                    UIManager.getLookAndFeel().getName().equals("Windows") );
    group.add(windowsMenuItem);
    windowsMenuItem.addItemListener(toggleUIListener);
    
    motifMenuItem = (JRadioButtonMenuItem)
          options.add(new JRadioButtonMenuItem("Motif Look and Feel"));
    motifMenuItem.setSelected(
          UIManager.getLookAndFeel().getName().equals("CDE/Motif"));
    group.add(motifMenuItem);
    motifMenuItem.addItemListener(toggleUIListener);
    
    metalMenuItem = (JRadioButtonMenuItem)
          options.add(new JRadioButtonMenuItem("Metal Look and Feel"));
    metalMenuItem.setSelected(
          UIManager.getLookAndFeel().getName().equals("Metal"));
    metalMenuItem.setSelected(true);
    group.add(metalMenuItem);
    metalMenuItem.addItemListener(toggleUIListener);

    macMenuItem = (JRadioButtonMenuItem)
          options.add(new JRadioButtonMenuItem("Macintosh Look and Feel"));
    macMenuItem.setSelected(
          UIManager.getLookAndFeel().getName().equals("Macintosh"));
    group.add(macMenuItem);
    macMenuItem.addItemListener(toggleUIListener);

    // Tooltip checkbox
    options.add(new JSeparator());
    
    JCheckBoxMenuItem cb = new JCheckBoxMenuItem ("Show ToolTips");
    options.add(cb); cb.setSelected(true);
    
    cb.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
	JCheckBoxMenuItem b = (JCheckBoxMenuItem)e.getSource();
	if(b.isSelected()) {
	  ToolTipManager.sharedInstance().setEnabled(true);
	} else {
	  ToolTipManager.sharedInstance().setEnabled(false);
	}
      }
    });
    
    //Help Menu
    JMenu helps = (JMenu) menuBar.add(new JMenu("Help"));
    
    menuItem = helps.add (new JMenuItem ("Manual"));
    menuItem.addActionListener (this);
    
    menuItem = helps.add (new JMenuItem ("Tour"));
    menuItem.addActionListener (this);
    
    menuItem = helps.add (new JMenuItem ("About"));
    menuItem.addActionListener (this);
    
    menuBar.add (helps);
    
    return menuBar;
  }
  
  private void setupEventHandlers()
  {
    this.enableEvents (AWTEvent.ACTION_EVENT_MASK);
  }
  
  //End of setup methods-------------------------------------------------------
  
  //Event Handler methods------------------------------------------------------
  //Only events from buttons are generated
  
  /**
   * Handles action events generated from buttons
   */
  public void actionPerformed ( ActionEvent evt )
  {
      String command = evt.getActionCommand();
      if ( command.equals( "New Frame" ) )
          new MainFrame( true, null, 0 );
      else if ( command.equals( "Select Logfile" ) )
          selectFile();
      else if ( command.equals( "Exit" ) )
          close();
      else if ( command.equals( "Manual" ) ) {
          URL btns_URL = getURL( btnsDefnFile );
          if ( btns_URL != null ) {
              btns_viewer = new HTMLviewer( btns_URL );
              btns_viewer.setVisible( true );
          }
          else
              new WarnDialog( this, "Cannot locate " + btnsDefnFile + "." );
      }
      else if ( command.equals( "Tour" ) ) {
          URL init_URL = getURL( tourHTMLFile );
          if ( init_URL != null ) {
              tour_viewer = new HTMLviewer( init_URL );
              tour_viewer.setVisible( true );
          }
          else
              new WarnDialog( this, "Cannot locate " + tourHTMLFile + "." );
      }
      else if ( command.equals( "About" ) ) {
          URL icon_URL = getURL( "images/jumpshot.gif" );
          if ( icon_URL != null ) {
              ImageIcon js_icon = new ImageIcon( icon_URL );
              JOptionPane.showMessageDialog( this, about, "About",
                                             JOptionPane.INFORMATION_MESSAGE,
                                             js_icon );
          }
          else
              JOptionPane.showMessageDialog( this, about, "About",
                                             JOptionPane.INFORMATION_MESSAGE );
      }
      else if (command.equals( "Read" ) ) {
          readLogFile();
      }
  }

  //End of event Handler methods----------------------------------------------
  
  private void disableRead()
  {
      read_btn.setEnabled(false);
  }
  
  // private void enableRead () {read_btn.setEnabled (true);}
  public void enableRead()
  {
      read_btn.setEnabled(true);
  }
  
  /**
   * selects the log file to be read
   */
//Bug: Method setCurrentDirectory of JFileChooser does not work completely.
//     So, method rescanCurrentDirectory has to be called also to get the 
//     desired effect. We want the JFileChooser dialog to open up with a 
//     listing of files of the directory from which Jumpshot was executed
  private void selectFile () 
  {
    if ( !isApplet ) { //Application - Read file from machine's memory
        openAppFileDlg = new JFileChooser();

        if ( openAppFileDlg != null ) {
            openAppFileDlg.setDialogTitle( "Select Logfile" );
	        File f = new File( (System.getProperties())
                               .getProperty( "user.dir" ) );
            openAppFileDlg.setCurrentDirectory( f );
            openAppFileDlg.rescanCurrentDirectory();
        }
        else
            selectFileMenuItem.setEnabled( false );

        int r = openAppFileDlg.showOpenDialog( this );
        if ( r == 0 ) {
            String file;
            File f = openAppFileDlg.getSelectedFile();

            if ( f != null )
                file = f.toString ();
            else
                file = null;
	
            if ( file != null ) {
                logFileName = file;
                logFileField.setText( logFileName );
                readLogFile();
            }
        }
        else
            JOptionPane.showMessageDialog( this, "No file chosen" );
    }
    else { //Applet - Read file from host machine's memory 
        MainApplet top_applet = (MainApplet) parent;
        openApltFileDlg = new ApltFileDlg( top_applet, "Select Logfile" );
        openApltFileDlg.show();
        if ( openApltFileDlg.getSelected() ) {
            String file = openApltFileDlg.getFile();
            if ( file != null ) {
                logFileField.setText( file );
                logFileName = top_applet.GetLogFileDirName() + "/" + file;
                String prefix = top_applet.GetLogFilePathPrefix();
                if ( prefix != null && prefix.length() > 0 )
                    logFileName = prefix + "/" + logFileName ;
                top_applet.showStatus( "Mainwin: After openApltFileDlg.show(), "
                                     + "logFileName = " + logFileName + ",  "
                                     + "file = " + file );
                readLogFile();
            }
        }
        else
            JOptionPane.showMessageDialog( this, "No file chosen" );
    }
  }
  
    /**
     * reads a log file by creating a FrameReader instance 
     */
    private void readLogFile()
    {
        // if ( ChkFileExist( logFileName ) ) {
        if ( true ) {
            // Clean up and free resources
            freeMem();

            // Spawn new thread, SwingWorker, to read the SLOG file header
            readWorker = new SwingWorker()
            {
                public Object construct() 
                {
                    MainApplet top_applet;
                    String serviceURLname;
                    String locationURL;

                    waitCursor();
                    disableRead();
                    reader_alive = true;
                    if ( isApplet ) {
                        top_applet = (MainApplet) parent;
                        serviceURLname = top_applet.GetServiceURLname();
                        locationURL    = serviceURLname;
                    }
                    else {
                        serviceURLname = null;
                        locationURL = "local filesystem";
                    }
                    /*
                    top_applet.showStatus( "ServiceURLname = "
                                         + top_applet.GetServiceURLname() );
                    */
                    try {
                        return ( new SLOG_ProxyInputStream( serviceURLname,
                                                            logFileName )
                               );
                    }
                    catch ( NotBoundException err ) {
                            new ErrorDialog( startwin,
                                             "NotBoundException: ServiceURL = "
                                           + serviceURLname );
                        return null;
                    }
                    catch ( FileNotFoundException err ) {
                            new ErrorDialog( startwin,
                                             "FileNotFoundException: File = "
                                           + logFileName + " at "
                                           + locationURL );
                        return null;
                    }
                    catch ( IOException err ) {
                            new ErrorDialog( startwin,
                                             "IOException: Reading file = "
                                           + logFileName + " fails at "
                                           + locationURL );
                        return null;
                    }
                }
                public void finished()
                {
                    reader_alive = false;
                    slog = ( SLOG_ProxyInputStream ) get();

                    if ( slog != null ) {
                        //  The btn_names[] needs to be synchronized with
                        //  SetIndexes() of StateGroupLabel class 
                        String[] btn_names = { "MPI-Process", "Thread",
                                               "Processor" };
                        frame_chooser = new ViewFrameChooser( slog, btn_names );
                        frame_chooser.SetInitWindow( startwin );
                        frame_chooser.pack();
                        frame_chooser.setVisible( true );
                    }

                    normalCursor();
                }
            };

            // Start the readWorker thread after the thread has been created.
            // It is done to avoid race condition in SwingWorker2. 
            readWorker.start();

        }
    }

    private boolean ChkFileExist( String filename )
    {
        InputStream in;

        if ( isApplet ) {
            URL url = null;
            try { url = new URL( filename ); }
            catch ( MalformedURLException e ) {
                new ErrorDialog(this, "Mainwin: ChkFileExist(): Bad URL: "
                                    + url + ", filename = " + filename );
                return false;
            }

            try { in = url.openStream(); }
            catch ( IOException e ) {
                new ErrorDialog(this, "IO Error:" + e.getMessage ());
                return false;
            }
        }
        else {
            try { in = new FileInputStream( filename ); }
            catch ( FileNotFoundException e ) { 
                  new ErrorDialog( this, "file: " + filename + " not found." );
                  return false;
            }
        }
        return true;
    }


  /**
   * Destructor
   */
  private void close ()
  {
    freeMem(); 
    this.setVisible( false );
    parent.setVisible( false );
    //    System.runFinalization (); System.gc ();
    if (!isChild && !isApplet) System.exit (0);
  }
  
  /**
   * frees up the memory - reader and display
   */
  private void freeMem () {
    if ( readWorker != null && reader_alive ) {
        readWorker.interrupt();
        readWorker = null;
    }
    // if ( disp != null ) disp.kill();
    //    System.runFinalization (); System.gc ();
    if ( frame_chooser != null ) frame_chooser = null;
  } 
  
  /**
   * sets the current cursor to DEFAULT_CURSOR
   */
  void normalCursor () {
    ROUTINES.makeCursor (this, new Cursor (Cursor.DEFAULT_CURSOR));
  }
  
  /**
   * sets the current cursor to WAIT_CURSOR
   */
  void waitCursor () {
    ROUTINES.makeCursor (this, new Cursor (Cursor.WAIT_CURSOR));
  }
    
  /**
   * method reads all the lines of text from the given file. All lines are
   * stored in one string separated by '/n's.
   */
  private String readLines (InputStream file) {
    if (file == null) return "";

    BufferedReader in = new BufferedReader (new InputStreamReader (file));
    String line, text = "";
    
    try {
      while ((line = in.readLine ()) != null) text += (line + "\n");
      file.close ();
    }
    catch (IOException x) 
        {new ErrorDialog (this, "IO Error:" + x.getMessage ());}

    return text;
  }
  
  private InputStream getFileIn (String name) {
    InputStream in = null;
    
    try {in = new FileInputStream (name);}
    catch (FileNotFoundException e) {
      new ErrorDialog (this, "file: " + name + " not found.");
      return null;
    }
    return in;
  }
  
  private InputStream getUrlIn (String name) {
    URL url = null;
    InputStream in = null;
    
    try {url = new URL (name);}
    catch (MalformedURLException e) {
      new ErrorDialog (this, "Mainwin: getUrlIn: Bad URL:" + url);
      return null;
    }
   
    try {in = url.openStream ();}
    catch (IOException e) {
      new ErrorDialog (this, "IO Error:" + e.getMessage ());
      return null;
    }
    return in;
  }
 
   
    private URL getURL(String filename)
    {
        URL url = null;

        /*
        if ( isApplet ) {
            URL codeBase = ( (JApplet) parent ).getCodeBase();
            try {
                url = new URL( codeBase, filename );
            } catch ( java.net.MalformedURLException e ) {
                System.out.println("badly specified URL");
                return null;
            }
        }
        else
           url = ClassLoader.getSystemResource( filename );
        */

        url = getClass().getResource( filename );

        return url;
    }

  
  void delDisp () {
    enableRead (); 
    //    disp = null;
    //    System.runFinalization (); System.gc ();
  }
  
  /**
   * Switch the between the Windows, Motif, Mac, and the Metal Look and Feel
   */
  private class ToggleUIListener implements ItemListener {
    public void itemStateChanged(ItemEvent e) {
      waitCursor ();

      JRadioButtonMenuItem rb = (JRadioButtonMenuItem) e.getSource();
      try {
	if (   rb.isSelected()
            && rb.getText().equals("Windows Style Look and Feel")) {
	  UIManager.setLookAndFeel( "com.sun.java.swing.plaf"
                                  + ".windows.WindowsLookAndFeel");
	  makeUIChanges ();
	} 
	else if (   rb.isSelected()
                 && rb.getText().equals("Macintosh Look and Feel")) {
	  UIManager.setLookAndFeel( "com.sun.java.swing.plaf"
                                  + ".mac.MacLookAndFeel" );
	  makeUIChanges ();
	} 
	else if(rb.isSelected() && rb.getText().equals("Motif Look and Feel")) {
	  UIManager.setLookAndFeel( "com.sun.java.swing.plaf"
                                  + ".motif.MotifLookAndFeel" );
	  makeUIChanges ();
	} 
	else if(rb.isSelected() && rb.getText().equals("Metal Look and Feel")) {
	  UIManager.setLookAndFeel( "javax.swing.plaf"
                                  + ".metal.MetalLookAndFeel" );
	  makeUIChanges ();
	} 
      } 
      catch (UnsupportedLookAndFeelException exc) {
	// Error - unsupported L&F
	rb.setEnabled(false);
	new ErrorDialog (null, "Unsupported LookAndFeel: " + rb.getText()
		             + ".\nLoading cross platform look and feel");
	
	// Set L&F to Metal
	try {
	  metalMenuItem.setSelected(true);
	  UIManager.setLookAndFeel( UIManager
                                    .getCrossPlatformLookAndFeelClassName() );
	  SwingUtilities.updateComponentTreeUI(startwin);
	} catch (Exception exc2) {
	  new ErrorDialog (null, "Could not load LookAndFeel: " + exc2);
	}
      } 
      catch (Exception exc) {
	rb.setEnabled(false);
	new ErrorDialog (null, "Could not load LookAndFeel: " + rb.getText());
      }
      normalCursor ();
    }
  }
  
  private class DispListener implements ItemListener {
    public void itemStateChanged(ItemEvent e) {
      JRadioButtonMenuItem rb = (JRadioButtonMenuItem) e.getSource();
      if ( rb.isSelected() && rb.getText().equals("Mountain Ranges") )
           dtype = CONST.MTN_RANGES;
      else if ( rb.isSelected() && rb.getText().equals("Time Lines") )
           dtype = CONST.TIMELINES;
    }
  }
  
private void makeUIChanges()
{
    SwingUtilities.updateComponentTreeUI( this );
    // setResizable (true); setSize (getMinimumSize ()); setResizable (false);
    setSize( getPreferredSize() );
    if ( !isApplet ) 
        SwingUtilities.updateComponentTreeUI( (Component)openAppFileDlg );
    else
        SwingUtilities.updateComponentTreeUI( openApltFileDlg );
    
    if ( btns_viewer != null )
        SwingUtilities.updateComponentTreeUI( btns_viewer );
    if ( tour_viewer != null )
        SwingUtilities.updateComponentTreeUI( tour_viewer );
    //  if (disp != null) disp.makeUIChanges ();
}
  
  protected void finalize() throws Throwable {super.finalize();}
}
