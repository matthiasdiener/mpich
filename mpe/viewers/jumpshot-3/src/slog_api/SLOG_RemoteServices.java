import java.rmi.*;
import java.rmi.server.*;
import java.io.*;

public interface SLOG_RemoteServices extends Remote
{
     public boolean                DoesRemoteFileExist( String filename )
     throws RemoteException;

     public ObjID                  OpenRemoteFile( String filename )
     throws RemoteException, IOException;

     public SLOG_RemoteInputStream GetRemoteStream( ObjID slog_objID )
     throws RemoteException;

     public SLOG_Frame             GetRemoteFrame( ObjID slog_objID,
                                                   long file_ptr )
     throws RemoteException, IOException;

     public void                   CloseRemoteFile( ObjID slog_objID )
     throws RemoteException, IOException;
}
