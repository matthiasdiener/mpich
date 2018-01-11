import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;


class CLOG_SRC {
  static final int size = (2 * 4) + CLOG_FILE.getSize (); 
  int srcloc;			         // id of source location
  int lineno;			         // line number in source file
  CLOG_FILE filename = new CLOG_FILE (); // source file of log statement
  
  void read (DataInputStream in) {
    try {
      srcloc = in.readInt ();
      lineno = in.readInt ();
      filename.read (in);
    }
    catch (IOException x) {
      System.out.println ("IOException has occurred in reading CLOG_SRC");
    }
  }
  
  public static int getSize () {return size;}

  public String toString () {
    return ("CLOG_SRC [srcloc=" + srcloc + ", lineno=" + lineno +
            ", file=" + filename.file + ", size=" + size + "]");
  }
}
