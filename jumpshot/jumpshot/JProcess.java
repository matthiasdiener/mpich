package jumpshot;

import java.awt.*;
import java.util.Vector;

//This class specifies each Process. 
public class JProcess extends CRectangle
{
  //this vector stores all states belonging to this process in ascending order 
  //of their end timestamps
  public Vector procStateVector; 
  public int procId;          //procId of this particular JProcess
  public boolean dispStatus;  //tells whether the process should be drawn or not
  double begT;                //starting time for the first state of this process
  
  //Constructor
  public JProcess (int pid) { 
    super (new Point (), new Dimension (0, 1), Color.red);
    procId = pid;
    dispStatus = true;
    procStateVector = new Vector ();
  }
}


