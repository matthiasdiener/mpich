import java.rmi.*;
import java.rmi.server.*;
import java.rmi.registry.*;
import java.io.*;
import java.util.Vector;

public class SLOG_RemoteServer extends    UnicastRemoteObject
                               implements SLOG_RemoteServices
{
    private static String protocolname = null;
    private static String machinename = "";
    private static int    reg_port = 2005;
    private static String servicename = "SLOG_RemoteServer";
    private static String servicelogname = "slogd.log";
    private static String service_str = null;
    private static boolean IsLogging = false;

    private        Vector  slogs;     //  Collection of slogs being served
    private static int     seedID = 0;
    private PrintStream    log_stm;

    public SLOG_RemoteServer()
    throws RemoteException
    {
        super();
        slogs = new Vector();
        log_stm = null;
    }

    public SLOG_RemoteServer( String logfilename )
    throws RemoteException, IOException, FileNotFoundException
    {
        super();
        slogs = new Vector();
        setLog( new FileOutputStream( logfilename ) );
        log_stm = getLog();
    }

    private void LogPrintln( String str )
    {
        if ( log_stm != null )
            log_stm.println( str );
    }

    private void LogPrint( String str )
    {
        if ( log_stm != null )
            log_stm.print( str );
    }

    //  Implementation of SLOG_RemoteServices interface
    public boolean                DoesRemoteFileExist( String filename )
    throws RemoteException
    {
        File logfile = new File( filename );
        return logfile.exists();
    }

    //  Implementation of SLOG_RemoteServices interface
    public ObjID                  OpenRemoteFile( String filename )
    throws RemoteException, IOException
    {
        SLOG_RemoteInputStream slog = new SLOG_RemoteInputStream( filename );
        slogs.addElement( slog );
        LogPrintln( "OpenRemoteFile(" + filename + "): Opened with ObjID = "
                  + slog.GetObjID() );
        return slog.GetObjID();
    }

    //  Implementation of SLOG_RemoteServices interface
    public SLOG_RemoteInputStream GetRemoteStream( ObjID slog_objID )
    throws RemoteException
    {
        SLOG_RemoteInputStream slog;
        for ( int idx = slogs.size()-1; idx >= 0; idx-- ) {
            slog = (SLOG_RemoteInputStream) slogs.elementAt( idx );
            if ( slog.GetObjID().equals( slog_objID ) ) {
                LogPrintln( "GetRemoteStream(" + slog_objID + "): Matched "
                          + "with slogs[" + idx + "]" );
                return slog;
            }
        }
        LogPrintln( "GetRemoteStream(" + slog_objID + "): Failed!" );
        return null;
    }

    //  Implementation of SLOG_RemoteServices interface
    public SLOG_Frame             GetRemoteFrame( ObjID slog_objID,
                                                  long file_ptr )
    throws RemoteException, IOException
    {
        SLOG_RemoteInputStream slog;
        for ( int idx = slogs.size()-1; idx >= 0; idx-- ) {
            slog = (SLOG_RemoteInputStream) slogs.elementAt( idx );
            if ( slog.GetObjID().equals( slog_objID ) ) {
                LogPrintln( "GetRemoteFrame(" + slog_objID + "): Matched "
                          + "with slogs[" + idx + "]" );
                /*
                SLOG_InputStream local_slog = (SLOG_InputStream) slog;
                SLOG_Frame frame = local_slog.GetFrame( file_ptr );
                System.out.println( frame );
                System.out.flush();
                return frame;
                */
                return ( (SLOG_InputStream) slog ).GetFrame( file_ptr );
            }
        }
        LogPrintln( "GetRemoteFrame(" + slog_objID + "): Failed!" );
        return null;
    }

    //  Implementation of SLOG_RemoteServices interface
    public void                   CloseRemoteFile( ObjID slog_objID )
    throws RemoteException, IOException
    {
        int idx;
        SLOG_RemoteInputStream slog;
        for ( idx = slogs.size()-1; idx >= 0; idx-- ) {
            slog = (SLOG_RemoteInputStream) slogs.elementAt( idx );
            if ( slog.GetObjID().equals( slog_objID ) ) {
                slog.Close();
                break;
            }
        }
        if ( idx >= 0 ) {
            LogPrintln( "CloseRemoteFile(" + slog_objID + "): Matched "
                      + "with slogs[" + idx + "]" );
            slogs.removeElementAt( idx );
            slog = null;
        }
        else
            LogPrintln( "CloseRemoteFile(" + slog_objID + "): Failed!" );
    }

    private static void ParseCmdLineArgs( String argv[] )
    {
        int cmd_idx = 0;
        StringBuffer err_msg = new StringBuffer();
        try {
            while ( cmd_idx < argv.length ) {
                if ( argv[ cmd_idx ].equals( "-h" ) ) {
                    StringBuffer help_msg = new StringBuffer();
                    help_msg.append( "java SLOG_RemoteServer " );
                    help_msg.append( "[-h] " );
                    help_msg.append( "[-p protocol_name] " );
                    help_msg.append( "[-m server_machine_name] " );
                    help_msg.append( "[-r registry_port_integer] " );
                    help_msg.append( "[-s service_name] " );
                    help_msg.append( "[-l [servicelog_filename]] " );
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
                else if ( argv[ cmd_idx ].equals( "-l" ) ) {
                    IsLogging = true;
                    cmd_idx++;
                    if ( cmd_idx < argv.length )
                        if ( ! argv[ cmd_idx ].startsWith( "-" ) ) {
                            servicelogname = argv[ cmd_idx ]; cmd_idx++;
                            err_msg.append( "servicelog = "
                                          + servicelogname + "\n" );
                        }
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
        System.out.println( "service_str = " + service_str );
        return service_str;
    }



    public static void main( String args[] )
    {
        SLOG_RemoteServer slog_server;

        ParseCmdLineArgs( args );

        /*
        if ( System.getSecurityManager() == null ) {
            System.setSecurityManager( new RMISecurityManager() );
        }
        */

        System.out.println( "Starting " + servicename );
        System.out.println( "\t" + "with Service URL = " + service_str );
        try {
            LocateRegistry.createRegistry( reg_port );
            if ( IsLogging ) {
                slog_server = new SLOG_RemoteServer( servicelogname );
                System.out.println( "\t" + "with activties logged in file = "
                                  + servicelogname );
            }
            else
                slog_server = new SLOG_RemoteServer();
            Naming.rebind( service_str, slog_server );
        }
        catch ( Exception err ) {
            System.err.println( servicename + " fails to start!" );
            // System.err.println( err );
            err.printStackTrace();
        }
    }
}
