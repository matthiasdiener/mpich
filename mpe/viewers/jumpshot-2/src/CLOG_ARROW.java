import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;

//This object is a repository for all ArrowInfo objects which represent messages
class CLOG_ARROW {
  Vector sends;         //Temporary vector to store incomplete ArrowInfo objects
                         //arising from unmatched RAW events
  Vector recvs;
  Vector arrowVector;         //Vector that stores completed ArrowInfo objects
  int startetype;        //startetype identifying the start of a message
  int endetype;          //endetype identifying the end of a message
    
  //Constructor
  public CLOG_ARROW () {
    sends = new Vector ();
    recvs = new Vector ();
    arrowVector = new Vector ();
  } 
  
  //This function takes a header associated with a RAW record whose
  //etype is of the start type for arrows. An arrowInfo object is created and
  //put in a temporary vector until its corresponding RAW record
  //with end etype is read.  
  void startArrowEvent (CLOG_HEADER header, CLOG_RAW raw) {
    if (recvs.size () > 0) {
      Enumeration enum = recvs.elements ();
      ArrowInfo  arrow = null;
      boolean found = false;
      
      while (enum.hasMoreElements () && !found) {
	arrow = (ArrowInfo)enum.nextElement ();
	if (header.procid == arrow.begProcId && raw.getTag () == arrow.tag &&
	    raw.data == arrow.endProcId) found = true;
      }
      
      if (found) {
	recvs.removeElement (arrow);
	arrowVector.addElement (arrow);
	arrow.begT = header.timestamp;
	return;
      }
    }
    ArrowInfo arrow = new ArrowInfo (header.procid, raw.data, header.timestamp, 
				     0, raw.getTag (), raw.getMsgSize ()); 
    sends.addElement (arrow);
  }
  
  //Once a RAW record with a end etype is read, its corresponding RAW
  //record with the start etype is assumed to have been read earlier.
  //That RAW record is searched for in the temporary vector. When found
  //the ArrowInfo object is removed from the temporary vector and we
  //have a completed arrow that we add to arrows
  void endArrowEvent (CLOG_HEADER header, CLOG_RAW raw) {
    if (sends.size () > 0) {
      Enumeration enum = sends.elements ();
      ArrowInfo  arrow = null;
      boolean found = false;
      
      while (enum.hasMoreElements () && !found) {
	arrow = (ArrowInfo)enum.nextElement ();
	if (header.procid == arrow.endProcId && raw.getTag () == arrow.tag &&
	    raw.data == arrow.begProcId) found = true;
      }
      
      if (found) {
	sends.removeElement (arrow);
	arrowVector.addElement (arrow);
	arrow.endT = header.timestamp;
	return;
      }
    }
    
    ArrowInfo arrow = new ArrowInfo (raw.data, header.procid, 0,
				     header.timestamp, 
				     raw.getTag (), raw.getMsgSize ()); 
    recvs.addElement (arrow);
  }
}
