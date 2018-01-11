import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;


class CLOG_COMM {
  static final int size = (4 * 4);
  int etype;			// type of communicator creation
  int parent;			// parent communicator
  int newcomm;		        // new communicator
  int srcloc;			// id of source location

  void read (DataInputStream in) {
    try {
      etype = in.readInt ();
      parent = in.readInt ();
      newcomm = in.readInt ();
      srcloc = in.readInt ();
    }
    catch (IOException x) {
      System.out.println ("IOException in reading CLOG_COMM.");
      return;
    }
  }
  
  public static int getSize () {return size;}

  public String toString () {
    return ("CLOG_COMM [etype=" + etype + ", pt=" + parent + 
            ", newcomm=" +  newcomm + " srcloc=" + srcloc + 
            ", size" + size + "]");
  }
}  
