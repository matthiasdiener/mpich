import java.util.Vector;



//This class stores information about a message
public class ArrowInfo extends Info
{
    StateGroupLabel  begGroupID;     // timelines window's Y axis label (begin)
    StateGroupLabel  endGroupID;     // timelines window's Y axis label ( end )

    iarray           args;           // array of integer arguments
    RecDef           arrowDef;       // record display description
  
    //Constructor for the object representing a message.
    //origID  = Task Id of the StateGroup where the message originates
    //destID  = Task Id of the StateGroup where the message ends.
    //begtime = The time at which the message originates.
    public ArrowInfo( StateGroupLabel origID,  StateGroupLabel destID, 
                      double          begtime, double          endtime )
    {
        super ();
        begGroupID = new StateGroupLabel( origID );
        endGroupID = new StateGroupLabel( destID );
        begT       = begtime;
        endT       = endtime;
        lenT       = endT - begT;
    }
  
    public ArrowInfo( final SLOG_Irec_min     irec_min,
                      final SLOG_Irec_vtrargs irec_vtrargs,
                      final SLOG_ThreadInfos  thread_infos,
                      final int               view_idx,
                      final boolean           forward )
    {
        if ( forward ) {
            begT       = irec_min.starttime;
            lenT       = irec_min.duration;
            endT       = begT + lenT;
            begGroupID = new StateGroupLabel( irec_min.origID,
                                              thread_infos, view_idx );
            endGroupID = new StateGroupLabel( irec_min.destID,
                                              thread_infos, view_idx );
        }
        else {
            endT       = irec_min.starttime;
            lenT       = irec_min.duration;
            begT       = endT + lenT;
            begGroupID = new StateGroupLabel( irec_min.destID,
                                              thread_infos, view_idx );
            endGroupID = new StateGroupLabel( irec_min.origID,
                                              thread_infos, view_idx );
        }
        args    = new iarray( ( (SLOG_Irec_args) irec_vtrargs.firstElement() )
                              .elems );

        blink      = false;
    }

    public void SetArrowDefLink( RecDef in_arrowdef )
    {
        arrowDef = in_arrowdef;
    }

    public String toString () {
        return ("ArrowInfo=[begGroupID=" + begGroupID
                          + ", endGroupID=" + endGroupID
	                  + ", begT=" + begT
                          + ", endT=" + endT
                          + ", name=" + arrowDef.description 
                          + ", args=" + args + " ]" );
    }
} 
