import com.sun.java.swing.*;
import java.awt.*;
import java.awt.event.*;

/**
 * Dialog showing information about a message arrow
 */
class MsgDlg extends JDialog 
implements ActionListener {
  
  /**
   * Constructor
   */
  public MsgDlg (Frame f, ArrowInfo info) {
    super (f, "Message Info");
    
    GridBagConstraints con = new GridBagConstraints ();
    con.anchor = GridBagConstraints.SOUTHWEST;
    
    JButton b = new JButton ();
    b.addActionListener (this);
    b.setLayout (new GridBagLayout ());
    b.setToolTipText ("Double click to destroy");
    b.setCursor (new Cursor (Cursor.HAND_CURSOR));
    
    JLabel x = new JLabel ();
    x.setText ("Message: Size " + Integer.toString (info.size) + ", Tag " + 
	       Integer.toString (info.tag) + ", " +
	       (new Float (info.size / (double)(info.endT - info.begT))).toString () + 
	       " (bytes/sec)");
    b.add (x, con);

    JLabel y = new JLabel ();
    y.setText ("from Process " + Integer.toString (info.begProcId) + " at " +
	       (new Float (info.begT)).toString () + " sec, to Process " + 
	       Integer.toString (info.endProcId) + " at " + (new Float (info.endT)).toString () +
	       " sec");
    
    con.gridy = 1;
    b.add (y, con);
    
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
