import java.rmi.*;
import java.rmi.server.*;
import java.io.*;

public class slog_print_remote
{
    private static String protocolname = null;
    private static String machinename = "";
    private static int    reg_port = 2005;
    private static String servicename = "SLOG_RemoteServer";
    private static String service_str = null;
    private static String filename = "";

    private static void ParseCmdLineArgs( String argv[] )
    {
        int cmd_idx = 0;
        StringBuffer err_msg = new StringBuffer();
        try {
            while ( cmd_idx < argv.length ) {
                if ( argv[ cmd_idx ].equals( "-h" ) ) {
                    StringBuffer help_msg = new StringBuffer();
                    help_msg.append( "java slog_print_remote " );
                    help_msg.append( "[-h] " );
                    help_msg.append( "[-p protocol_name] " );
                    help_msg.append( "[-m server_machine_name] " );
                    help_msg.append( "[-r registry_port_integer] " );
                    help_msg.append( "[-s service_name] " );
                    help_msg.append( "[-f] slog_filename" );
                    System.out.println( help_msg );
                    System.out.flush();
                    System.exit( 0 );
                }
                else if ( argv[ cmd_idx ].equals( "-p" ) ) {
                    protocolname = argv[ ++cmd_idx ]; cmd_idx++;
                    err_msg.append( "protocol = " + protocolname + "\n");
                }
                else if ( argv[ cmd_idx ].equals( "-m" ) ) {
                    machinename = argv[ ++cmd_idx ]; cmd_idx++;
                    err_msg.append( "host = " + machinename + "\n" );
                }
                else if ( argv[ cmd_idx ].equals( "-r" ) ) {
                    reg_port = Integer.parseInt( argv[ ++cmd_idx ] ); cmd_idx++;                    err_msg.append( "registry port = " + reg_port + "\n" );
                }
                else if ( argv[ cmd_idx ].equals( "-s" ) ) {
                    servicename = argv[ ++cmd_idx ]; cmd_idx++;
                    err_msg.append( "service = " + servicename + "\n" );
                }
                else if ( argv[ cmd_idx ].equals( "-f" ) ) {
                    filename = argv[ ++cmd_idx ]; cmd_idx++;
                    err_msg.append( "file = " + filename + "\n" );
                }
                else {
                    filename = argv[ cmd_idx ]; cmd_idx++;
                    err_msg.append( "file = " + filename + "\n" );
                }
            }
        } catch ( ArrayIndexOutOfBoundsException err ) {
            System.err.println( err + " occurs at cmd_idx = " + cmd_idx );
            System.err.println( err_msg.toString() );
            err.printStackTrace();
        }
        err_msg = null;

        if ( protocolname != null )
            service_str = protocolname + "://" + machinename + ":" + reg_port
                        + "/" + servicename;
        else
            service_str = "//" + machinename + ":" + reg_port
                        + "/" + servicename;
    }

    private static String GetServiceURLname()
    {
        return service_str;
    }

    private static String GetLogFilename()
    {
        return filename;
    }



    public static void main( String argv[] ) throws IOException
    {
        ParseCmdLineArgs( argv );

        long fptr2frame = 0;
        SLOG_ProxyInputStream slog = null;

        System.out.println( "Start reading" );
        System.out.println( "ServiceURLname = " + GetServiceURLname() );
        System.out.flush();

        try {
            try {
                slog = new SLOG_ProxyInputStream( GetServiceURLname(), 
                                                  GetLogFilename() );
            }
            catch ( FileNotFoundException err ) {
                System.out.println( "File " + GetLogFilename()
                                  + " does NOT exist!" );
                // err.printStackTrace();
                System.exit(0);
            }
            System.out.flush();
            System.out.println( "\t __SLOG_ProxyInputStream__ \n" );
            System.out.flush();
            System.out.println( slog );
            System.out.flush();

            SLOG_Frame slog_frame;
            for ( int idx = 0; idx < slog.GetDir().NumOfFrames(); idx++ ) {
                if ( idx % 1 == 0 ) {
                    fptr2frame = slog.GetDir().EntryAt( idx ).fptr2framehdr;
                    System.out.println( "Main : fptr2frame = " + fptr2frame );
                    System.out.flush();
                    slog_frame = slog.GetFrame( fptr2frame );
                    System.out.println( "SLOG_Frame No. " + idx + ": \n" );
                    System.out.flush();
                    System.out.println( slog_frame );
                    System.out.flush();
                }
            }
            slog.Close();
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
