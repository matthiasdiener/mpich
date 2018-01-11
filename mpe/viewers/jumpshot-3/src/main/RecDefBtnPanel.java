import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

public class RecDefBtnPanel extends JPanel
{
    public RecDefBtnPanel( RecDef def, JComponent listener,
                           boolean hasSelected )
    {
        super();

        setBorder( BorderFactory.createLoweredBevelBorder() );
        // setLayout( new FlowLayout( FlowLayout.LEFT ) );
        setLayout( new BoxLayout( this, BoxLayout.X_AXIS ) );

        def.checkbox = new JCheckBox();
        def.checkbox.setSelected( hasSelected );
        def.checkbox.setToolTipText( "Enable or disable " + def.description );
        // def.checkbox.setHorizontalAlignment( SwingConstants.RIGHT );
        def.checkbox.addItemListener( (ItemListener) listener );
        add( def.checkbox );

        JButton btn = new JButton( def.description );
        btn.setHorizontalAlignment( SwingConstants.LEFT );
        btn.setToolTipText( "Press " + def.description + " for histogram" );
        // btn.setBackground( def.color );
        btn.setIcon( new ColoredRect( def.color ) );
        btn.setBorder( BorderFactory.createRaisedBevelBorder() );
        btn.addActionListener( (ActionListener) listener );
        add( btn );

        /*
        System.out.println( def.description + "'s Size = " + getSize() + "\n"
                          + def.description + "'s PerferredSize = "
                                            + getPreferredSize()+ "\n"
                          + def.description + "'s MinimumSize = "
                                            + getMinimumSize() + "\n"
                          + def.description + "'s MaximumSize = "
                                            + getMaximumSize() + "\n");
        */
    }
}
