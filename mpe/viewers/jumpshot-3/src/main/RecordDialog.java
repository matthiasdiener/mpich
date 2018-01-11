import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;

public class RecordDialog extends JDialog
implements ActionListener
{
    private JButton            btn;
    private GridBagConstraints con;

    public RecordDialog( Frame f, StateInterval state )
    {
        super( f, "Rectangle Info" );

        StateInfo info      = state.info;
        RecDef    statedef  = info.stateDef;
   
        Init( statedef.classtype + " : \'" + statedef.description + "\'",
              info.begT, info.groupID, info.endT, info.groupID );
        SetArgs( statedef.arg_labels, info.args );
        SetInstrAddr( info.instr_addr1 );
        SetInstrAddr( info.instr_addr2 );
        Finalize();
    }

    public RecordDialog( Frame f, ArrowInfo arrow )
    {
        super( f, "Arrow Info" );

        RecDef    arrowdef  = arrow.arrowDef;

        Init( arrowdef.classtype + " : \'" + arrowdef.description + "\' ",
              arrow.begT, arrow.begGroupID, arrow.endT, arrow.endGroupID );
        SetArgs( arrowdef.arg_labels, arrow.args );
        Finalize();
    }

    private void Init( String description_str,
                       double beg_time, final StateGroupLabel beg_groupID,
                       double end_time, final StateGroupLabel end_groupID )
    {
        con = new GridBagConstraints();
        con.anchor = GridBagConstraints.SOUTHWEST;
        
        btn = new JButton();
        btn.addActionListener( this );
        btn.setLayout( new GridBagLayout() );
        btn.setToolTipText( "Click once to close this dialog" );
        btn.setCursor( new Cursor( Cursor.HAND_CURSOR ) );

        con.gridy = 0;
        btn.add( new JLabel( description_str ), con );
        con.gridy++;
        if ( beg_time > Double.NEGATIVE_INFINITY )
            btn.add( new JLabel( "Starts at : task ID = " + beg_groupID + " &  "
                               + "time = " + beg_time + " sec." ), con );
        else
            btn.add( new JLabel( "Starts at : task ID = " + beg_groupID + " &  "
                               + "time = -UNKNOWN" ), con );

        con.gridy++;
        if ( end_time < Double.POSITIVE_INFINITY )
            btn.add( new JLabel( "Ends   at : task ID = " + end_groupID + " &  "
                               + "time = " + end_time + " sec." ), con );
        else
            btn.add( new JLabel( "Ends   at : task ID = " + end_groupID + " &  "
                               + "time = +UNKNOWN" ), con );
    }



    private void SetArgs( final Vector arg_labels, final iarray args )
    {
        if ( arg_labels != null && args != null ) {

            String  str_label;
            String  cur_label;
            int     cur_value;

            Enumeration labels = arg_labels.elements();
            Enumeration values = args.elements();
            while ( labels.hasMoreElements() && values.hasMoreElements() ) {
                con.gridy++;
                cur_label = (String) labels.nextElement();
                cur_value = ( (Integer) values.nextElement() ).intValue();

                if ( cur_value != Integer.MIN_VALUE )
                    str_label = new String( cur_label + " = " + cur_value );
                else
                    str_label = new String( cur_label + " = UNKNOWN" );

                btn.add( new JLabel( str_label ), con );
            }
        }
    }

    private void SetInstrAddr( int  instr_addr )
    {
        if ( instr_addr != SLOG_Const.NULL_iaddr ) {
            String  str_label;

            con.gridy++;
            str_label = new String( "Instruction Address = "
                                  + Integer.toHexString( instr_addr ) );
            btn.add( new JLabel( str_label ), con );
        }
    }


    private void Finalize()
    {
        getContentPane().setLayout( new BorderLayout() );
        getContentPane().add( btn, BorderLayout.CENTER );

        addWindowListener (new WindowAdapter () {
            public void windowClosing( WindowEvent e )
            {
                dispose();
            }
        });

        pack();
        setSize( getMinimumSize() );
        setResizable( false );
    }

    public void actionPerformed( ActionEvent e )
    { 
        dispose();
    }
}
