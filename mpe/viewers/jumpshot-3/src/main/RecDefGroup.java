import java.util.*;

public class RecDefGroup extends Vector
{
    public RecDefGroup()
    {
        super();
    }

    public RecDefGroup( final RecDefGroup all_recdefs, short in_intvltype )
    {
        super();

        RecDef           cur_def           = null;

        Enumeration defs = all_recdefs.elements();
        while ( defs.hasMoreElements() ) {
            cur_def = (RecDef) defs.nextElement();
            if ( cur_def.intvltype == in_intvltype )
                this.AddRecDef( cur_def );
        }
    }

    public void AddRecDef( final RecDef in_recdef )
    {
        RecDef           cur_def           = null;
        boolean          HasDefBeenAdded   = false;
        int              idx               = -1;
        short            in_intvltype      = in_recdef.intvltype;
        int              in_order          = in_recdef.bebits.GetOrder();

        Enumeration defs = super.elements();
        while ( defs.hasMoreElements() ) {
            idx ++;
            cur_def = (RecDef) defs.nextElement();
            //  RecDefs are sorted in accending order of intvltype & bebits
            if (    ( cur_def.intvltype > in_intvltype )
                 || (    cur_def.intvltype == in_intvltype
                      && cur_def.bebits.GetOrder() > in_order ) ) {
                super.insertElementAt( in_recdef, idx ); //  <-- Insert Elem ---
                HasDefBeenAdded = true;
                break;
            }
            else if ( cur_def.intvltype == in_intvltype
                      && cur_def.bebits.GetOrder() == in_order  ) {
                System.err.println( "Input RecDef " + in_recdef
                                  + "is duplicated!" );
                break;
            }
            else  // if (    ( cur_def.intvltype < in_intvltype )
                  //      || (    cur_def.intvltype == in_intvltype
                  //           && cur_def.bebits.GetOrder() < in_order ) )
                {  /*  Continue  */  }
        }   //  Endof  while ( defs.hasMoreElements() )

        if ( ! HasDefBeenAdded ) {
            super.addElement( in_recdef );               //  <--- Add Elem -----
            HasDefBeenAdded = true;
        }
    }

    /**
     * This function traverses through the vector containing state definitions
     * looking for the state definition with matching intvltype and bebits. 
     * Otherwise, a null is returned.
     */
    public RecDef GetRecDef( short ref_intvltype, final SLOG_bebits ref_bebits )
    {
        RecDef       cur_def;

        Enumeration defs = super.elements();
        while ( defs.hasMoreElements () ) {
            cur_def = ( RecDef ) defs.nextElement();
            if (    ref_intvltype == cur_def.intvltype
                 && ref_bebits.IsEqualTo( cur_def.bebits ) )
                return cur_def;
        }
        String err_msg = new String( "RecDefGroup.GetRecDef() : Error ! \n"
                                   + "\tCannot locate Record Definition "
                                   + "[ interval-type=" + ref_intvltype + ", "
                                   + "bibits=" + ref_bebits + " ]!\n" 
                                   + "\tCheck the Record Definition Table and "
                                   + "Display Profile using slog_print." );
        System.err.println( err_msg );
        new ErrorDialog( null, err_msg );
        return null;
    }

    public RecDef GetRecDef( final SLOG_Irec_min  irec_min )
    {
        return( this.GetRecDef( irec_min.intvltype, irec_min.bebits ) );
    }

    public RecDef GetRecDef( final IrecStack  irec_stack )
    {
        return( this.GetRecDef( irec_stack.GetIntvlType(),
                                SLOG_Const.ful_bebits ) );
    }

    public RecDef GetRecDef( final String name )
    {
        RecDef       cur_def;

        Enumeration defs = super.elements();
        while ( defs.hasMoreElements () ) {
            cur_def = ( RecDef ) defs.nextElement();
            if ( cur_def.description.compareTo( name ) == 0 )
                return cur_def;
        }

        return null;
    }
    
}
