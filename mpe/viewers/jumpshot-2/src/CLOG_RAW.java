import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;


//class representing a RAW record
class CLOG_RAW {
  final static int size = (4 * 4) + CLOG_DESC.getSize ();
  int etype;			// raw event
  int data;			// uninterpreted data
  int srcloc;			// id of source location
  int pad;
  CLOG_DESC string = new CLOG_DESC ();   	// uninterpreted string
 
  //read the record from the given input stream
  void readBigEnd (DataInputStream in) {
    try {
      etype = in.readInt ();
      data = in.readInt ();
      srcloc = in.readInt ();
      pad = in.readInt ();
      string.read (in);
    }
    catch (IOException x) {
      System.out.println ("IOException has occurred in CLOG_RAW.");
      return;
    }
  }
  
  //read the record from the given input stream
  void readLitEnd (DataInputStream in) {
    try {
      etype = FUNCTS.swapInt (in);
      data = FUNCTS.swapInt (in);
      srcloc = FUNCTS.swapInt (in);
      pad = in.readInt ();
      string.read (in);
    }
    catch (IOException x) {
      System.out.println ("IOException has occurred in CLOG_RAW.");
      return;
    }
  }
  
  //Returns the number of bytes the record occupies
  public static int getSize () {return size;}

  //Returns tag specified in string field. This is pertinent when
  //RAW record represents a start or end of a message arrow
  //If tag cannot be parsed a 0 is returned
  int getTag () {
    int i = string.desc.indexOf (' '); if (i < 1) return 0;
    String s = string.desc.substring (0, i);
    int j;
    try {j = (Integer.valueOf (s)).intValue ();}
    catch (NumberFormatException x) {return 0;}
    return j;
  }
  
  //Returns message size specified in string field. This is pertinent
  //when RAW record represents a start or end of a message arrow
  //If message cannot be parsed a 0 is returned
  int getMsgSize () {
    int i = string.desc.indexOf (' '); if (i < 0) i = 0;
    if (i > (string.desc.length () - 2)) return 0;
    String s = string.desc.substring (i + 1);
    int j;
    try {j = (Integer.valueOf (s)).intValue ();}
    catch (NumberFormatException x) {return 0;}
    return j;
  }
  
  //Copy Constructor
  CLOG_RAW Copy () {
    CLOG_RAW cp = new CLOG_RAW ();
    cp.etype = this.etype;
    cp.data = this.data;
    cp.srcloc = this.srcloc;
    cp.pad = this.pad;
    cp.string = this.string;
    return cp;
  }

  public String toString () {
    return ("CLOG_RAW [etype=" + etype + ", data=" + data + ", srcloc=" + 
            srcloc + ", string=" + string.desc + ", pad=" + pad + 
            ", size=" + size + "]");
  }
}
