package jumpshot;

import java.io.*;
import java.util.*;
    
/*
  This class reads the data from the clog file and returns the finished
  data structures to Mainwin. The clog file is traversed and records obtained.
  Each record is passed on to a RecordHandler class which sifts out 
  the needed information and forms the data structures
*/
public class ClogReader {
  int numevents = 0;
  double firsttime = 0;
  public Mainwin parent;
  boolean swap;
  
  //Contructor
  public ClogReader (Mainwin p) {
    parent = p;
  }
   
  //reads the clog file and returns a RecordHandler object that contains
  //all the data structures
  public RecordHandler ReadClogFile (Mainwin app) {    
    String filename = app.logFileName;
    RecordHandler mainTool = new RecordHandler ();
    
    //Check whether logfile is big endian. It it is little endian swapping
    //of bytes will occur. clog files are a collection of pairs
    //[CLOG_HEADER and corresponding CLOG_record]. We read a CLOG_HEADER
    //from the clog file and check if CLOG_HEADER.rectype makes sense ie.,
    //it is a valid rectype. If it is invalid we assume that the clog
    //file is little endian and swapping will have to be performed for
    //int and double values.
    boolean flag = checkEndian (filename, mainTool);
    if (!flag) return null;
    
    //At this stage we do not know for sure where exactly the RAW records
    //will be specified. They may occur before, after or even in the middle
    //of RAW records. Thus, we take a fool proof approach by making 2 passes
    //through the file

    //Pass 1: traversing through the clog file we obtain all the state 
    //definitions
    flag = getStateDefs (filename, mainTool);
    if (!flag) return null;
    
    //Pass 2: Having got the state definitions we now read RAW records
    flag = getRawEvents (filename, mainTool);
    if (!flag) return null;
    //------------------------------or-------------------------------------
    
    //At a later date if state definitions will always occur before RAW 
    //records then we can replace the 2 passes with a single one by
    //calling the funcion below
    //boolean flag = getData (filename, mainTool);
    //if (!flag) return null;
    
    return mainTool;
  }
  
  //checks whether file is big endian by reading CLOG_HEADER
  boolean checkEndian (String filename, RecordHandler mainTool) {
    FileInputStream inFile;
    byte [] block = new byte [CLOG_HEADER.getSize ()];
    
    try {inFile = new FileInputStream (filename);}
    catch (FileNotFoundException x) {
      new ErrorDiag (parent, "logfile:" + filename + " not found");
      return false;
    }
    
    int bytesRead;
    
    try {bytesRead = inFile.read (block);}
    catch (IOException x) {
      new ErrorDiag (parent, "IOException while reading logfile");
      return false;
    }
    
    if (bytesRead == -1) return false;
    
    DataInputStream in = new DataInputStream (new ByteArrayInputStream (block));
    
    CLOG_HEADER hdr = new CLOG_HEADER ();
    hdr.readBigEnd (in);
    int rtype = hdr.rectype;
    
    switch (rtype) {
    case CONST.CLOG_STATEDEF: case CONST.CLOG_RAWEVENT:
    case CONST.CLOG_SRCLOC: case CONST.CLOG_COMMEVENT:
    case CONST.CLOG_MSGEVENT: case CONST.CLOG_COLLEVENT:
    case CONST.CLOG_EVENTDEF: case CONST.CLOG_ENDBLOCK:
    case CONST.CLOG_ENDLOG: swap = false; break;
    default: 
      new ErrorDiag (parent, "Unrecognized record type:" + rtype +
			    ". Maybe the file is little endian");
      swap = true; 
    }
    
    try {inFile.close ();}
    catch (IOException x) {
      new ErrorDiag (parent, "logfile:" + filename + " cannot be closed.");
      return false;
    }
    
    return true;
  }
  
  //reads CLOG_STATE records and skips over everything else
  boolean getStateDefs (String filename, RecordHandler mainTool) {
    FileInputStream inFile;
    
    try {inFile = new FileInputStream (filename);}
    catch (FileNotFoundException x) {
      new ErrorDiag (parent, "logfile " + filename + " cannot not be found"); 
      return false;
    }
    
    for (int retval = readStates (inFile, mainTool);
         retval != 0;
         retval = readStates (inFile, mainTool));
    
    try {inFile.close ();} 
    catch (IOException y) {
      new ErrorDiag (parent, "logfile " + filename + " cannot be closed." +
                     " Proceeding anyway.");
    }
    return true;
  }

