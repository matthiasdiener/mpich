import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;

//This class represents each state
class StateInfo extends Info { 
  int level = 0;           //For nested states
  StateInfo higher = null; //Pointer to the next higher state enveloping this.

  CLOG_STATE stateDef;
  int procId;
  
  //Constructor 
  public StateInfo () {super ();};
  
  //Constructor
  public StateInfo (int pid, double beg, double end, CLOG_STATE s) {
    super ();
    procId = pid;
    begT = beg;
    endT = end;
    stateDef = s;
  } 
  
  public String toString () {
    return  ("StateInfo [level=" + level + ", procId=" + procId + 
             ", begT=" + begT + ", endT=" + endT + ", lenT=" + lenT +
             ", name=" + stateDef.description.desc +
             "]");
  }
}
