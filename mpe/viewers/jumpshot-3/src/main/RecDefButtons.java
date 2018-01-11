import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import javax.swing.plaf.*;
import javax.swing.*;
import java.util.Vector;
import java.util.Enumeration;
import javax.swing.border.Border;

/**
 * This class creates the frame with buttons which stand for each
 * state type. The buttons are pressed to produce histogram frames
 */
public class RecDefButtons extends JToolBar
implements ActionListener, ItemListener,
           ComponentListener, PropertyChangeListener 
{
  FrameDisplay parent;
  private Vector      hists;
  private Dimension   init_win_dim;
  private JScrollPane scrollpane;
  private Vector      state_btns, arrow_btns;
  private JPanel      all_states_pane, all_arrows_pane;
  private JCheckBox   all_states_chkbox, all_arrows_chkbox;

  private JPanel      main_panel;
  private JPanel      states_panel, arrows_panel;
  private GridLayout  states_gridlayout, arrows_gridlayout;

  private boolean     IsRunningStateOff;
  private Dimension   max_btn_dim;
  private int         Nrow, Ncolumn;
  private int         Nbtn;
  
  public RecDefButtons( FrameDisplay p, Dimension in_init_win_dim,
                        boolean in_IsRunningStateOff )
  {
    super ();
    
    parent            = p;
    init_win_dim      = in_init_win_dim;
    IsRunningStateOff = in_IsRunningStateOff;
    hists             = new Vector ();
    Nrow              = 0;
    Ncolumn           = 0;
    CreateButtonsAndInit();
    SetAllBtnPanelsSize();
    SetupPanels( init_win_dim );
    addPropertyChangeListener( this );
    addComponentListener( this );

    //  Explicitly set the FloatingFrame __Resizable__
    setUI( new javax.swing.plaf.basic.BasicToolBarUI()
    {
        protected JFrame createFloatingFrame( JToolBar tbar )
        {
            JFrame frame = super.createFloatingFrame( tbar );
            frame.setResizable( true );
            frame.setTitle( "Legends for TimeLine window" );
            return frame;
        }
    });
  }
  
  private void CreateButtonsAndInit()
  {
    RecDefBtnPanel  btn;
    Dimension       btn_pref_dim;
    String          btn_description;
    boolean         btn_flag;

    max_btn_dim  = new Dimension( 0, 0 );

    //  Determine the maximum size among all the STATE RecDefBtnPanel
    state_btns = new Vector(); 
    if ( parent.stateDefs.size() > 0 ) {
        Enumeration defs = parent.stateDefs.elements();
        while ( defs.hasMoreElements() ) {
            RecDef def = ( RecDef ) defs.nextElement();
            if ( def.stateVector.size() > 0 ) {  // eliminate the empty ones
                btn_flag = true;
                if ( IsRunningStateOff ) {
                    btn_description = ( new String( def.description ) )
                                      .toLowerCase();
                    if ( btn_description.indexOf( "running" ) >= 0 )
                        btn_flag = false;
                }
                btn = new RecDefBtnPanel( def, this, btn_flag );
                btn_pref_dim = btn.getPreferredSize();
                if ( btn_pref_dim.width > max_btn_dim.width ) 
                    max_btn_dim.width = btn_pref_dim.width;
                if ( btn_pref_dim.height > max_btn_dim.height )
                    max_btn_dim.height = btn_pref_dim.height;
                state_btns.addElement( btn );
            }
        }

        // States CheckBox Control, all_states_pane
        if ( state_btns.size() > 1 ) {
            all_states_pane = new JPanel();
            all_states_pane.setBorder(
                            BorderFactory.createLoweredBevelBorder() );
            all_states_pane.setLayout( new FlowLayout( FlowLayout.LEFT ) );

            all_states_chkbox = new JCheckBox( "     All States", true );
            all_states_chkbox.setToolTipText( "Enable or disable all states" );
            all_states_chkbox.addItemListener( this );
            all_states_pane.add( all_states_chkbox );

            btn_pref_dim = all_states_pane.getPreferredSize();
            if ( btn_pref_dim.width > max_btn_dim.width ) 
                max_btn_dim.width = btn_pref_dim.width;
            if ( btn_pref_dim.height > max_btn_dim.height )
                max_btn_dim.height = btn_pref_dim.height;
        }
    }

    //  Determine the maximum size among all the ARROW RecDefBtnPanel
    arrow_btns = new Vector(); 
    if ( parent.arrowDefs.size() > 0 ) { 
        Enumeration defs = parent.arrowDefs.elements();
        while ( defs.hasMoreElements() ) {
            RecDef def = ( RecDef ) defs.nextElement();
            if ( def.stateVector.size() > 0 ) {  // eliminate the empty ones
                btn = new RecDefBtnPanel( def, this, true );
                btn_pref_dim = btn.getPreferredSize();
                if ( btn_pref_dim.width > max_btn_dim.width )
                    max_btn_dim.width = btn_pref_dim.width;
                if ( btn_pref_dim.height > max_btn_dim.height )
                    max_btn_dim.height = btn_pref_dim.height;
                arrow_btns.addElement( btn );
            }
        }
        // Arrows CheckBox Control, all_arrows_pane
        if ( arrow_btns.size() > 1 ) {
            all_arrows_pane = new JPanel();
            all_arrows_pane.setBorder(
                            BorderFactory.createLoweredBevelBorder() );
            all_arrows_pane.setLayout( new FlowLayout( FlowLayout.LEFT ) );

            all_arrows_chkbox = new JCheckBox( "     All Arrows", true );
            all_arrows_chkbox.setToolTipText( "Enable or disable all arrows" );
            all_arrows_chkbox.addItemListener( this );
            all_arrows_pane.add( all_arrows_chkbox );

            btn_pref_dim = all_arrows_pane.getPreferredSize();
            if ( btn_pref_dim.width > max_btn_dim.width ) 
                max_btn_dim.width = btn_pref_dim.width;
            if ( btn_pref_dim.height > max_btn_dim.height )
                max_btn_dim.height = btn_pref_dim.height;
        }
    }
  }  //  Endof  CreateButtonsAndPanels()

  private void SetAllBtnPanelsSize()
  {
    RecDefBtnPanel  btn;

    //  Set the size of the all buttons to fixed size.
    Enumeration btns;
    btns = state_btns.elements();
    while ( btns.hasMoreElements() ) {
        btn = ( RecDefBtnPanel ) btns.nextElement();
        btn.setMaximumSize( max_btn_dim );
    }
    if ( state_btns.size() > 1 )
        all_states_pane.setMaximumSize( max_btn_dim );

    btns = arrow_btns.elements();
    while ( btns.hasMoreElements() ) {
        btn = ( RecDefBtnPanel ) btns.nextElement();
        btn.setMaximumSize( max_btn_dim );
    }
    if ( arrow_btns.size() > 1 )
        all_arrows_pane.setMaximumSize( max_btn_dim );
  }

  private void SetupPanels( Dimension cur_dim )
  {
    Border          border1, border2;

    //  Determine the optimal Ncolumn of the GridLayout
    //     50 is for the JToolBar's tab & ScrollPane's vert scrollbar
    //     0.4 is used to round integer of x.6 to x+1
    if ( getOrientation() == SwingConstants.HORIZONTAL ) {
        Ncolumn = (int) ( (double)(cur_dim.width - 50)
                          / max_btn_dim.width
                        + 0.4 );
        if ( Ncolumn < 1 )
            Ncolumn = 1;
    }
    else
        Ncolumn = 1;

/*
    System.out.println( "SetupPanels(): cur_dim = " + cur_dim );
    System.out.println( "SetupPanels(): init_win_dim = " + init_win_dim );
    System.out.println( "SetupPanels(): max_btn_dim = " + max_btn_dim );
    System.out.println( "SetupPanels(): getOrientation() = "
                                      + getOrientation() );
    System.out.println( "SetupPanels(): HORIZONTAL = "
                                      + SwingConstants.HORIZONTAL );
    System.out.println( "SetupPanels(): Ncolumn = " + Ncolumn );
*/

    border2 = BorderFactory.createLoweredBevelBorder();

    //  Setup the main panel
    main_panel = new JPanel();
    main_panel.setLayout( new BoxLayout( main_panel, BoxLayout.Y_AXIS ) );
    main_panel.setAlignmentX( 0 );
    main_panel.setAlignmentY( 0 );

    // Setup States Panels
    if ( state_btns.size() > 0 ) {
        //  Initialize the State sections of the panel
        Nbtn    = state_btns.size() + ( state_btns.size() > 1 ? 1 : 0 );
        Nrow    = ( Nbtn / Ncolumn ) + ( Nbtn % Ncolumn > 0 ? 1 : 0 ); 

        if ( Nrow > 1 )
            states_gridlayout = new GridLayout( 0, Ncolumn );
        else
            states_gridlayout = new GridLayout( 0, Nbtn );
        states_panel = new JPanel( states_gridlayout );
        border1 = BorderFactory.createEmptyBorder( 4, 4, 2, 4 );
        states_panel.setBorder( BorderFactory.createCompoundBorder( border1,
                                                                    border2) );
    
        // Add individual state buttons
        Enumeration btns = state_btns.elements();
        while ( btns.hasMoreElements() )
            states_panel.add( ( RecDefBtnPanel ) btns.nextElement() );
    
        // add all-states checkbox control
        if ( state_btns.size() > 1 )
            states_panel.add( all_states_pane );
    
        main_panel.add( states_panel );
    }   //  Endof  if ( parent.stateDefs.size() > 0 )
    
    // Setup Arrows Panels
    if ( arrow_btns.size() > 0 ) {
        //  Initialize the Arrow sections of the panel
        Nbtn    = arrow_btns.size() + ( arrow_btns.size() > 1 ? 1 : 0 );
        Nrow    = ( Nbtn / Ncolumn ) + ( Nbtn % Ncolumn > 0 ? 1 : 0 );

        if ( Nrow > 1 )
            arrows_gridlayout = new GridLayout( 0, Ncolumn );
        else
            arrows_gridlayout = new GridLayout( 0, Nbtn );
        arrows_panel = new JPanel( arrows_gridlayout );
        border1 = BorderFactory.createEmptyBorder (2, 4, 4, 4);
        arrows_panel.setBorder( BorderFactory.createCompoundBorder( border1,
                                                                    border2 ) );

        // add individual arrow buttons
        Enumeration btns = arrow_btns.elements();
        while ( btns.hasMoreElements() )
            arrows_panel.add( ( RecDefBtnPanel ) btns.nextElement() );

        // add all-arrows checkbox control
        if ( arrow_btns.size() > 1 )
            arrows_panel.add( all_arrows_pane );

        main_panel.add( arrows_panel );
    }   //  Endof  if ( arrow_btns.size() > 0 )

    // Setup "Change Orientation" button
    JButton orientation_btn = new JButton( "Change Orientation" );
    orientation_btn.setToolTipText( "Switch the orientation of this panel "
                                  + "when undocked from the timeline window" );
    orientation_btn.addActionListener( this );
    main_panel.add( orientation_btn );
      
/*
    System.out.println( "JToolBar's Size = " + getSize() + "\n"
                      + "JToolBar's Perferred   = " + getPreferredSize() + "\n"
                      + "JToolBar's MinimumSize = " + getMinimumSize() + "\n"
                      + "JToolBar's MaximumSize = " + getMaximumSize() );
*/

    //  Add main_panel to JScrollPane
    scrollpane = new JScrollPane( main_panel );
    add( scrollpane );
  }  //  Endof  SetupPanels()

  private void SetInternalLayout()
  {
    //  Determine the optimal Ncolumn of the GridLayout
    //     50 is for the JToolBar's tab & ScrollPane's vert scrollbar
    //     0.4 is used to round integer of x.6 to x+1
    if ( getOrientation() == SwingConstants.HORIZONTAL ) {
        Ncolumn = (int) ( 0.4 + (double) ( getSize().width - 50 ) 
                              / max_btn_dim.width );
        if ( Ncolumn < 1 )
            Ncolumn = 1;
    }
    else
        Ncolumn = 1;

    Nrow = 0;
    int  Nrow_total = 0;

    // Setup Arrows Panels with Ncolumn computed
    if ( state_btns.size() > 0 ) {
        //  Initialize the State sections of the panel
        Nbtn    = state_btns.size() + ( state_btns.size() > 1 ? 1 : 0 );
        Nrow    = ( Nbtn / Ncolumn ) + ( Nbtn % Ncolumn > 0 ? 1 : 0 );
    
        if ( Nrow > 1 )
            states_gridlayout.setColumns( Ncolumn );
        else
            states_gridlayout.setColumns( Nbtn );
        states_panel.invalidate();
    }
    Nrow_total += Nrow;

    Nrow = 0;

    // Setup Arrows Panels with Ncolumn computed
    if ( arrow_btns.size() > 0 ) {
        //  Initialize the Arrow sections of the panel
        Nbtn    = arrow_btns.size() + ( arrow_btns.size() > 1 ? 1 : 0 );
        Nrow    = ( Nbtn / Ncolumn ) + ( Nbtn % Ncolumn > 0 ? 1 : 0 );

        if ( Nrow > 1 )
            arrows_gridlayout.setColumns( Ncolumn );
        else
            arrows_gridlayout.setColumns( Nbtn );
        arrows_panel.invalidate();
    }
    Nrow_total += Nrow;

    validate();
    SetAllBtnPanelsSize();

/*
    //  Determine the preferred size based on the getOrientation()
    Dimension pref_size;
    if ( Ncolumn > 1 ) {
        if ( Nrow_total > 3 )
            pref_size = new Dimension( scrollpane.getSize().width,
                                       (int) (3.5*max_btn_dim.height) );
        else
            pref_size = new Dimension( scrollpane.getSize().width,
                                       (int) (Nrow_total*max_btn_dim.height) );
    }
    else
        pref_size = new Dimension( max_btn_dim.width + 15, 
                                   scrollpane.getSize().height );
      
    //  main_panel.setPreferredSize( perf_size ) cannot be called, it
    //  seems to set the main_panel to a fixed size, this renders the
    //  arrows_panel disappear from the scroll_pane.

    //  scrollpane.setPreferredSize( perf_size ) can be called.
     
    //  this.setPreferredSize( pref_size ) activates a call to propertyChange()

    scrollpane.setPreferredSize( pref_size );
*/

    // setPreferredSize( getSize() );
  }  // Endof  SetInternalLayout()

  //  For PropertyChangeListener
  public void propertyChange( PropertyChangeEvent evt )
  {
/*
    System.out.println( "\n\n"
                      + "propertyChange(): Size = "
                                         + getSize() + "\n"
                      + "propertyChange(): PerferredSize   = "
                                         + getPreferredSize()+ "\n"
                      + "propertyChange(): MinimumSize = "
                                         + getMinimumSize() + "\n"
                      + "propertyChange(): MaximumSize = "
                                         + getMaximumSize() + "\n"
                      + "propertyChange(): Orientation = "
                                         + getOrientation() );
    System.out.println( "propertyChange(): evt = " + evt );
*/

/*
    SetInternalLayout();
*/

    //  Determine the preferred size based on the getOrientation()
    Dimension parent_size, pref_size, min_size, max_size;
    parent_size = parent.getSize();
    if ( getOrientation() == SwingConstants.HORIZONTAL ) {
        if ( getSize().width > parent_size.width ) 
            pref_size = new Dimension( getSize().width, 
                                       (int) (3.5*max_btn_dim.height) );
        else
            pref_size = new Dimension( parent_size.width, 
                                       (int) (3.5*max_btn_dim.height) );
        max_size  = new Dimension( parent_size.width,
                                   parent_size.height / 4 );
    }
    else {
        if ( getPreferredSize().height > parent_size.height )
            pref_size = new Dimension( max_btn_dim.width + 15,
                                       getPreferredSize().height );
        else
            pref_size = new Dimension( max_btn_dim.width + 15,
                                       parent_size.height );
        max_size  = new Dimension( parent_size.width / 4,
                                   parent_size.height );
    }

    //  setSize( perf_size ) does NOT work, it seems value set by setSize()
    //  will be modified by the values in setPreferredSize later on
    setMaximumSize( max_size );
    setPreferredSize( pref_size );
  }

//  For ComponentChangeListener
  public void componentResized( ComponentEvent evt )
  {
/*
    System.out.println( "\n\n"
                      + "componentResized(): Size = "
                                           + getSize() + "\n"
                      + "componentResized(): PerferredSize   = "
                                           + getPreferredSize()+ "\n"
                      + "componentResized(): MinimumSize = "
                                           + getMinimumSize() + "\n"
                      + "componentResized(): MaximumSize = "
                                           + getMaximumSize() + "\n"
                      + "componentResized(): Orientation = "
                                           + getOrientation() );
    System.out.println( "componentResized(): evt = " + evt );
*/

    SetInternalLayout();
  }

  public void componentHidden( ComponentEvent evt ) {;}
  public void componentMoved( ComponentEvent evt ) {;}
  public void componentShown( ComponentEvent evt ) {;}
  
  private void SetRecDefsEnabled( Vector rec_defs, boolean val )
  {
      Enumeration defs = rec_defs.elements();
      while ( defs.hasMoreElements() ) {
          RecDef def = (RecDef) defs.nextElement();
          if ( def.stateVector.size() > 0 )
              def.checkbox.setSelected( val );
      }
  }


  /**
   * causes the histogram frame to be displayed for the given state name
   */
  private void getStateHistogram( String name )
  {
      waitCursor();
      parent.waitCursor();
      parent.canvas.waitCursor();
      parent.init_win.waitCursor();
      RecDef def = getRecDef( name );

      if ( def != null ) 
          hists.addElement( new Histwin( def, parent ) );
      else
          new ErrorDialog(this, "Record Definition " + name
                              + " does not exist"); 

      parent.init_win.normalCursor();
      parent.canvas.normalCursor();
      parent.normalCursor();
      normalCursor();
  }
  
  /**
   * This function returns the RecDef associated with the given name string.
   */
  private RecDef getRecDef( String name ) {
      RecDef      def;

      def = parent.stateDefs.GetRecDef( name );
      if ( def == null )
          def = parent.arrowDefs.GetRecDef( name );
    
      return def;
  }
  
  /**
   * The function prints out key to jumpshot data.
   */
  int print (Graphics g, int x, int y, int width, int height) {
    Font f = g.getFont ();
    FontMetrics fm = getToolkit ().getFontMetrics (f);
    
    int charW = fm.stringWidth (" "), charH = fm.getHeight ();
    int hgap1 = charW, hgap2 = 2 * charW, vgap = fm.getAscent ();
    int rectW = 30, rectH = charH; //Dimensions of state rectangles
    int xcord = x, ycord = y;
    
    Enumeration enum = parent.stateDefs.elements ();
    while (enum.hasMoreElements ()) {
      RecDef s = (RecDef)enum.nextElement ();
      
      if (s.stateVector.size () > 0) {
	int strW = fm.stringWidth (s.description);
	
	if ((xcord + rectW + hgap1 + strW) > (width + x)) {
          xcord = x; ycord += (charH + vgap);
        }
        
        g.setColor (s.color);
        g.fillRect (xcord, ycord, rectW, rectH);
        g.setColor (Color.black);
        g.drawRect (xcord, ycord, rectW - 1, rectH - 1);
        g.drawString( s.description,
                      xcord + rectW + hgap1,
                      ycord + rectH - fm.getDescent () - 1);
        xcord += (rectW + hgap1 + strW + hgap2);
      }
    }
    return (ycord - y + (2 * charH));
  }      

  /**
   * sets the cursor to the WAIT_CURSOR type
   */
  void waitCursor () {setCursor (new Cursor (Cursor.WAIT_CURSOR));} 
  
  /** 
   * sets the cursor to the cursor associated with this frame
   */
  void normalCursor () {setCursor (new Cursor (Cursor.DEFAULT_CURSOR));}
  
  public void actionPerformed( ActionEvent evt )
  {
      String command = evt.getActionCommand();
      if ( command.equals( "Change Orientation" ) ) {
          if ( getOrientation() == SwingConstants.HORIZONTAL )
              setOrientation( SwingConstants.VERTICAL );
          else
              setOrientation( SwingConstants.HORIZONTAL );
      }
      else
          getStateHistogram( command );
  }
  
  public void itemStateChanged( ItemEvent ievt )
  {
      Object obj = ievt.getItemSelectable();
      if ( obj == all_states_chkbox )
          SetRecDefsEnabled( parent.stateDefs,
                             all_states_chkbox.isSelected() );
      if ( obj == all_arrows_chkbox )
          SetRecDefsEnabled( parent.arrowDefs,
                             all_arrows_chkbox.isSelected() );
      parent.canvas.Refresh();
  }
  
  private void killHists()
  {
      if ( hists.size() > 0 ) {
          Enumeration enum = hists.elements();
          while ( enum.hasMoreElements() ) {
              Histwin h = (Histwin) enum.nextElement();
	      if ( h != null ) {
                  h.kill (); h = null;
              }
          }
          hists.removeAllElements ();
      }
  }
  
  void kill()
  {
      killHists();
      removeAll();
  }
}
