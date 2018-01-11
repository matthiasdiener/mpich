import com.sun.java.swing.*;
import java.awt.*;
import java.awt.event.*;

class ErrorDiag {
  public ErrorDiag(Component p, String txt) {
      JOptionPane.showMessageDialog(p, txt, "Error", JOptionPane.ERROR_MESSAGE);
  }
}
