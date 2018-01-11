import javax.swing.*;
import java.awt.*;

public class WarnDialog
{
    public WarnDialog( Component p, String txt )
    {
       JOptionPane.showMessageDialog( p, txt, "Warning",
                                      JOptionPane.WARNING_MESSAGE );
    }
}
