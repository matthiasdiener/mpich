import java.io.*;
import java.util.*;

//  IrecGroupList is a Vector of IrecGroup
public class IrecGroupList extends Vector
{
    /*  private data needed to create StateGroupLabel for Irec  */
    private SLOG_ThreadInfos  thread_infos;
    private int               view_idx;

    public IrecGroupList( final SLOG_ThreadInfos in_thread_infos,
                                int              in_view_idx )
    {
        super();
        thread_infos     = in_thread_infos;
        view_idx         = in_view_idx;
    }

    //  Because of AddIrec(), Init() seems redundant 
    public void Init()
    {
        SLOG_ThreadInfo          thread;
        IrecGroup                cur_group;
        StateGroupLabel          groupID;
        boolean                  HasGroupBeenAdded;
        int                      th_idx, seq_idx;

        groupID = new StateGroupLabel();

        int Nthreads = thread_infos.NumOfThreads();
        for ( th_idx = 0; th_idx < Nthreads; th_idx++ ) {

            thread = thread_infos.EntryAt( th_idx );
            groupID.SetIndexes( thread, view_idx );

            HasGroupBeenAdded = false;
            seq_idx           = -1;
            cur_group         = null;
            Enumeration groups = super.elements();
            while ( groups.hasMoreElements() ) {
                seq_idx++;
                cur_group = (IrecGroup) groups.nextElement();
                if ( cur_group.GetGroupLabel().IsGreaterThan( groupID ) ) {
                    IrecGroup group = new IrecGroup( groupID );
                    super.insertElementAt( group, seq_idx );
                    HasGroupBeenAdded = true;
                    break;  // exit the while loop
                }
                else if ( cur_group.GetGroupLabel().IsEqualTo( groupID ) ) {
                    HasGroupBeenAdded = true;
                    break;  // exit the while loop
                }
                else  // if ( cur_group.GetGroupLabel().IsLessThan( groupID ) )
                    { /* Continue */ }
            }   //  Endof  while ( groups.hasMoreElements() )
            if ( ! HasGroupBeenAdded ) {
                IrecGroup group = new IrecGroup( groupID );
                super.addElement( group );
                HasGroupBeenAdded = true;
            }

        }   //  for ( th_idx = 0; th_idx < Nthreads; th_idx++ )
    }

    /**
     *   Add the input StateInfo to a StateGroupList whose elements,
     *   StateGroups, are assumed to have their groupIDs arranged in the
     *   accending order of their values
     */
    public void AddIrec( final SLOG_Irec in_Irec )
    {
        IrecGroup        cur_group         = null;
        boolean          HasIrecBeenAdded  = false;
        int              idx               = -1;
        StateGroupLabel  in_groupID
        = new StateGroupLabel( in_Irec.fix.origID, thread_infos, view_idx );

        Enumeration groups = super.elements();
        while ( groups.hasMoreElements() ) {
            idx ++;
            cur_group = (IrecGroup) groups.nextElement();
            //  Assume the groups is sorted in accending order of groupID
            if ( cur_group.GetGroupLabel().IsGreaterThan( in_groupID ) ) {
                // Create the corresponding StateGroup with the given groupID
                IrecGroup group = new IrecGroup( in_groupID );
                super.insertElementAt( group, idx );   //  <--- Insert Elem --

                // Add the in_Irec to the appropriate IrecGroup
                group.AddIrec( in_Irec );
                HasIrecBeenAdded = true;
                break;
            }
            else if ( cur_group.GetGroupLabel().IsEqualTo( in_groupID ) ) {
                // Add the in_Irec to the appropriate StateGroup
                cur_group.AddIrec( in_Irec );
                HasIrecBeenAdded = true;
                break;
            }
            else  // if ( cur_group.GetGroupLabel().IsLessThan( in_groupID ) )
                {  /*  Continue  */  }
        }   //  Endof  while ( enum.hasMoreElements() )

        if ( ! HasIrecBeenAdded ) {
            // Create the corresponding StateGroup with the given groupID
            IrecGroup group = new IrecGroup( in_groupID );
            super.addElement( group );                 //  <--- Add Elem -----

            // Add the in_stateinfo to the appropriate StateGroup
            group.AddIrec( in_Irec );
            HasIrecBeenAdded = true;
        }
    }

    public IrecStack GetIrecStack( final SLOG_Irec_min Irec_min )
    {
        IrecGroup        cur_group             = null;
        IrecStack        cur_istack            = null;
        boolean          HasIstackBeenLocated  = false;

        StateGroupLabel  in_groupID
        = new StateGroupLabel( Irec_min.origID, thread_infos, view_idx );

        Enumeration groups = super.elements();
        while ( groups.hasMoreElements() && ! HasIstackBeenLocated ) {
            cur_group = (IrecGroup) groups.nextElement();
            if ( cur_group.GetGroupLabel().IsEqualTo( in_groupID ) ) {
                cur_istack = cur_group.GetIrecStack( Irec_min.intvltype );
                if ( cur_group.size() == 0 )
                    super.removeElement( cur_group );
                HasIstackBeenLocated = true;
            }
        }

        if ( HasIstackBeenLocated )
            return cur_istack;
        else
            return null;
    }

    public IrecGroupListEnumerator IrecStackElements()
    {
        return ( new IrecGroupListEnumerator( this ) );
    }

}
