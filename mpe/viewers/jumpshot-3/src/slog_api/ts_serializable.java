import java.io.*;

public class ts_serializable
{
    public static void main( String argv[] ) throws IOException
    {
        String filename     = "slogfile.data";
        String tmp_filename = "tmp_serialized_slog";
        long fptr2frame = 0;

        if ( argv.length >= 1 ) filename = argv[ 0 ];

        /*
        ByteArrayOutputStream bary_out = new ByteArrayOutputStream();
        ObjectOutputStream    obj_out  = new ObjectOutputStream( bary_out );
        */
        FileOutputStream   file_out = new FileOutputStream( tmp_filename );
        ObjectOutputStream obj_out  = new ObjectOutputStream( file_out );
        try {
            SLOG_RemoteInputStream slog = new SLOG_RemoteInputStream(filename);
            obj_out.writeObject( slog );
            // System.out.println( "\t __SLOG_RemoteInputStream__ \n" );
            // System.out.println( slog );

            for ( int idx = 0; idx < slog.GetDir().NumOfFrames(); idx++ ) {
                if ( idx % 1 == 0 ) {
                    fptr2frame = slog.GetDir().EntryAt( idx ).fptr2framehdr;
                    //System.out.println( "Main : fptr2frame = " + fptr2frame );
                    SLOG_Frame slog_frame = slog.GetFrame( fptr2frame );
                    // System.out.println( "SLOG_Frame No. " + idx + ": \n" );
                    // System.out.println( slog_frame );
                    obj_out.writeObject( slog_frame );
                }
            }
        }
        catch ( Exception e ) {
            e.printStackTrace();
        }
        obj_out.flush();
        obj_out.close();
        file_out.close();


        /*
        ByteArrayInputStream bary_in = new ByteArrayInputStream(
                                           bary_out.toByteArray() );
        ObjectInputStream    obj_in  = new ObjectInputStream( bary_in );
        */
        FileInputStream   file_in = new FileInputStream( tmp_filename );
        ObjectInputStream obj_in  = new ObjectInputStream( file_in );
        try {
            SLOG_RemoteInputStream S_slog
            = (SLOG_RemoteInputStream) obj_in.readObject();
            System.out.println( "\t __Serialized_SLOG_RemoteInputStream__ \n" );
            System.out.println( S_slog );

            for ( int idx = 0; idx < S_slog.GetDir().NumOfFrames(); idx++ ) {
                if ( idx % 1 == 0 ) {
                    fptr2frame = S_slog.GetDir().EntryAt( idx ).fptr2framehdr;
                    System.out.println( "Main : fptr2frame = " + fptr2frame );
                    SLOG_Frame S_slog_frame = (SLOG_Frame) obj_in.readObject();
                    System.out.println( "SLOG_Frame No. " + idx + ": \n" );
                    System.out.println( S_slog_frame );
                }
            }
        }
        catch ( Exception e ) {
            e.printStackTrace();
        }
        obj_in.close();
        file_in.close();

    }
}
