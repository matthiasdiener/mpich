package jumpshot;

/** This class starts a Jumpshot application. */
public class Jumpshot {
  
  /**
   * this main method expects the log filename as a command line parameter.
   * However, if the name is not given, a null value is assumed.
   */
  public static void main (String []  args) {
    String param = null;
    try {param = args [0];}
    catch (Exception x) {param = null;}
    
    //Start the Jumpshot application
    new Mainwin (param, false);
  }
}

