import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;


class FUNCTS {
  public static int swapInt (DataInputStream in)
       throws IOException
  {
    byte [] b = new byte [4];
    byte t;
    in.read (b);
    t = b [0]; b [0] = b [3]; b [3] = t;
    t = b [1]; b [1] = b [2]; b [2] = t;
    return (new DataInputStream (new ByteArrayInputStream (b))).readInt ();
  }
  
  //This method may be not be producing correct results in some cases
  //So the above method is used. However this may be faster if fixed??
  public static int swapInt (int i) {
    int a = (i & 0xFF000000) >> 24;
    int b = (i & 0x00FF0000) >> 8;
    int c = (i & 0x0000FF00) << 8;
    int d = (i & 0x000000FF) << 24;
    return (a | b | c | d);
  }
  
  public static double swapDouble (DataInputStream in)
       throws IOException
  {
    byte [] b = new byte [8];
    byte t;
    in.read (b);
    t = b [0]; b [0] = b [7]; b [7] = t;
    t = b [1]; b [1] = b [6]; b [6] = t;
    t = b [2]; b [2] = b [5]; b [5] = t;
    t = b [3]; b [3] = b [4]; b [4] = t;
    return (new DataInputStream (new ByteArrayInputStream (b))).readDouble ();
  }
}