  int readStates (FileInputStream inFile, RecordHandler mainTool) {
    byte[] block;
    block = new byte [CONST.CLOG_BLOCK_SIZE];
    int bytesRead = 0;
    
    try {bytesRead = inFile.read (block);}
    catch (IOException x) {
      new ErrorDiag (parent, "IOException has occurred while reading logfile");
      return 0;
    }
    
    if (bytesRead == -1) return 0;
       
    DataInputStream in = new DataInputStream (new ByteArrayInputStream (block));
    
    int rtype;
    CLOG_HEADER hdr = new CLOG_HEADER ();
    CLOG_RAW raw = new CLOG_RAW ();

    rtype = CONST.CLOG_UNDEF;
    while (rtype != CONST.CLOG_ENDBLOCK && rtype != CONST.CLOG_ENDLOG) {
      
      if (swap) hdr.readLitEnd (in); else hdr.readBigEnd (in);
      rtype = hdr.rectype;
      
      try {
        switch (rtype){
        case CONST.CLOG_STATEDEF:
          CLOG_STATE state = new CLOG_STATE();
          if (swap) state.readLitEnd (in); else state.readBigEnd (in);
          mainTool.HandleStateDef (state);
          break;    
        case CONST.CLOG_RAWEVENT:
          in.skipBytes (CLOG_RAW.getSize ()); break;   
        case CONST.CLOG_SRCLOC:
          in.skipBytes (CLOG_SRC.getSize ()); break;
        case CONST.CLOG_COMMEVENT:
          in.skipBytes (CLOG_COMM.getSize ()); break;
        case CONST.CLOG_MSGEVENT:
          in.skipBytes (CLOG_MSG.getSize ()); break;
        case CONST.CLOG_COLLEVENT:
          in.skipBytes (CLOG_COLL.getSize ()); break;
        case CONST.CLOG_EVENTDEF:
          in.skipBytes (CLOG_EVENT.getSize ()); break;
        case CONST.CLOG_ENDBLOCK: break;
        case CONST.CLOG_ENDLOG: break;
          
        default: 
          new ErrorDiag (parent, "Unrecognized record type:" + rtype);
          return 0;
        }
      }
      catch (IOException x) {
        new ErrorDiag (parent, "IOException occured while reading logfile");
        return 0;
      }
    }

    return bytesRead;
  }
  
  //reads only RAW records
  boolean getRawEvents (String filename, RecordHandler mainTool) {
    FileInputStream inFile;
    
    try {inFile = new FileInputStream (filename);}
    catch (FileNotFoundException x) {
      new ErrorDiag (parent, "logfile " + filename + "cannot be found");
      return false;
    }
    
    firsttime = 0; numevents = 0;
    for (int retval = readRaw (inFile, mainTool);
         retval != 0;
         retval = readRaw (inFile, mainTool));
    
    try {inFile.close ();} 
    catch (IOException y) {
      new ErrorDiag (parent, "logfile " + filename + " cannot be closed." + 
                     " Proceeding anyway.");
    }
    return true;
  }

  int readRaw (FileInputStream inFile, RecordHandler mainTool) {
    byte[] block;
    block = new byte [CONST.CLOG_BLOCK_SIZE];
    int bytesRead = 0;
    
    try {bytesRead = inFile.read (block);}
    catch (IOException x) {
      new ErrorDiag (parent, "IOException occured while reading logfile.");
      return 0;
    }
    
    if (bytesRead == -1) return 0;
       
    DataInputStream in = new DataInputStream (new ByteArrayInputStream (block));
    
    int rtype;
    CLOG_HEADER hdr = new CLOG_HEADER ();
    CLOG_RAW raw = new CLOG_RAW ();

    rtype = CONST.CLOG_UNDEF;
    while (rtype != CONST.CLOG_ENDBLOCK && rtype != CONST.CLOG_ENDLOG) {
      if (swap) hdr.readLitEnd (in); else hdr.readBigEnd (in);
      rtype = hdr.rectype;
      
      try {
        switch (rtype){
          
        case CONST.CLOG_RAWEVENT:
          if (numevents++ == 0) firsttime = hdr.timestamp;
          if (swap) raw.readLitEnd (in); else raw.readBigEnd (in);
          hdr.timestamp = hdr.timestamp - firsttime;
          mainTool.HandleRaw (new RECORD (hdr, raw));
          break;
          
        case CONST.CLOG_STATEDEF:
          in.skipBytes (CLOG_STATE.getSize ()); break;
        case CONST.CLOG_SRCLOC:
          in.skipBytes (CLOG_SRC.getSize ()); break;
        case CONST.CLOG_COMMEVENT:
          in.skipBytes (CLOG_COMM.getSize ()); break;
        case CONST.CLOG_MSGEVENT:
          in.skipBytes (CLOG_MSG.getSize ()); break;
        case CONST.CLOG_COLLEVENT:
          in.skipBytes (CLOG_COLL.getSize ()); break;
        case CONST.CLOG_EVENTDEF:
          in.skipBytes (CLOG_EVENT.getSize ()); break;
        case CONST.CLOG_ENDBLOCK: break;
        case CONST.CLOG_ENDLOG: break;
          
        default: 
          new ErrorDiag (parent, "Unrecognized record type:" + rtype);
          return 0;
        }
      }
      catch (IOException x) {
        new ErrorDiag (parent, "IOException occured while reading logfile");
        return 0;
      }
    }

    return bytesRead;
  }

