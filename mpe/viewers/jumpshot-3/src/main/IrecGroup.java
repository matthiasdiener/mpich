import java.util.*;

/*
    Vector of IrecStack which has Irec.intvltype as identifier
*/
public class IrecGroup extends Vector
{
    private StateGroupLabel  groupID;

    public IrecGroup( int major_ID, int minor_ID )
    {
        super();
        groupID         = new StateGroupLabel( major_ID, minor_ID );
    }

    public IrecGroup( StateGroupLabel label )
    {
        super();
        groupID         = new StateGroupLabel( label );
    }

    public StateGroupLabel GetGroupLabel()
    {
        return( groupID );
    }

    public void AddIrec( final SLOG_Irec in_Irec )
    {
        IrecStack        cur_stack         = null;
        boolean          HasIrecBeenAdded  = false;
        int              idx               = -1;
        short            in_intvltype      = in_Irec.fix.intvltype;

        Enumeration stacks = super.elements();
        while ( stacks.hasMoreElements() ) {
            idx ++;
            cur_stack = (IrecStack) stacks.nextElement();
            //  Assume the stacks is sorted in accending order of intvltype
            if ( cur_stack.GetIntvlType() > in_intvltype ) {
                // Create the corresponding IrecStack with the given intvltype
                IrecStack istack = new IrecStack( in_intvltype );
                super.insertElementAt( istack, idx );   //  <--- Insert Elem --

                // Add the in_Irec to the appropriate IrecStack
                istack.push( in_Irec );
                HasIrecBeenAdded = true;
                break;
            }
            else if ( cur_stack.GetIntvlType() == in_intvltype  ) {
                // Add the in_Irec to the appropriate IrecStack
                cur_stack.push( in_Irec );
                HasIrecBeenAdded = true;
                break;
            }
            else  // if ( cur_stack.GetIntvlType() < in_intvltype )
                {  /*  Continue  */  }
        }   //  Endof  while ( stacks.hasMoreElements() )

        if ( ! HasIrecBeenAdded ) {
            // Create the corresponding IrecStack with the given intvltype
            IrecStack istack = new IrecStack( in_intvltype );
            super.addElement( istack );                 //  <--- Add Elem -----

            // Add the in_Irec to the appropriate IrecStack
            istack.push( in_Irec );
            HasIrecBeenAdded = true;
        }
    }

    public IrecStack GetIrecStack( final short in_intvltype )
    {
        IrecStack        cur_stack            = null;
        boolean          HasIstackBeenLocated = false;

        Enumeration stacks = super.elements();
        while ( stacks.hasMoreElements() ) {
            cur_stack = (IrecStack) stacks.nextElement();
            if ( cur_stack.GetIntvlType() == in_intvltype ) {
                super.removeElement( cur_stack );
                HasIstackBeenLocated = true;
                break;
            }
        }

        if ( HasIstackBeenLocated )
            return cur_stack;
        else
            return null;
    }

}
