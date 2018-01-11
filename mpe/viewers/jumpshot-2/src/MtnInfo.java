import java.io.*;
import java.awt.*;
import java.util.*;
import com.sun.java.swing.*;

class MtnInfo { 
  double begT;
  double endT;
  int []s;
  
  //Constructor 
  public MtnInfo (int n) {
    s = new int [n];
  };
  
  public String toString () {
    return  ("MtnInfo [begT=" + begT + ", endT=" + endT
	     + ", number = " + s.length + "]");
  }
}
