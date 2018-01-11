import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;


class CLOG_FILE {
  static final int size = (5 * 8);
  String file;

  public CLOG_FILE (){file  = new String ();}

  void read (DataInputStream in) {
    byte []b = new byte [5 * 8];
    try {
      in.readFully (b);
    }
    catch (IOException x) {
      System.out.println ("IOException in CLOG_FILE.read");
      return;
    }
    //Here we use platform's default character encoding. 
    //Its probably from ascii but, don't know whether 
    //will work always."INVESTIGATE!"
    file = (new String (b, 0, 5 * 8)).trim ();  
  }
  
  public static int getSize () {return size;}

  public String toString () {
    return ("CLOG_FILE [file=" + file + ", size=" + size + "]");}
}
