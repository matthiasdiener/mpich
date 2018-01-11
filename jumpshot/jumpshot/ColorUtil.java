package jumpshot;
import java.awt.Color;
import java.io.*;
import java.util.*;

//This class interprets X Colors as java colors specified with R-G-B values
//1. The file containing all the X Colors is specified by the Mainwin object
//   and its File object is passed onto this object. 
//2. The static method readColors then traverses through this file. 
//   The method expects the file to have the following format. 
//   Every color should be in a unique line.
//   [red value] [green value] [blue value]     [color name]
//       .             .             .                .
//3. The colors alongwith their rgb values are read and a Vector 'colors' of
//   CLOG_COLOR objects is formed.
//4. Whenever an X Color name is read during the reading of the clog file
//   its corresponding rgb value is obtained from getColor () method. If an
//   unrecognized color is sought a default color is returned. Only those colors
//   specified in the COLOR FILE ARE VALID, the rest being INVALID.
class COLOR_UTIL
{
  public static Mainwin parent;
  public static Vector colors;   //Vector of CLOG_COLOR objects containing X based colors
                                 //and the corresponding rgb values
  public static Color defColor;  //default Color to be returned if no X matching color found
  public static File colorFile; //Name of the file containing X based colors

  //This part will be executed as soon as program is run
  static {
    colors = new Vector ();
    defColor = Color.pink;
  }
  
  //this method will read X Colors from the specified file.
  //file = pointer to file containing X based colors in the correct format
  public static void readColors (Mainwin p, File file) {
    parent = p;
    colorFile = file;
    FileInputStream inFile;
    try {inFile = new FileInputStream (file);}
    catch (FileNotFoundException x) {
      new ErrorDiag (parent, "Color file " + file.toString () + " cannot be found");
      return;
    }
    int size;
    try {size = inFile.available ();}
    catch (IOException x) {
      new ErrorDiag (parent, "IOException occured while reading colorfile " + file.toString ());
      return;
    }
    byte[] buf;
    buf = new byte [size];
    try {inFile.read (buf);}
    catch (IOException x) {
      new ErrorDiag (parent, "IOException occured while reading colorfile " + file.toString ());
      return;
    }
    
    ByteArrayInputStream in = new ByteArrayInputStream (buf);
    InputStreamReader rd = new InputStreamReader (in);
    BufferedReader block = new BufferedReader (rd);
    String s;
    try {
      while ((s = block.readLine ()) != null) {
        StringTokenizer tokens = new StringTokenizer (s);
        int r = (Integer.valueOf (tokens.nextToken ())).intValue ();
        int g = (Integer.valueOf (tokens.nextToken ())).intValue ();
        int b = (Integer.valueOf (tokens.nextToken ())).intValue ();
        String name = new String (tokens.nextToken ());
        int extra = tokens.countTokens ();
        if (extra > 0) {
          for (int i = 0; i < extra; i++) {
            String next = tokens.nextToken ();
            name += " ";
            name += next;
          }
        }
          
        colors.addElement (new CLOG_COLOR (name, new Color (r, g, b)));
      }
      try {inFile.close ();}
      catch (IOException y) {System.out.println ("File cannot be closed."); return;}
    }
    catch (IOException x) {
      new ErrorDiag (parent, "IOException occured while reading colorfile " + file.toString ());
      return;
    }
  }

  //this method returns a java.awt.Color object for the given string specifying
  //a X based color
  //name = the name of the X based color
  public static Color getColor (String name) {
    Enumeration enum = colors.elements ();
    while (enum.hasMoreElements ()) {
      CLOG_COLOR c = (CLOG_COLOR)enum.nextElement ();
      int len = name.indexOf (':');
      if (len == -1) len = name.length ();
      if (c.name.regionMatches (0, name, 0, len) && (c.name.length () == len)) {
        return c.color;
      }
    }    
    new ErrorDiag (parent, "Color " + name + " could not be found in " + 
                   colorFile.toString () + ". Returning " + defColor.toString ());
    return defColor;
  }
}  
//------------------------------------------------------------------------------------

//this class specifies a CLOG_COLOR object 
class CLOG_COLOR {
  public String name;
  public Color color;
  
  //Constructor
  public CLOG_COLOR (String n, Color c) {
    name = n;
    color = c;
  }
}


