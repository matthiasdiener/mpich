import java.rmi.*;
import java.rmi.server.*;
import java.io.*;

public class ts_RMI
{
    public static void main( String argv[] ) throws IOException
    {
        final int reg_port = 2005;
        String service_str = "//:" + reg_port + "/" + "SLOG_RemoteServer";
        String filename = "slogfile.data";
        long fptr2frame = 0;

        if ( argv.length >= 1 ) filename = argv[ 0 ];

        System.out.println( "Start reading" );
        System.out.flush();
        try {
            SLOG_RemoteServices slogd
            = (SLOG_RemoteServices) Naming.lookup( service_str );
            System.out.println( "slogd.class = " + slogd.getClass().getName() );
            /*
            if ( ! slogd.DoesRemoteFileExist( filename ) ) {
                System.out.println( filename + " does NOT exist!" );
                System.exit(0);
            }
            */  
            try {
                ObjID slog_objID = slogd.OpenRemoteFile( filename );
            }
            catch ( FileNotFoundException err ) {
                System.out.println( filename + " does NOT exist!" );
                System.exit(0);
            }
            System.out.println( "slog_objID = " + slog_objID );
            System.out.flush();
            SLOG_RemoteInputStream slog = slogd.GetRemoteStream( slog_objID );
            System.out.println( "slog.GetObjID() = " + slog.GetObjID() );
            System.out.flush();
            System.out.println( "\t __SLOG_RemoteInputStream__ \n" );
            System.out.flush();
            System.out.println( slog );
            System.out.flush();

            SLOG_Frame slog_frame;
            for ( int idx = 0; idx < slog.GetDir().NumOfFrames(); idx++ ) {
                if ( idx % 1 == 0 ) {
                    fptr2frame = slog.GetDir().EntryAt( idx ).fptr2framehdr;
                    System.out.println( "Main : fptr2frame = " + fptr2frame );
                    System.out.flush();
                    slog_frame = slogd.GetRemoteFrame( slog.GetObjID(),
                                                       fptr2frame );
                    System.out.println( "SLOG_Frame No. " + idx + ": \n" );
                    System.out.flush();
                    System.out.println( slog_frame );
                    System.out.flush();
                }
            }
            slogd.CloseRemoteFile( slog.GetObjID() );
        }
        catch ( NotSerializableException e ) {
            System.out.println( e );
            System.out.println( e.getMessage() + "could NOT be serialized" );
            e.printStackTrace();
        }
        catch ( Exception e ) {
            System.out.println( "ts_RMI: " + e );
            e.printStackTrace();
        }
    }
}
