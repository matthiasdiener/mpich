package jumpshot;

import java.util.Vector;
import java.util.Random;
import java.util.Enumeration;

/*
  This class is responsible for building data structures from records
  of the clog file. Each record read by ClogReader is passed here and
  RecordHandler sifts out the relevent data from it.
  
  Once the clog file is read, 4 data structures exist.
  
  1. (stateDefs) This is a vector of all the state definitions in the 
     order they were read. 
  2. Each of the state definition objects mentioned above contains a vector
     'stateVector' that is a collection of states of the same type
     in the ascending order of their end timestamps.
  3. (data) This is vector or all states of all types in ascending order 
     of their end timestamps.
  4. (a) This is an object that contains a vector 'arrows' which is a 
     list of ArrowInfo objects representing messages.
*/

public class RecordHandler {
  public Vector stateDefs;
  public Vector data;
  public CLOG_ARROW a;
  
  //Constructor
  public RecordHandler () {
    stateDefs = new Vector ();
    data = new Vector ();
    setupArrows ();
  }
  
  void setupArrows () {
    a = new CLOG_ARROW ();
    a.startetype = CONST.LOG_MESG_SEND;
    a.endetype = CONST.LOG_MESG_RECV;
  }
   
  //Get information from a RAW record and its header
  public void HandleRaw (RECORD rec) {
    CLOG_HEADER header = (CLOG_HEADER)(rec.header);
    
    CLOG_RAW raw = (CLOG_RAW)(rec.data);
    int etype = raw.etype;
    
    if (etype == a.startetype) a.startArrowEvent (header, raw);
    else if (etype == a.endetype) a.endArrowEvent (header, raw);
    else {
      CLOG_STATE state = resolve (etype);
      if (state == null) return;
      if (state.startetype == etype) state.startEvent (header);
      else data.addElement (state.endEvent (header));
    }
  }
  
  //Get information from a CLOG_STATE record
  public void HandleStateDef (CLOG_STATE state) {stateDefs.addElement (state);}
  
  //This function traverses through the vector containing state definitions
  //looking for the state definition whose starting or ending 
  //etype corresponds with the given etype. If no such state def. is found
  //a null is returned.
  CLOG_STATE resolve (int etype) {
    Enumeration enum = stateDefs.elements ();
    while (enum.hasMoreElements ()) {
      CLOG_STATE s = (CLOG_STATE)(enum.nextElement ());
      if (etype == s.startetype || etype == s.endetype) 
        return s;
    }
    System.out.println ("Error: Unrecognized etype in RAW record");
    return null;
  }
}

