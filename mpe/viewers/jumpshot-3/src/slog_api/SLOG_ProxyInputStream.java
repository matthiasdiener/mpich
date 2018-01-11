import java.rmi.*;
import java.rmi.server.*;
import java.io.*;

public class SLOG_ProxyInputStream
{
    private SLOG_RemoteServices     slog_ser    = null;
    private ObjID                   slog_objID  = null;
    private SLOG_RemoteInputStream  slog_obj    = null;

    public SLOG_ProxyInputStream( String   serviceURLname,
                                  String   filename )
    throws RemoteException, IOException, NotBoundException
    {
        if ( serviceURLname != null && serviceURLname.length() > 0 ) {
            slog_ser    = (SLOG_RemoteServices) Naming.lookup( serviceURLname );
            slog_objID  = slog_ser.OpenRemoteFile( filename );
            slog_obj    = slog_ser.GetRemoteStream( slog_objID );
        }
        else {
            slog_ser = null;
            slog_obj = new SLOG_RemoteInputStream( filename );
            slog_objID = slog_obj.GetObjID();
        }
    }

    public void SetRemoteServices( SLOG_RemoteServices in_slogd )
    {
        slog_ser = in_slogd;
    }

    public SLOG_RemoteServices GetRemoteServices()
    {
        return slog_ser;
    }

    public SLOG_Frame GetFrame( long file_ptr )
    throws IOException, IllegalArgumentException
    {
        if ( slog_ser != null )
            return slog_ser.GetRemoteFrame( slog_objID, file_ptr );
        else
            return slog_obj.GetFrame( file_ptr );
    }

    public void Close()
    throws RemoteException, IOException
    {
        if ( slog_ser != null )
            slog_ser.CloseRemoteFile( slog_objID );
        else
            slog_obj.Close();
    } 

    public SLOG_Header GetHeader()
    {
        if ( slog_obj != null )
            return slog_obj.GetHeader();
        else
            return null;
    }

    public SLOG_Statistics GetStatistics()
    {
        if ( slog_obj != null )
            return slog_obj.GetStatistics();
        else
            return null;
    }

    public SLOG_Preview GetPreview()
    {
        if ( slog_obj != null )
            return slog_obj.GetPreview();
        else
            return null;
    }

    public SLOG_Profile GetProfile()
    {
        if ( slog_obj != null )
            return slog_obj.GetProfile();
        else
            return null;
    }

    public SLOG_ThreadInfos GetThreadInfos()
    {
        if ( slog_obj != null )
            return slog_obj.GetThreadInfos();
        else
            return null;
    }

    public SLOG_RecDefs GetRecDefs()
    {
        if ( slog_obj != null )
            return slog_obj.GetRecDefs();
        else
            return null;
    }

    public SLOG_Dir GetDir()
    {
        if ( slog_obj != null )
            return slog_obj.GetDir();
        else
            return null;
    }

    public String toString()
    {
         StringBuffer rep = new StringBuffer();
         rep.append( "\t" + "slog_ser = " + slog_ser + "\n" );
         rep.append( slog_obj.toString() );
         return( rep.toString() );
    }
}
