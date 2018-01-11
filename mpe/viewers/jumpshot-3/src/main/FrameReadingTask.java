import java.io.*;

public class FrameReadingTask
{
    private ViewFrameChooser       frame_chooser  = null;
    private SLOG_ProxyInputStream  slog           = null;
    private int                    frame_idx      = 0;
    private int                    connect_idx    = 0;
    private int                    view_idx       = 0;

    private SLOG_Frame             slog_frame     = null;
    private PlotData               slog_plotdata  = null;

    private SwingWorker            read_worker;

    private final int              max_prog_idx   = 100;
    private final int              min_prog_idx   = 0;
    private int                    cur_prog_idx   = min_prog_idx;

    private String                 status_str = "";
    private boolean                plotdata_ready = false;

    private static int             ONE_SECOND     = 1000;
    private final  int             refresh_intvl  = ONE_SECOND / 10;

    public FrameReadingTask(       ViewFrameChooser       in_chooser,
                             final SLOG_ProxyInputStream  in_slog,
                             final int                    in_frame_idx,
                             final int                    in_connect_idx,
                             final int                    in_view_idx )
    {
        frame_chooser = in_chooser;
        slog          = in_slog;
        frame_idx     = in_frame_idx;
        connect_idx   = in_connect_idx;
        view_idx      = in_view_idx;
    }

    public void Start()
    {
        read_worker = new SwingWorker()
        {
            public Object construct()
            {
                plotdata_ready = false;
                ReadFrame();
                ConvertFrameToPlotData();
                return( slog_plotdata );
            }
            public void finished()
            {
                slog_plotdata = ( PlotData ) get();
                plotdata_ready = true;
                frame_chooser.DisplayFrame();
            }
        };
        read_worker.start();
    }

    private void ReadFrame()
    {
        SetCurProgIdx( min_prog_idx );

        SLOG_DirEntry dir_entry = slog.GetDir().EntryAt( frame_idx );
        try {
            SetStatusFieldText( "Reading Frame from Logfile" );
            slog_frame = slog.GetFrame( dir_entry.fptr2framehdr );
        } catch ( IOException ioerr ) {
            String err_msg = new String( "FrameReadingTask: "
                                       + "IO Error in slog.GetFrame()" );
            SetStatusFieldText( err_msg );
            System.err.println( err_msg );
            new ErrorDialog( null, err_msg );
        }
    } 

    private void ConvertFrameToPlotData()
    {
        SetCurProgIdx( max_prog_idx * 5 / 10 );

        //  Artifical code to slow the code segment down
        //  for ( int ii = 0; ii < 100000; ii++ ) System.out.print( "" );

        if ( slog_frame != null ) {
            SetStatusFieldText( "Converting Selected Frame to Plotting data" );
            try {
                slog_plotdata = new PlotData( slog, slog_frame,
                                              connect_idx, view_idx );
            } catch ( RuntimeException rterr ) {
                String err_msg = new String( "FrameReadingTask: "
                                           + "Runtime Error in "
                                           + "PlatData[] constructor" );
                SetStatusFieldText( err_msg );
                System.err.println( err_msg );
                new ErrorDialog( null, err_msg );
            }
        }
        else {
            String err_msg = new String( "FrameReadingTask: "
                                       + "slog.GetFrame() returns NULL" );
            SetStatusFieldText( err_msg );
            System.err.println( err_msg );
            new ErrorDialog( null, err_msg );
        }

        SetCurProgIdx( max_prog_idx - 1 );
        SetStatusFieldText( "Ending...." );
    } 

    public void Stop()
    {
        read_worker.interrupt();
    }

    public boolean Finished()
    {    return ( plotdata_ready );   }

    private void SetStatusFieldText( String str )
    {  
        try {
            status_str = str;
            // put this thread to sleep to let event-dispatching thread
            // to refresh spawning GUI 
            Thread.sleep( refresh_intvl );
            // Thread.yield();
        } catch ( InterruptedException interrupt ) {}
    }

    private void SetCurProgIdx( int idx )
    {
        try {
            cur_prog_idx = idx;
            // put this thread to sleep to let event-dispatching thread
            // to refresh spawning GUI 
            Thread.sleep( refresh_intvl );
            // Thread.yield();
        } catch ( InterruptedException interrupt ) {}
    }

    public String GetMessage()
    {   return status_str;   }

    public int GetMinProgIdx()
    {   return min_prog_idx;    }

    public int GetMaxProgIdx()
    {    return max_prog_idx;   }

    public int GetCurProgIdx()
    {    return cur_prog_idx;   }

    public PlotData GetPlotData()
    {    return slog_plotdata;   }
}
