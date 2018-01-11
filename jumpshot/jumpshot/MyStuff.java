package jumpshot;
import java.awt.Button;
import java.awt.event.ActionListener;
import java.awt.TextField;
import java.awt.Color;

//This class extends Button to add the ActionListener
class MyButton extends Button {
  //Constructor
  public MyButton (ActionListener a) {
    super ();
    this.addActionListener (a);
  }
  
  //Constructor
  public MyButton (String label, ActionListener a) {
    super (label);
    this.addActionListener (a);
  }
}

//--------------------------------------------------------

//This class extends TextField to put background and foreground
//colors to the TextField and make it editable according to the options.
class MyTextField extends TextField {
  public MyTextField (boolean b) {super (); setup (b);}
  public MyTextField (String text, boolean b) {super (text); setup (b);}
  public MyTextField (int columns, boolean b) {super (columns); setup (b);}
  public MyTextField (String text, int columns, boolean b) {
    super (text, columns);
    setup (b);
  }
  
  void setup (boolean b) {
    this.setBackground (Color.white);
    this.setForeground (Color.black);
    this.setEditable (b);
  }
}
