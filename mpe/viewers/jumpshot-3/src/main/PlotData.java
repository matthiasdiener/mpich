import java.util.Vector;
import java.util.Random;
import java.util.Enumeration;
import java.awt.Color;
import java.io.*;

/**
 * This class is responsible for building data structures from records
 *  of the slog file. Each record read by SlogReader is passed here and
 * PlotData sifts out the relevent data from it.
 * 
 * Once the slog file is read, 4 data structures exist.
 * 
 * 1. (stateDefs) This is a vector of all the state definitions with increasing
 *    order of intvltype and bebits 
 * 2. Each of the state definition objects mentioned above contains a vector
 *    'stateVector' that is a collection of states of the same type
 *    in the ascending order of their end timestamps.
 * 3. (data) This is vector or all states of all types in ascending order 
 *    of their end timestamps.
 * 4. (a) This is an object that contains a vector 'arrows' which is a 
 *    list of ArrowInfo objects representing messages.
 */

public class PlotData
{
            SLOG_ProxyInputStream    slog                 = null;
            SLOG_Frame               slog_frame           = null;

            RecDefGroup              stateDefs;
            StateGroupListPair       all_states;
            RecDefGroup              arrowDefs;
            /* Not used in drawing canvas */
            ArrowList                all_arrows;
            Vector                   mtns;

            String                   view_indexes_label;



    /**
     * Constructor
     */
    public PlotData( final SLOG_ProxyInputStream in_slog, 
                     final SLOG_Frame       in_frame,
                           int              connect_idx,
                           int              view_idx )
    {
        slog              = in_slog;
        slog_frame        = in_frame;

        view_indexes_label = StateGroupLabel.GetIndexesLabel( view_idx );

        stateDefs  = new RecDefGroup();
        all_states = new StateGroupListPair();
        arrowDefs  = new RecDefGroup();
        all_arrows = new ArrowList();
        mtns       = new Vector();

        all_states.visible.Init( slog.GetThreadInfos(), connect_idx, view_idx );
        CreateStateDefsAndArrowDefs();
        CreateStatesAndArrows( connect_idx, view_idx );
        all_states.visible.SetBeginTime();
    }

    /**
     *  Scan SLOG_InputStream's profile to create the StateDefs[]
     */
    private void CreateStateDefsAndArrowDefs()
    {
        SLOG_ProfileEntry entry;
        String            entry_classtype;
        String            entry_description;
        RecDef            statedef, arrowdef;

        Enumeration profile = slog.GetProfile().GetEnumerationOfEntries();
        while ( profile.hasMoreElements() ) {
            entry    = ( SLOG_ProfileEntry ) profile.nextElement();
            entry_classtype = ( new String( entry.classtype ) ).toLowerCase();
            if ( entry_classtype.indexOf( "message" ) >= 0 ) {
                arrowdef = new RecDef( entry );
                arrowDefs.AddRecDef( arrowdef );
                entry_description = ( new String( entry.label ) ).toLowerCase();
                if ( entry_description.indexOf( "forward" ) >= 0 )
                    all_arrows.SetIntvltypeForForwardArrow( entry.intvltype );
                if ( entry_description.indexOf( "backward" ) >= 0 )
                    all_arrows.SetIntvltypeForBackwardArrow( entry.intvltype );
            }
            else {
                statedef = new RecDef( entry );
                stateDefs.AddRecDef( statedef );
            }
        }
    }

