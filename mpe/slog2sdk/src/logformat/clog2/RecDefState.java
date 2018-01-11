/*
 *  (C) 2001 by Argonne National Laboratory
 *      See COPYRIGHT in top-level directory.
 */

/*
 *  @author  Anthony Chan
 */

package logformat.clog2;

import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import java.io.*;


// Class corresponds to CLOG_STATE
public class RecDefState
{
    public  final static int RECTYPE  = Const.RecType.STATEDEF;
    private final static int BYTESIZE = 4 * 4
                                      + StrCname.BYTESIZE
                                      + StrDesc.BYTESIZE
                                      + StrFormat.BYTESIZE;
    public         int     stateID;        // integer identifier for the state
    public         Integer startetype;     // beginning event for the state 
    public         Integer finaletype;     // ending event for the state 
    private static int     pad;
    public         String  color;          // string describing color
    public         String  name;           // naming the state
    public         String  format;         // format string for the state
  
    //read the record from the given input stream
    public int readFromDataStream( MixedDataInputStream in )
    {
        try {
            stateID      = in.readInt();
            startetype   = new Integer( in.readInt() );
            finaletype   = new Integer( in.readInt() );
            pad          = in.readInt();
            color        = in.readString( StrCname.BYTESIZE );
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

    // Mimic <MPE>/src/log_wrap.c's definition of MPI_Init() in initialization
    // of states[] and the way MPI_xxx routines's CLOG_STATE is defined
    // in MPI_Finalize();
    public static List getMPIinitUndefinedStateDefs()
    {
        RecDefState def;

        // evtID is equivalent to the variable "stateid" MPI_Init() 
        // in <MPE>/src/log_wrap.c
        int evtID = Const.MPE_1ST_EVENT;  
        List defs = new ArrayList( Const.MPE_MAX_STATES );
        for ( int idx = 0; idx < Const.MPE_MAX_STATES; idx++ ) {
            def              = new RecDefState();
            def.startetype   = new Integer( evtID++ );
            def.finaletype   = new Integer( evtID++ );
            def.color        = "pink";
            def.name         = null;
            def.format       = null;
            defs.add( def );
        }
        return defs;
    }
 
    public static List getUSERinitUndefinedStateDefs()
    {
        RecDefState def;

        // evtID is equivalent to the variable "stateid" MPI_Init() 
        // in <MPE>/src/log_wrap.c
        int evtID = Const.MPE_USER_1ST_EVENT;  
        List defs = new ArrayList( Const.MPE_USER_MAX_STATES );
        for ( int idx = 0; idx < Const.MPE_USER_MAX_STATES; idx++ ) {
            def              = new RecDefState();
            def.startetype   = new Integer( evtID++ );
            def.finaletype   = new Integer( evtID++ );
            def.color        = "pink";
            def.name         = null;
            def.format       = null;
            defs.add( def );
        }
        return defs;
    }
 
    public String toString()
    { 
        return ( "RecDefState"
               + "[ stateID=" + stateID
               + ", startetype=" + startetype
               + ", finaletype=" + finaletype
               // + ", pad=" + pad
               + ", color=" + color
               + ", name=" + name 
               + ", format=" + format
               // + ", BYTESIZE=" + BYTESIZE
               + " ]" );
    }

    public static final void main( String[] args )
    {
        List defs = RecDefState.getMPIinitUndefinedStateDefs();
        Iterator itr = defs.iterator();
        while ( itr.hasNext() )
            System.out.println( (RecDefState) itr.next() );
    }
}
