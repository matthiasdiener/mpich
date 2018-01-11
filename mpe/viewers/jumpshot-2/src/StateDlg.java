import com.sun.java.swing.*;
import java.awt.*;
import java.awt.event.*;

/**
 * Dialog showing information about the state
 */
class StateDlg extends JDialog 
implements ActionListener
{
  /**
   * Constructor
   */
  public StateDlg (Frame f, ProcessState ps) {
    super (f, "State Info");
    
    StateInfo info = ps.info;
    
    GridBagConstraints con = new GridBagConstraints ();
    con.anchor = GridBagConstraints.SOUTHWEST;
    
    JButton b = new JButton ();
    b.addActionListener (this);
    b.setLayout (new GridBagLayout ());
    b.setToolTipText ("Click to close");
    b.setCursor (new Cursor (Cursor.HAND_CURSOR));
    
    b.add (new JLabel ("State: " + info.stateDef.description.desc + " from " + 
				       (new Float (info.begT)).toString () + " to " + 
				       (new Float (info.endT)).toString () + " "), 
			   con); 
    
    con.gridy = 1;
    b.add (new JLabel ("Length: " + (new Float (info.lenT)).toString () + 
				       " sec, Process: " + Integer.toString (ps.info.procId)), 
			   con);
   
    
    getContentPane ().setLayout (new BorderLayout ());
    getContentPane ().add (b, BorderLayout.CENTER);
       
    addWindowListener (new WindowAdapter () {
      public void windowClosing (WindowEvent e) {dispose ();}
    });
    
    pack ();
    setSize (getMinimumSize ()); setResizable (false);
  }
  
  public void actionPerformed (ActionEvent e) {dispose ();}
}
