import java.util.Vector;



//This class represents Rectangle( State ) Info
public class StateInfo extends Info
{ 
    StateGroupLabel  groupID;        // timelines window's Y axis label

    iarray           args;           // array of integer arguments
    RecDef           stateDef;       // record display desciption

    int              level = 0;      // For nested states
    StateInfo        higher = null;  // Pointer to the next higher state
                                     // enveloping this state.
    int              instr_addr1;    // source code instruction address( beg )
    int              instr_addr2;    // source code instruction address( end )

    //Constructor 
    public StateInfo() 
    { 
        super();
    }
  
    public StateInfo( final SLOG_Irec_min     irec_min,
                      final SLOG_Irec_vtrargs irec_vtrargs,
                      final SLOG_ThreadInfos  thread_infos,
                      final int               view_idx )
    {
        begT        = irec_min.starttime;
        lenT        = irec_min.duration;
        endT        = begT + lenT;
        blink       = false;
        groupID     = new StateGroupLabel( irec_min.origID, 
                                           thread_infos, view_idx );
        args        = new iarray( ((SLOG_Irec_args) irec_vtrargs.firstElement())
                                  .elems );
        instr_addr2 = irec_min.instr_addr;
        instr_addr1 = SLOG_Const.NULL_iaddr;
    }

    public StateInfo(       IrecStack         irec_stack,
                      final RecDefGroup       statedefs,
                      final SLOG_ThreadInfos  thread_infos,
                      final int               view_idx )
    {
        SLOG_Irec        irec;
        SLOG_Irec_min    irec_min;
        double           irec_begtime, irec_endtime;
        iarray           irec_args, tmp_args;
        short            irec_itype;
        int              irec_order;
        int              prev_order, beg_order, end_order;
        int              Nargs;

        RecDef           statedef;

        irec         = ( SLOG_Irec ) irec_stack.pop();
        irec_min     = irec.fix;
        irec_begtime = irec_min.starttime;
        irec_endtime = irec_begtime + irec_min.duration;
        irec_order   = irec_min.bebits.GetOrder();
        irec_itype   = irec_stack.GetIntvlType();

        begT         = irec_begtime;
        endT         = irec_endtime;
        lenT         = irec_endtime - irec_begtime;
        blink        = false;
        groupID      = new StateGroupLabel( irec.fix.origID, 
                                            thread_infos, view_idx );
        args         = new iarray( ( (SLOG_Irec_args)
                                     irec.vtrargs.firstElement() ).elems );

        // Since the 1st from the Stack will be the last in time, 
        // Initialize instr_addr
        instr_addr2  = irec_min.instr_addr;
        instr_addr1  = SLOG_Const.NULL_iaddr;

        end_order    = irec_order;  // The 1st from Stack is the last in time
        beg_order    = irec_order;

        while ( ! irec_stack.empty() ) {
            prev_order   = irec_order;

            irec         = ( SLOG_Irec ) irec_stack.pop();
            irec_min     = irec.fix;
            irec_begtime = irec_min.starttime;
            irec_endtime = irec_begtime + irec_min.duration;
            irec_args    = ( (SLOG_Irec_args) irec.vtrargs.firstElement() )
                           .elems;
            //  If the current Irec's instr_addr is NOT null( 0 ),
            //  update the "instr_addr"
            if ( irec_min.instr_addr != SLOG_Const.NULL_iaddr )
                instr_addr1 = irec_min.instr_addr;

            irec_order   = irec_min.bebits.GetOrder();

            if ( irec_order > prev_order ) {
                if ( irec_args != null ) args.append( irec_args );
            }
            else if ( irec_order == prev_order ) {
                if ( irec_args != null ) {
                    if ( irec_begtime < begT )
                        args.prepend( irec_args );
                    if ( irec_endtime > endT )
                        args.append( irec_args );
                }
            }
            else { // if ( irec_order < prev_order )
                if ( irec_args != null ) args.prepend( irec_args );
            }

            if ( irec_begtime < begT ) {
                begT = irec_begtime;
                beg_order = irec_order;
            }
            if ( irec_endtime > endT ) {
                endT = irec_endtime;
                end_order = irec_order;
            }
        }   // End of while ( ! irec_stack.empty() )

        lenT = endT - begT;

        // prepend NULL arguments if the begin bebit interval is NOT found
        if ( beg_order != 0 ) {
            statedef     = statedefs.GetRecDef( irec_itype,
                                                SLOG_Const.beg_bebits );
            if ( statedef != null ) {
                begT     = Double.NEGATIVE_INFINITY;
                if ( statedef.arg_labels != null ) {
                    Nargs        = statedef.arg_labels.size(); 
                    if ( Nargs > 0 ) {
                        tmp_args     = new iarray();
                        Integer itmp = new Integer( Integer.MIN_VALUE );
                        for ( int ii = 0; ii < Nargs; ii++ )
                            tmp_args.addElement( itmp );
                        args.prepend( tmp_args );
                    }
                }
            }
            else {
                String err_msg = new String( "StateInfo(): "
                                           + "Since the BEGIN bebit interval "
                                           + "for the state cannot be found, \n"
                                           + "\tthe state will be marked as "
                                           + "connected" );
                System.err.println( err_msg );
                new WarnDialog( null, err_msg );
            }
        }
        // append NULL arguments if the end bebit interval is NOT found
        if ( end_order != 2 ) {
            statedef     = statedefs.GetRecDef( irec_itype,
                                                SLOG_Const.end_bebits );
            if ( statedef != null ) {
                endT     = Double.POSITIVE_INFINITY;
                if ( statedef.arg_labels != null ) {
                    Nargs        = statedef.arg_labels.size(); 
                    if ( Nargs > 0 ) {
                        tmp_args   = new iarray();
                        Integer itmp = new Integer( Integer.MIN_VALUE );
                        for ( int ii = 0; ii < Nargs; ii++ )
                            tmp_args.addElement( itmp );
                        args.append( tmp_args );
                    }
                }
            }
            else {
                String err_msg = new String( "StateInfo(): "
                                           + "Since the FINAL bebit interval "
                                           + "for the state cannot be found, \n"
                                           + "\tthe state will be marked as "
                                           + "connected" );
                System.err.println( err_msg );
                new WarnDialog( null, err_msg );
            }
        }

    }

    public void SetStateDefLink( RecDef in_statedef )
    {
        stateDef = in_statedef;
    }

    public String toString()
    {
        return ( "StateInfo=[ level=" + level
                           + ", groupID=" + groupID
                           + ", begT=" + begT
                           + ", endT=" + endT
                           + ", lenT=" + lenT
                           + ", name=" + stateDef.description
                           + ", args=" + args + " ]" );
    }
}
