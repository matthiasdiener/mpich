import java.io.*;

    class SLOG_bebits
    {
        private byte[]        bits;
       
        public SLOG_bebits()
        {
            bits      = new byte[ 2 ];
        }

        public SLOG_bebits( byte bit_0, byte bit_1 )
        {
            bits      = new byte[ 2 ];
            bits[ 0 ] = bit_0;
            bits[ 1 ] = bit_1;
        }

        public SLOG_bebits( final SLOG_bebits src )
        {
            bits      = new byte[ 2 ];
            bits[ 0 ] = src.bits[ 0 ];
            bits[ 1 ] = src.bits[ 1 ];
        }

        public SLOG_bebits( DataInputStream data_istm )
        throws IOException
        {   
            bits      = new byte[ 2 ];
            this.ReadFromDataStream( data_istm );
        }

        public SLOG_bebits( RandomAccessFile file_stm )
        throws IOException
        {  
            bits      = new byte[ 2 ];
            this.ReadFromRandomFile( file_stm );
        }
        
        private byte conv( byte bit )
        {
            return  (byte)( bit > 0 ? 1 : 0 );
        }

        public byte encode()
        {
            return (byte)( 02 * conv( bits[1] ) + conv( bits[0] ) );
            /*
            return (byte)(   conv( bits[1] ) << 1
                           + conv( bits[0] ) );
            */
        }

        public  void decode( byte in_bebits )
        throws IllegalArgumentException
        {
            if ( in_bebits >= 0 && in_bebits <= 3 ) {
                bits[ 0 ] = (byte) ( in_bebits % 2 );
                bits[ 1 ] = (byte) ( in_bebits / 2 );
                /*
                bits[ 0 ] = (byte) ( in_bebits & 0x01 );
                bits[ 1 ] = (byte) ( in_bebits >> 1 );
                */
            }
            else
                throw new IllegalArgumentException( "input bebits = " 
                                                  + in_bebits );
        }

        public boolean IsEqualTo( final SLOG_bebits the_bebits )
        {
            if (    bits[0] == the_bebits.bits[0]
                 && bits[1] == the_bebits.bits[1] )
                return true;
            else
                return false;
        }

        public void ReadFromDataStream( DataInputStream data_istm )
        throws IOException, IllegalArgumentException
        {   
            byte tmp = data_istm.readByte();
            this.decode( tmp );
        }  

        public void ReadFromRandomFile( RandomAccessFile file_stm )
        throws IOException, IllegalArgumentException
        {   
            byte tmp = file_stm.readByte();
            this.decode( tmp );
        }  

        public void WriteToDataStream( DataOutputStream data_ostm )
        throws IOException
        {
            byte tmp = this.encode();
            data_ostm.writeByte( tmp );
        }
    
        public void WriteToRandomFile( RandomAccessFile file_stm )
        throws IOException
        {
            byte tmp = this.encode();
            file_stm.writeByte( tmp );
        }
    
        public boolean IsBeginIntvl()
        {
            return ( bits[ 0 ] == 1 && bits[ 1 ] == 0 );
        }

        public boolean IsMiddleIntvl()
        {
            return ( bits[ 0 ] == 0 && bits[ 1 ] == 0 );
        }

        public boolean IsFinalIntvl()
        {
            return ( bits[ 0 ] == 0 && bits[ 1 ] == 1 );
        }

        public boolean IsWholeIntvl()
        {
            return ( bits[ 0 ] == 1 && bits[ 1 ] == 1 );
        }

        public int GetOrder()
        {
            if ( IsBeginIntvl() )
                return 0;
            else if ( IsMiddleIntvl() )
                return 1;
            else if ( IsFinalIntvl() )
                return 2;
            else if ( IsWholeIntvl() )
                return 3;
            else {
                System.err.println( "bebit " + toString() + " is invalid" );
                return -1;
            }
        }

        public String toString()
        {
            StringBuffer representation = new StringBuffer();

            if ( IsBeginIntvl() )
                representation.append( "(begin)" );
            else if ( IsMiddleIntvl() )
                representation.append( "(middle)" );
            else if ( IsFinalIntvl() )
                representation.append( "(final)" );
            else if ( IsWholeIntvl() )
                /* representation.append( "(whole)" ) */ ;
            else
                representation.append( "(" + bits[ 0 ]
                                    + ", " + bits[ 1 ] + ")" );

            return ( representation.toString() );
        }
    }
