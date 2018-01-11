import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;


class CLOG_EVENT {
  static final int size = (2 * 4) + CLOG_DESC.getSize ();
  int etype;			            // event
  int pad;                                  // pad 
  CLOG_DESC description = new CLOG_DESC (); // string describing event
  
  void read (DataInputStream in) {
    try {
      etype = in.readInt ();
      pad = in.readInt ();
      description.read (in);
    }
    catch (IOException x) {
      System.out.println ("IOException in reading CLOG_EVENT.");
      return;
    }
  }
  
  public static int getSize () {return size;}

  public String toString () {
    return ("CLOG_EVENT [etype=" + etype + ", desc=" + description.desc + 
            ", pad=" + pad + ", size=" + size + "]");
  }
}
