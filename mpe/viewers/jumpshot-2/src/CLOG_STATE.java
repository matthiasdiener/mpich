import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;


//This class represents a state definition
class CLOG_STATE {
  final static int size = (4 * 4) + CLOG_DESC.getSize () + CLOG_CNAME.getSize ();
  int stateid;		// integer identifier for state
  int startetype;	// starting event for state 
  int endetype;		// ending event for state 
  int pad;
  
  CLOG_DESC description = new CLOG_DESC ();   //string describing state
  
  Vector events;        // Temporary data structure storing unmatched events
  Vector stateVector;   // Vector storing completed states (paired up events)
  Color color;		// Color given to this state

  //This checkbox determines whether states beloging to this state def., should
  //be displayed or not.
  JCheckBox checkbox;
  

  //Contructor
  public CLOG_STATE () {
    events = new Vector ();
    stateVector = new Vector ();
  }
  
  //read the record from the given input stream
  void readBigEnd (DataInputStream in) {
    try {
      stateid = in.readInt ();
      startetype = in.readInt ();
      endetype = in.readInt ();
      pad = in.readInt ();
      CLOG_CNAME tempColor = new CLOG_CNAME ();
      tempColor.read (in);
      color = ColorUtil.getColor (tempColor.name);
      description.read (in);
    }
    catch (IOException x) {
      System.out.println ("IOException in reading CLOG_STATE.");
      return;
    }
  }
  
  //read the record from the given input stream
  void readLitEnd (DataInputStream in) {
    try {
      stateid = FUNCTS.swapInt (in);
      startetype = FUNCTS.swapInt (in);
      endetype = FUNCTS.swapInt (in);
      pad = in.readInt ();
      CLOG_CNAME tempColor = new CLOG_CNAME ();
      tempColor.read (in);
      color = ColorUtil.getColor (tempColor.name);
      description.read (in);
    }
    catch (IOException x) {
      System.out.println ("IOException in reading CLOG_STATE.");
      return;
    }
  }
  
  //returns the number of bytes this record occupies
  public static int getSize () {return size;}

  //This function takes a header associated with a RAW record whose
  //etype is of the start type. A stateInfo object is created and
  //put in a temporary vector until its corresponding RAW record
  //with end etype is read.
  void startEvent (CLOG_HEADER header) {
    StateInfo evt = new StateInfo (header.procid, header.timestamp, 0, this);
    events.addElement (evt);
  }
  
  //Once a RAW record with a end etype is read, its corresponding RAW
  //record with the start etype is assumed to have been read earlier.
  //That RAW record is searched for in the temporary vector. When found
  //the StateInfo object is removed from the temporary vector and we
  //have a completed state that we add to stateVector
  StateInfo endEvent (CLOG_HEADER header) {
    //The assumption here is that events of the same state and
    //same procids will not overlap. 
    Enumeration enum = events.elements ();
    StateInfo evt = null;
    while (enum.hasMoreElements ()) {
      evt = (StateInfo)enum.nextElement ();
      if (header.procid == evt.procId) break;
    }
    events.removeElement (evt);
    evt.endT = header.timestamp;
    evt.lenT = evt.endT - evt.begT;
    stateVector.addElement (evt);
    return evt;
  }

  public String toString () { 
    return ("CLOG_STATE: [stateid=" + stateid + ", startetype=" + startetype +
            ", endetype=" + endetype + ", color=" + color.toString () +
            ", description=" + description.desc + ", pad=" + pad + 
            ", size=" + size + "]");
  }
}
