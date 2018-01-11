import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;


class CLOG_CNAME {
  static final int size = (3 * 8);
  String name;
  
  //Constructor
  public CLOG_CNAME () {name = new String ();}
  
  //read the record
  void read (DataInputStream in) {
    byte []b = new byte [3 * 8];
    try {
      in.readFully (b);
    }
    catch (IOException x) {
      System.out.println ("IOException in CLOG_CNAME.read"); 
      return;
    }
    //Here we use platform's default character encoding. 
    //Its probably from ascii but, don't know whether 
    //will work always."INVESTIGATE!"
    name = (new String (b, 0, 3 * 8)).trim ();
  }
  
  public static int getSize () {return size;}

  public String toString () {
    return ("CLOG_CNAME [name=" + name + ", size=" + size + "]");
  }
}