    private void SplitSectorIntoStatesAndArrows( SLOG_FrameSector sector,
                                                 int              connect_idx,
                                                 int              view_idx )
    {
        IrecStack           irec_stack;
        SLOG_Irec           irec;
        RecDefGroup         sub_statedefs;
        RecDef              statedef;
        RecDef              arrowdef;
        Enumeration         irecs;
        boolean             isforward;

        SLOG_ThreadInfos    slog_threadinfos = slog.GetThreadInfos();
        IrecGroupList       buffer = new IrecGroupList( slog_threadinfos,
                                                        view_idx );

        /*
        if ( connect_idx != 0 )
            buffer.Init( slog_threadinfos, view_idx );
        */

        irecs = sector.irecs.elements();
        while ( irecs.hasMoreElements() ) {
            irec = ( SLOG_Irec ) irecs.nextElement();
            if ( irec.fix.rectype == CONST.RectypeForMessage ) {
                isforward = all_arrows.IsForwardArrow( irec.fix.intvltype );
                ArrowInfo arrow = new ArrowInfo( irec.fix,
                                                 irec.vtrargs,
                                                 slog_threadinfos,
                                                 view_idx,
                                                 isforward );

                arrowdef = arrowDefs.GetRecDef( irec.fix );
                arrow.SetArrowDefLink( arrowdef );

                arrowdef.stateVector.addElement( arrow );     // For histogram
                all_arrows.addElement( arrow );
                debug.println( arrow + ",  " + isforward );
            }
            else {
                if ( connect_idx == 0 || irec.fix.bebits.IsWholeIntvl() ) {
                    StateInfo state = new StateInfo( irec.fix,
                                                     irec.vtrargs,
                                                     slog_threadinfos,
                                                     view_idx );

                    statedef = stateDefs.GetRecDef( irec.fix );
                    state.SetStateDefLink( statedef );

                    statedef.stateVector.addElement( state ); // For histogram
                    all_states.visible.AddStateInfo( state );
                }
                else {  // connect_idx == 1 && ! irec.fix.bebits.IsWholeIntvl()
                    buffer.AddIrec( irec );
                    if ( irec.fix.bebits.IsFinalIntvl() ) {
                        irec_stack = buffer.GetIrecStack( irec.fix );

                        statedef = stateDefs.GetRecDef( irec_stack );
                        sub_statedefs = new RecDefGroup( stateDefs,
                                                  irec_stack.GetIntvlType() );
                        StateInfo state = new StateInfo( irec_stack,
                                                         sub_statedefs,
                                                         slog_threadinfos,
                                                         view_idx );
                        state.SetStateDefLink( statedef );

                        statedef.stateVector.addElement( state );
                        all_states.visible.AddStateInfo( state );
                    }
                }
            }
        }

        if ( connect_idx != 0 ) {
            IrecGroupListEnumerator irec_stacks = buffer.IrecStackElements();
            while ( irec_stacks.HasMoreIrecStacks() ) {
                irec_stack = irec_stacks.NextIrecStack();

                statedef = stateDefs.GetRecDef( irec_stack );
                sub_statedefs = new RecDefGroup( stateDefs,
                                          irec_stack.GetIntvlType() );
                StateInfo state = new StateInfo( irec_stack,
                                                 sub_statedefs,
                                                 slog_threadinfos,
                                                 view_idx );
                state.SetStateDefLink( statedef );

                statedef.stateVector.addElement( state );
                all_states.visible.AddStateInfo( state );
            }
        }

/*
        if ( connect_idx != 0 ) {
            IrecGroup   cur_group;
            Enumeration stacks;
            Enumeration groups = buffer.elements();
            while ( groups.hasMoreElements() ) {
                cur_group = ( IrecGroup ) groups.nextElement();
                stacks = cur_group.elements();
                while ( stacks.hasMoreElements() ) {
                    irec_stack = ( IrecStack ) stacks.nextElement();

                    System.out.println( "B: " + irec_stack );

                    statedef = stateDefs.GetRecDef( irec_stack );
                    sub_statedefs = new RecDefGroup( stateDefs,
                                              irec_stack.GetIntvlType() );
                    StateInfo state = new StateInfo( irec_stack,
                                                     sub_statedefs,
                                                     slog_threadinfos,
                                                     view_idx );
                    state.SetStateDefLink( statedef );
  
                    statedef.stateVector.addElement( state );
                    all_states.visible.AddStateInfo( state );
                }
            }
        }
*/

    }

    private void CreateStatesAndArrows( int connect_idx, int view_idx )
    {
        /*
        //   For Current Sector
        SplitSectorIntoStatesAndArrows( slog_frame.cur,
                                        connect_idx, view_idx );

        //   For Incoming Sector
        SplitSectorIntoStatesAndArrows( slog_frame.inc,
                                        connect_idx, view_idx );

        //   For Passing Through Sector
        SplitSectorIntoStatesAndArrows( slog_frame.pas,
                                        connect_idx, view_idx );

        //   For Outgoing Sector
        SplitSectorIntoStatesAndArrows( slog_frame.out,
                                        connect_idx, view_idx );
        */

        SLOG_FrameSector frame = new SLOG_FrameSector( slog_frame.cur );
        frame.prepend( slog_frame.inc );
        frame.append( slog_frame.pas );
        frame.append( slog_frame.out );

        SplitSectorIntoStatesAndArrows( frame, connect_idx, view_idx );
    }

    protected void finalize() throws Throwable
    { 
        stateDefs   = null;
        all_states  = null;
        arrowDefs   = null;
        all_arrows  = null;
        mtns        = null;

        super.finalize();
    }
}
