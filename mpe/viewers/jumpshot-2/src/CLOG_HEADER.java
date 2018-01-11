import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;

class CLOG_HEADER {
  static final int size = (1 * 8) + (4 * 4);
  double timestamp;
  int rectype;
  int length;	
  int procid;			// currently rank in COMM_WORLD
  int pad;
  
  //read the record from the givne input stream
  void readBigEnd (DataInputStream in) {
    try {
      timestamp = in.readDouble ();  
      rectype = in.readInt ();
      length = in.readInt ();
      procid = in.readInt ();
      pad = in.readInt ();
    }
    catch (IOException x) {
      System.out.println ("IOException in CLOG_HEADER.read");
      return;
    }
  }
  
  //read the record from the givne input stream
  void readLitEnd (DataInputStream in) {
    try {
      timestamp = FUNCTS.swapDouble (in);  
      rectype = FUNCTS.swapInt (in);
      length = FUNCTS.swapInt (in);
      procid = FUNCTS.swapInt (in);
      pad = in.readInt ();
    }
    catch (IOException x) {
      System.out.println ("IOException in CLOG_HEADER.read");
      return;
    }
  }
  
  //Returns the number of bytes this record occupies
  public static int getSize () {return size;};

  //Copy Constructor
  CLOG_HEADER Copy () {
    CLOG_HEADER cp = new CLOG_HEADER ();
    cp.timestamp = this.timestamp;
    cp.rectype = this.rectype;
    cp.length = this.length;
    cp.procid = this.procid;
    cp.pad = this.pad;
    return cp;
  }
  
  public String toString () {
    return ("CLOG_HEADER [timestamp=" + timestamp + ", rectype=" + rectype + 
            ", length=" + length + ", procid=" + procid + ", pad =" + pad + 
            ", size=" + size + "]");
  } 
}
