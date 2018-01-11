import java.rmi.server.ObjID;
import java.io.*;

public class SLOG_RemoteInputStream extends SLOG_InputStream
                                    implements Serializable
{
    private ObjID                    obj_ID;

    public SLOG_RemoteInputStream( String filename )
    throws IOException
    {
        super( filename );
        obj_ID = new ObjID();
    }

    public void SetObjID( ObjID new_obj_ID )
    {
        obj_ID = new_obj_ID;
    }

    public ObjID GetObjID()
    {
        return obj_ID;
    }

    public String toString()
    {
         StringBuffer rep = new StringBuffer();
         rep.append( "\t" + "obj_ID = " + obj_ID + "\n" );
         rep.append( super.toString() );
         return( rep.toString() );
    }
}
