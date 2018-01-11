import java.io.*;
import java.util.*;


public class IrecGroupListEnumerator
{
    private IrecGroupList  igroups;
    private Enumeration    groups;
    private Enumeration    stacks    = null;
    private IrecGroup      cur_group = null;
    private IrecStack      cur_stack = null;

    private boolean        groups_hasMoreElements = false;
    private boolean        stacks_hasMoreElements = false;

    public IrecGroupListEnumerator( IrecGroupList in_igroups )
    {
        igroups = in_igroups;
        groups  = igroups.elements();
    }

    public boolean HasMoreIrecStacks()
    {
        if ( groups.hasMoreElements() )
            return true;
        else
            if ( stacks != null )
                if ( stacks.hasMoreElements() )
                    return true;
        
        return false;
    }

    public IrecStack NextIrecStack()
    {
        if ( stacks == null ) {
            if ( groups.hasMoreElements() ) {
                cur_group = ( IrecGroup ) groups.nextElement();
                stacks = cur_group.elements();
            }
            else
                throw new NoSuchElementException( 
                                "A: forget to call HasMoreIrecStacks() ?"
                                + "\n" + igroups );
        }
      
        if ( stacks.hasMoreElements() ) {
            cur_stack = ( IrecStack ) stacks.nextElement();
            if ( ! stacks.hasMoreElements() && groups.hasMoreElements() ) {
                 cur_group = ( IrecGroup ) groups.nextElement();
                 stacks = cur_group.elements();
            }
            return cur_stack;
        }

        throw new NoSuchElementException( 
                               "B: forget to call HasMoreIrecStacks() ?" + "\n"
                             + "igroups = " + igroups + "\n"
                             + "groups = " + groups + "\n"
                             + "stacks = " + stacks + "\n" );
    }
}
