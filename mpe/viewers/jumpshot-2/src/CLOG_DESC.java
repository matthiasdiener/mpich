import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;


class CLOG_DESC {
  static final int size = (2 * 8);
  String desc;
  
  //Constructor
  public CLOG_DESC (){desc = new String ();}
  
  //read the record
  void read (DataInputStream in) {
    byte []b = new byte [2 * 8];
    try {
      in.readFully (b);
    }
    catch (IOException x) {
      System.out.println ("IOException in reading CLOG_DESC.");
      return;
    }
    //Here we use platform's default character encoding. 
    //Its probably from ascii but, don't know whether 
    //will work always."INVESTIGATE!"
    desc = (new String (b, 0, 2*8)).trim ();
  }

  public static int getSize () {return size;}
  
  public String toString () {
    return ("CLOG_DESC [desc=" + desc + ", size=" + size + "]");
  }
}
