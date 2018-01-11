import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;

//This class stores information about a message
class ArrowInfo  extends Info{
  int begProcId;
  int endProcId;
  int tag;
  int size;
  
  //Constructor for the object representing a message.
  //bid = Process Id of the process where the message originates
  //eid = Process Id of the process where the message ends.
  //beg = The time at which the message originates.
  //t = Value of tag contained in message.
  public ArrowInfo (int bid, int eid, double beg, double end, int t, int s) {
    super ();
    begProcId = bid; endProcId = eid; begT = beg; endT = end; tag = t; size = s;
  }
  
  public String toString () {
    return ("ArrowInfo [begProcId=" + begProcId + ", endProcId=" + endProcId +
	    ", begT=" + begT + ", endT=" + endT + ", tag=" + tag + ", size=" +
	    size + "]");
  }
} 
