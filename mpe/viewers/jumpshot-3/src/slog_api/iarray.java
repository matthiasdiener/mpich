import java.io.*;
import java.util.*;

public class iarray extends Vector
                    implements Serializable
{
    private int bytesize;

    public iarray()
    {
        super();
        bytesize = 0;
    }

    public iarray( int Nelem )
    {
        super( Nelem );
        bytesize = Nelem * 4;
    }

    public iarray( iarray src )
    {
        super( src.size() );

        int  ivalue;
        Enumeration src_elems = src.elements();
        while( src_elems.hasMoreElements() ) {
            ivalue = ( (Integer) src_elems.nextElement() ).intValue();
            super.addElement( new Integer( ivalue ) );
        }
        bytesize = super.size() * 4;
    }

    public iarray( DataInputStream data_istm, int Nelem )
    throws IOException
    {
        super( Nelem );
        this.ReadFromDataStream( data_istm, Nelem );
    }

    //  add iarray, src, before the head of current vector
    public void prepend( final iarray src )
    {
        int src_size  = src.size();
        if ( src_size > 0 ) {
            int orig_size = super.size();

            super.ensureCapacity( orig_size + src_size );

            int idx, src_idx;
            if ( orig_size > src_size ) {
                for ( idx = orig_size - src_size; idx < orig_size; idx++ )
                    super.addElement( super.elementAt( idx ) );
                for ( idx = orig_size - src_size - 1; idx >= 0; idx-- )
                    super.setElementAt( super.elementAt( idx ), src_size+idx );
                for ( src_idx = 0; src_idx < src_size; src_idx++ )
                    super.setElementAt( src.elementAt( src_idx ), src_idx );
            }
            else {
                for ( src_idx = orig_size; src_idx < src_size; src_idx++ )
                    super.addElement( src.elementAt( src_idx ) );
                for ( idx = 0; idx < orig_size; idx++ )
                    super.addElement( super.elementAt( idx ) );
                for ( src_idx = 0; src_idx < orig_size; src_idx++ )
                    super.setElementAt( src.elementAt( src_idx ), src_idx );
            }
            bytesize = super.size() * 4;
        }
    }

    //  add iarray, src, after the end of current vector
    public void append( final iarray src )
    {
        super.ensureCapacity( super.size() + src.size() );

        Enumeration src_elems  = src.elements();
        while ( src_elems.hasMoreElements() )
            super.addElement( src_elems.nextElement() );
        bytesize = super.size() * 4;
    }

    public void ReadFromDataStream( DataInputStream  data_istm,
                                    int              Nelem )
    throws IOException
    {
        super.ensureCapacity( Nelem );
        for ( int ii = Nelem; ii > 0; ii-- )
            super.addElement( new Integer( data_istm.readInt() ) );
        bytesize = Nelem * 4;
    } 

    public void WriteToDataStream( DataOutputStream  data_ostm )
    throws IOException
    {
        Enumeration enum = super.elements();
        while( enum.hasMoreElements() )
            data_ostm.writeInt( ( (Integer) enum.nextElement() ).intValue() );
    }

    public int NbytesInFile()
    {
        return bytesize;
    }

    //  Method toString() is inherited from Vector 

    public static void main( String args[] )
    {
        iarray A = new iarray();
        for ( int ii = 0; ii < 4; ii++ )
           A.addElement( new Integer( 10 + ii+1 ) );

        iarray B = new iarray();
        for ( int ii = 0; ii < 9; ii++ )
           B.addElement( new Integer( 20 + ii+1 ) );

        System.out.println( "A = " + A );
        System.out.println( "B = " + B );
 
        A.prepend( B );

        System.out.println( "A = " + A );
    }
}
