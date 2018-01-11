import java.io.*;

public class slog_print
{
    public static void main( String argv[] )
    throws IOException, java.rmi.NotBoundException
    {
        String filename = "slogfile.data";
        long fptr2frame = 0;

        if ( argv.length >= 1 ) filename = argv[ 0 ];

        SLOG_ProxyInputStream slog = new SLOG_ProxyInputStream( null,
                                                                filename );
        System.out.println( "\t __SLOG_InputStream__ \n" );
        System.out.println( slog );

        for ( int idx = 0; idx < slog.GetDir().NumOfFrames(); idx++ ) {
            if ( idx % 1 == 0 ) {
                fptr2frame = slog.GetDir().EntryAt( idx ).fptr2framehdr;
                System.out.println( "Main : fptr2frame = " + fptr2frame );
                SLOG_Frame slog_frame = slog.GetFrame( fptr2frame );
                System.out.println( "SLOG_Frame No. " + idx + ": \n" );
                System.out.println( slog_frame );
            }
        }

    }
}