  //reads both RAW and CLOG_STATE records and assumes that CLOG_STATE
  //records will occur before RAW.
  boolean getData (String filename, RecordHandler mainTool) {
    FileInputStream inFile;
    
    try {inFile = new FileInputStream (filename);}
    catch (FileNotFoundException x) {
      new ErrorDiag (parent, "logfile" + filename + " cannot be found");
      return false;
    }
    
    firsttime = 0; numevents = 0;

    for (int retval = readBlock (inFile, mainTool);
         retval != 0;
         retval = readBlock (inFile, mainTool));
    
    try {inFile.close ();} 
    catch (IOException y) {
      new ErrorDiag (parent, "logfile " + filename + " cannot be closed." + 
                     "Proceeding anyway.");
    }
    return true;
  }

  int readBlock (FileInputStream inFile, RecordHandler mainTool) {
    byte[] block;
    block = new byte [CONST.CLOG_BLOCK_SIZE];
    int bytesRead = 0;
    
    try {bytesRead = inFile.read (block);}
    catch (IOException x) {
      new ErrorDiag (parent, "IOException occured while reading logfile");
      return 0;
    }
    
    if (bytesRead == -1) return 0;
       
    DataInputStream in = new DataInputStream (new ByteArrayInputStream (block));
    
    int rtype;
    CLOG_HEADER hdr = new CLOG_HEADER ();
    CLOG_RAW raw = new CLOG_RAW ();

    rtype = CONST.CLOG_UNDEF;
    while (rtype != CONST.CLOG_ENDBLOCK && rtype != CONST.CLOG_ENDLOG) {
      
      if (swap) hdr.readLitEnd (in); else hdr.readBigEnd (in);
      rtype = hdr.rectype;
      
      try {
        switch (rtype){
          
        case CONST.CLOG_RAWEVENT:
          if (numevents++ == 0) firsttime = hdr.timestamp;
          if (swap) raw.readLitEnd (in); else raw.readBigEnd (in);
          hdr.timestamp = hdr.timestamp - firsttime;
          mainTool.HandleRaw (new RECORD (hdr, raw));
          break;
          
        case CONST.CLOG_STATEDEF:
          CLOG_STATE state = new CLOG_STATE();
          if (swap) state.readLitEnd (in); else state.readBigEnd (in);
          mainTool.HandleStateDef (state);
          break;    
          
        case CONST.CLOG_SRCLOC:
          in.skipBytes (CLOG_SRC.getSize ()); break;
        case CONST.CLOG_COMMEVENT:
          in.skipBytes (CLOG_COMM.getSize ()); break;
        case CONST.CLOG_MSGEVENT:
          in.skipBytes (CLOG_MSG.getSize ()); break;
        case CONST.CLOG_COLLEVENT:
          in.skipBytes (CLOG_COLL.getSize ()); break;
        case CONST.CLOG_EVENTDEF:
          in.skipBytes (CLOG_EVENT.getSize ()); break;
        case CONST.CLOG_ENDBLOCK: break;
        case CONST.CLOG_ENDLOG: break;
          
        default: 
          new ErrorDiag (parent, "Unrecognized record type:" + rtype);
          return 0;
        }
      }catch (IOException x) {
        new ErrorDiag (parent, "IOException occured while reading logfile.");
        return 0;
      }
    }

    return bytesRead;
  }
}
