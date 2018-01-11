import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import com.sun.java.swing.*;

class ApltFileDlg extends JDialog 
implements ActionListener
{
  List urlList;
  Mainwin parent;
  boolean select;
  
  public ApltFileDlg(Mainwin p,String txt)
  {
    super((Frame)p, txt, true);
    parent = p;
    setup ();
  }
  
  //setup methods--------------------------------------------------------
  void setup () {
    setupPanel ();
      
    pack ();
    setResizable (true);
    //    setSize (getMinimumSize ()); 
    
    // Define, instantiate and register a WindowListener object.
    addWindowListener (new WindowAdapter () {
      public void windowClosing (WindowEvent e) {select = false; setVisible (false);}
    });
  }
  
  void setupPanel () {
    MyJPanel p = new MyJPanel (new BorderLayout (), "Files");
    
    p.add (urlList = new List (4, false), BorderLayout.CENTER);
    urlList.setBackground (Color.white);
    urlList.setForeground (Color.black);
    getUrls ();
    urlList.select (0);
    
    JPanel pj = new JPanel (new GridLayout (1, 2));
    pj.add (new MyButton ("OK", "Open selected logfile", this));
    pj.add (new MyButton ("Cancel", "Cancel logfile open", this));
    
    p.add (pj, BorderLayout.SOUTH);
    
    getContentPane ().add ("Center", p);
  }
  //end of setup methods----------------------------------------------------------
  
  //event handler methods--------------------------------------------------------
  //event handler method for ActionEvents generated by buttons
  public void actionPerformed (ActionEvent evt) {
    String command = evt.getActionCommand ();
    if (command.equals ("Cancel")) {select = false; setVisible (false);}
    else if (command.equals ("OK")) {select = true; setVisible (false);}
  }
  //end of event handler methods------------------------------------------------
  
  //Utility methods-------------------------------------------------------------
  public String getFile () {return urlList.getSelectedItem ();}
  
  void getUrls () {
    String u = parent.parent.getParameter ("logfiledir"), line, s;
    URL url = null;
    BufferedReader in = null;
    int x, y;
    
    try {
      url = new URL(parent.distributionUrl + u + "/" + 
		    parent.parent.getParameter ("logfileinfo"));
    }
    catch (MalformedURLException e){
      new ErrorDiag (parent, "Bad URL:" + url); return;
    }
    
    try {
      in = new BufferedReader (new InputStreamReader (url.openStream ()));
      while ((line = in.readLine ()) != null) {
	x = line.indexOf (".clog");
	for (y = x - 1; y > -1 && line.charAt (y) != '<' && line.charAt (y) != ' ' 
	       && line.charAt (y) != '"' ; y--);
	if (x != -1) {
	  String name = parent.distributionUrl + u + "/" + line.substring (y + 1, x) + ".clog";
	  urlList.addItem (name); 
	}
      }
      in.close ();
    }
    catch (IOException e) {
      new ErrorDiag (parent, "IO Error:" + e.getMessage ());
    }
  }
}
