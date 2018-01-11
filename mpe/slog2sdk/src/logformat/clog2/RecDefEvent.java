/*
 *  (C) 2001 by Argonne National Laboratory
 *      See COPYRIGHT in top-level directory.
 */

/*
 *  @author  Anthony Chan
 */

package logformat.clog2;

import java.io.*;


// Class corresponds to CLOG_Rec_EventDef
public class RecDefEvent
{
    public  static final int RECTYPE  = Const.RecType.EVENTDEF;
    private static final int BYTESIZE = 2 * 4
                                      + StrColor.BYTESIZE
                                      + StrDesc.BYTESIZE
                                      + StrFormat.BYTESIZE;
    public         Integer     etype;            // event
    private static int         pad;              // pad 
    public         String      color;            // string for color
    public         String      name;             // string describing event
    public         String      format;           // format string for the event
  
    public int readFromDataStream( MixedDataInputStream in )
    {
        try {
            etype        = new Integer( in.readInt() );
            pad          = in.readInt();
            color        = in.readString( StrColor.BYTESIZE );
            name         = in.readString( StrDesc.BYTESIZE );
            format       = in.readString( StrFormat.BYTESIZE );
        } catch ( IOException ioerr ) {
            ioerr.printStackTrace();
            return 0;
        }

        return BYTESIZE;
    }

    public int skipBytesFromDataStream( DataInputStream in )
    {
        try {
            in.skipBytes( BYTESIZE );
        } catch ( IOException ioerr ) {
            ioerr.printStackTrace();
            return 0;
        }

        return BYTESIZE;
    }

    public String toString()
    {
        return ( "RecDefEvent"
               + "[ etype=" + etype
               // + ", pad=" + pad
               + ", color=" + color
               + ", name=" + name
               + ", format=" + format
               // + ", BYTESIZE=" + BYTESIZE
               + " ]");
    }
}
