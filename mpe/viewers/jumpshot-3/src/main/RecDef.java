import java.util.Vector;
import java.awt.Color;
import javax.swing.*;

//This class represents a Record definition description
class RecDef
{
    short        intvltype;
    SLOG_bebits  bebits;

    String       classtype;
    String       description;  // string describing state
    Color        color;        // Color given to this state
    Vector       arg_labels;   // Vector of argument labels String

    Vector       stateVector;  // Vector of completed states (paired up events)

    // This checkbox determines whether states belonging to this state def.,
    // should be displayed or not.
    JCheckBox checkbox;

    public RecDef()
    {
        description = new String();
        color       = null;

        // stateVector.size() determines if JCheckbox = true 
        // in RecDefButtons.setupPanels()
        stateVector = new Vector( 0 );
    }

    public RecDef( SLOG_ProfileEntry entry )
    {
        intvltype   = entry.intvltype;
        bebits      = new SLOG_bebits( entry.bebits );

        classtype   = new String( entry.classtype );
        description = new String( entry.label + " " + entry.bebits );
        color       = ColorUtil.getColor( entry.color );
        if ( entry.arg_labels != null )
            arg_labels  = ( Vector ) entry.arg_labels.clone();
        else
            arg_labels  = null;
             

        // stateVector.size() determines if JCheckbox = true 
        // in RecDefButtons.setupPanels()
        stateVector = new Vector( 0 );
    }

    public String toString()
    {
        return ("RecDef=[ intvltype=" + intvltype
                       + ", bebits=" + bebits
                       + ", classtype=" + classtype
                       + ", color=" + color.toString()
                       + ", description=" + description
                       + ", arg_labels=" + arg_labels
                       + " ]");
    }
}
