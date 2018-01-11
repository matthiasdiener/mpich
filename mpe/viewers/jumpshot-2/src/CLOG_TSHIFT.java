import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;


class CLOG_TSHIFT {
  static final int size = (1 * 8);
  double timeshift;		// time shift for this process 
  
  void read (DataInputStream in) {
    try {
      timeshift = in.readDouble ();
      System.out.print (" shift=" + (new Float (timeshift)).toString()); 
    }
    catch (IOException x) {
	System.out.println ("IOException has occurred in TSHIFT");
	return;
    }
  }
  
  public static int getSize () {return size;}

  public String toString () {
    return ("CLOG_TSHIFT [timeshift=" + timeshift+ ", size=" + size + "]");
  }
}
