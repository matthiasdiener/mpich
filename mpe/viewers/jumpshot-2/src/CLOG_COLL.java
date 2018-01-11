import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;


class CLOG_COLL {
  static final int Size = (6 * 4);
  int etype;			// type of collective event 
  int root;			// root of collective op 
  int comm;			// communicator
  int size;			// length in bytes 
  int srcloc;			// id of source location 
  int pad;
 
  void read (DataInputStream in) {
    try {
      etype = in.readInt ();
      root = in.readInt ();
      comm = in.readInt ();
      size = in.readInt ();
      srcloc = in.readInt ();
      pad = in.readInt ();
      
      }
    catch (IOException x) {
      System.out.println ("IOException while reading CLOG_COLL.");
      return;
    }
  }
  
  public static int getSize () {return Size;}

  public String toString () {
    return ("CLOG_COLL [etype=" + etype + ", root=" + root + 
            ", comm=" +  comm + ", size=" + size + ", srcloc=" + srcloc +
            ", pad=" + pad + ", Size(in Bytes)=" + Size + "]");
  }
}
