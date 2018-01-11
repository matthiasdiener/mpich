import java.awt.*;
import java.awt.event.*;
import java.net.*;
import java.applet.*;
import javax.swing.*;

public class MainApplet extends JApplet
{
    private static String protocolname;
    private static String machinename;
    private static int    reg_port;
    private static String servicename;
    private static String service_str;

    private static String logfile_dirname = null;
    private static String logfile_listingfile = null;

    public void init()
    {
        MainFrame.checkVersion();
        SetAppletParameters();
        String fullfilename = null;
        setContentPane( new Mainwin( this, false, fullfilename, 0 ) );
    }

    public void SetAppletParameters()
    {
        String tmp_str;

        tmp_str = getParameter( "protocol" );
        protocolname = tmp_str != null ? tmp_str : "";

        tmp_str = getParameter( "server_machine" );
        machinename = tmp_str != null ? tmp_str : getCodeBase().getHost();

        tmp_str = getParameter( "registry_port" );
        reg_port = tmp_str != null ? Integer.parseInt( tmp_str ) : 2005;

        tmp_str = getParameter( "service_name" );
        servicename = tmp_str != null ? tmp_str : "SLOG_RemoteServer";

        if ( protocolname != null && protocolname.length() > 0 )
            service_str = protocolname + "://" + machinename + ":" + reg_port
                        + "/" + servicename;
        else
            service_str = "//" + machinename + ":" + reg_port
                        + "/" + servicename;
    }

    public String GetServiceURLname()
    {
        return service_str;
    }

    
    public String GetLogFilePathPrefix()
    {
        String tmp_str;
        tmp_str = getParameter( "logfile_prefix" );
        logfile_dirname = tmp_str != null ? tmp_str : "";
        return logfile_dirname;
    }

    public String GetLogFileDirName()
    {
        String tmp_str;
        tmp_str = getParameter( "logfile_dir" );
        logfile_dirname = tmp_str != null ? tmp_str : "logfiles";
        return logfile_dirname;
    }

    public String GetLogFileListingFileName()
    {
        String tmp_str;
        tmp_str = getParameter( "logfile_listingfile" );
        logfile_listingfile = tmp_str != null ? tmp_str : "logfile_listing.txt";
        return logfile_listingfile;
    }

    public URL GetLogFileListingURL()
    {
        URL url = null;
        try {
            url = new URL( getCodeBase(), GetLogFileDirName() + "/"
                                        + GetLogFileListingFileName() );
            return url;
        }
        catch ( MalformedURLException e ) {
            new ErrorDialog( this, "MainApplet.GetLogFileListingURL(): "
                                 + "Bad URL( " + getCodeBase() + ", "
                                 + GetLogFileDirName() + "/"
                                 + GetLogFileListingFileName() + " )" );
            e.printStackTrace();
            return null;
        }
    }
}
