import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;


class CLOG_MSG {
  static final int Size = (6 * 4);
  int etype;			// kind of message event 
  int tag;			// message tag 
  int partner;		        // source or destination in send/recv
  int comm;			// communicator
  int size;			// length in bytes 
  int srcloc;			// id of source location
  
  void read (DataInputStream in) {
    try {
      etype = in.readInt ();
      tag = in.readInt ();
      partner = in.readInt ();
      comm = in.readInt ();
      size = in.readInt ();
      srcloc = in.readInt ();
    }
    catch (IOException x) {
	System.out.println ("IOException while reading CLOG_MSG.");
	return;
    }
  }

  public static int getSize () {return Size;}

  public String toString () {
    return ("CLOG_MSG [etype=" + etype + ", tag=" + tag + ", partner=" +  
            partner + ", comm=" + comm + ", size=" + size + 
            ", srcloc=" + srcloc + "Size(in Bytes)=" + Size + "]");
  }
}
