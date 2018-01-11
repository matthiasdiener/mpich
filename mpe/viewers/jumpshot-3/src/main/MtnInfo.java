public class MtnInfo
{
    double begT;
    double endT;
    int []s;

    //Constructor 
    public MtnInfo (int n) { s = new int [n]; }

    public String toString()
    {
        return ( "MtnInfo=[ begT=" + begT
                         + ", endT=" + endT
                         + ", number=" + s.length + " ]");
    }
}
