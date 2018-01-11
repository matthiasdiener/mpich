import java.io.*;
import java.util.*;

public class SLOG_FrameSector implements Serializable
{
    private int     bytesize;          // Byte Size of the sector in file
    private int     Nrec;              // Number of Record in the sector
    public  Vector  irecs;             // Vector of SLOG_Irec

    public SLOG_FrameSector()
    {
        bytesize = 0;
        Nrec     = 0;
        irecs    = null;
    }

    public SLOG_FrameSector( final SLOG_FrameSector src )
    {
        bytesize = 0;
        Nrec     = 0;
        irecs    = new Vector( src.irecs.size() );

        Enumeration src_irecs  = src.irecs.elements();
        while ( src_irecs.hasMoreElements() )
            irecs.addElement( src_irecs.nextElement() );
        bytesize += src.bytesize;
        Nrec     += src.Nrec;
    }

    public void Init()
    {
        bytesize = 0;
        Nrec     = 0;
        if ( irecs != null ) {
            irecs.removeAllElements();
            irecs = null;
        }
    }

    public void SetByteSize( int in_bytesize )
    {
        bytesize = in_bytesize;
    }

    public void SetNumOfRecs( int in_Nrec )
    {
        Nrec = in_Nrec;
    }

    public void ReadFromDataStream(       DataInputStream  data_istm,
                                    final SLOG_RecDefs     recdefs )
    throws IOException, IllegalArgumentException
    {
        int bytesize_expected = bytesize;
        int Nrec_expected     = Nrec;

        bytesize = 0;
        Nrec     = 0;
        
        irecs = new Vector( Nrec_expected );
        for ( Nrec = 0; Nrec < Nrec_expected; Nrec++ ) {
            SLOG_Irec irec = new SLOG_Irec( data_istm, recdefs );
            irecs.addElement( irec );
            bytesize += irec.fix.bytesize;
        }

        if ( bytesize != bytesize_expected )
            throw new IOException( "Inconsistency in bytes read : "
                                 + "Actual bytes read = " + bytesize + ", "
                                 + "bytes expected = " + bytesize_expected );
        if ( irecs.size() != Nrec_expected )
            throw new IOException( "Inconsistency in No. of Record read : "
                                 + "Actual No. of Records read = " 
                                 + irecs.size() + ", "
                                 + "No. of Records expected = "
                                 + Nrec_expected );
    }

    public void prepend( final SLOG_FrameSector src )
    {
        Vector src_irecs = src.irecs;
        int src_size  = src_irecs.size();
        if ( src_size > 0 ) {
            int orig_size = irecs.size();

            irecs.ensureCapacity( orig_size + src_size );

            int idx, src_idx;
            if ( orig_size > src_size ) {
                for ( idx = orig_size - src_size; idx < orig_size; idx++ )
                    irecs.addElement( irecs.elementAt( idx ) );
                for ( idx = orig_size - src_size - 1; idx >= 0; idx-- )
                    irecs.setElementAt( irecs.elementAt( idx ), src_size+idx );
                for ( src_idx = 0; src_idx < src_size; src_idx++ )
                    irecs.setElementAt( src_irecs.elementAt( src_idx ),
                                        src_idx );
            }
            else {
                for ( src_idx = orig_size; src_idx < src_size; src_idx++ )
                    irecs.addElement( src_irecs.elementAt( src_idx ) );
                for ( idx = 0; idx < orig_size; idx++ )
                    irecs.addElement( irecs.elementAt( idx ) );
                for ( src_idx = 0; src_idx < orig_size; src_idx++ )
                    irecs.setElementAt( src_irecs.elementAt( src_idx ),
                                        src_idx );
            }
            bytesize += src.bytesize;
            Nrec     += src.Nrec;
        }
    } 

    public void append( final SLOG_FrameSector src )
    {
        irecs.ensureCapacity( irecs.size() + src.irecs.size() );

        Enumeration src_irecs  = src.irecs.elements();
        while ( src_irecs.hasMoreElements() )
            irecs.addElement( src_irecs.nextElement() );
        bytesize += src.bytesize;
        Nrec     += src.Nrec;
    }

    public String toString()
    {
        StringBuffer representation = new StringBuffer();

        Enumeration enum = irecs.elements();
        while( enum.hasMoreElements() ) {
            String str = enum.nextElement().toString();
            representation.append( str + "\n" );
        }

        return( representation.toString() );
    }
}

