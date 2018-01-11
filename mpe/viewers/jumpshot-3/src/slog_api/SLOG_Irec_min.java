import java.io.*;

public class SLOG_Irec_min
{
    public short           bytesize;
    public short           rectype;
    public short           intvltype;
    public SLOG_bebits     bebits;
    public double          starttime;
    public double          duration;
    public SLOG_tasklabel  origID;
    public SLOG_tasklabel  destID;
    public int             instr_addr;

    public SLOG_Irec_min()
    {
        bytesize   = 0;
        rectype    = SLOG_Const.INVALID_short;
        intvltype  = SLOG_Const.INVALID_short;
        bebits     = null;
        starttime  = SLOG_Const.INVALID_double;
        duration   = SLOG_Const.INVALID_double;
        origID     = null;
        destID     = null;
        instr_addr = SLOG_Const.NULL_iaddr;
    }

    public SLOG_Irec_min( short   in_bytesize,
                          short   in_rectype,
                          short   in_intvltype,
                          byte    in_bits_0,
                          byte    in_bits_1,
                          double  in_starttime,
                          double  in_duration,
                          short   in_orig_node,
                          byte    in_orig_cpu,
                          int     in_orig_thread,
                          int     in_instr_addr )
    {
        bytesize   = in_bytesize;
        rectype    = in_rectype;
        intvltype  = in_intvltype;
        bebits     = new SLOG_bebits( in_bits_0, in_bits_1 );
        starttime  = in_starttime;
        duration   = in_duration;
        origID     = new SLOG_tasklabel( in_orig_node, in_orig_cpu,
                                         in_orig_thread );
        instr_addr = in_instr_addr;
    }

    public SLOG_Irec_min( short   in_bytesize,
                          short   in_rectype,
                          short   in_intvltype,
                          byte    in_bits_0,
                          byte    in_bits_1,
                          double  in_starttime,
                          double  in_duration,
                          short   in_orig_node,
                          byte    in_orig_cpu,
                          int     in_orig_thread,
                          int     in_instr_addr,
                          short   in_dest_node,
                          byte    in_dest_cpu,
                          int     in_dest_thread )
    {
        bytesize   = in_bytesize;
        rectype    = in_rectype;
        intvltype  = in_intvltype;
        bebits     = new SLOG_bebits( in_bits_0, in_bits_1 );
        starttime  = in_starttime;
        duration   = in_duration;
        origID     = new SLOG_tasklabel( in_orig_node, in_orig_cpu, 
                                         in_orig_thread );
        destID     = new SLOG_tasklabel( in_dest_node, in_dest_cpu, 
                                         in_dest_thread );
        instr_addr = in_instr_addr;
    }

    public SLOG_Irec_min( DataInputStream data_istm )
    throws IOException
    {
        this.ReadFromDataStream( data_istm );
    }

    public void ReadFromDataStream( DataInputStream data_istm )
    throws IOException
    {
        bytesize      = data_istm.readShort();
        rectype       = data_istm.readShort();
        intvltype     = data_istm.readShort();
        bebits        = new SLOG_bebits( data_istm );
        starttime     = data_istm.readDouble();
        duration      = data_istm.readDouble();
        origID        = new SLOG_tasklabel( data_istm );
        if ( SLOG_global.IsOffDiagRec( rectype ) )
           destID        = new SLOG_tasklabel( data_istm );
        instr_addr    = data_istm.readInt();
    }

    public final int NbytesInFile()
    {
        if ( SLOG_global.IsOffDiagRec( rectype ) )
//             return( 37 );
            return( 41 );
        else
//            return( 30 );
            return( 34 );
    }

    public String toString()
    {
        StringBuffer representation = new StringBuffer( "[ " );
        representation.append(       bytesize  );
        representation.append( " " + rectype   );
        representation.append( " " + intvltype );
        representation.append( " " + bebits    );
        representation.append( " " + starttime );
        representation.append( " " + duration  );
        representation.append( " " + origID    );
        representation.append( " " + Integer.toHexString( instr_addr ) );    
        if ( SLOG_global.IsOffDiagRec( rectype ) )
            representation.append( " " + destID    );
        representation.append( " ]" );
        return( representation.toString() );
    }
}
