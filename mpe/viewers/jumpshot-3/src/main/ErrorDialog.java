import javax.swing.*;
import java.awt.*;

public class ErrorDialog
{
    public ErrorDialog( Component p, String txt )
    {
       JOptionPane.showMessageDialog( p, txt, "Error",
                                      JOptionPane.ERROR_MESSAGE );
    }
}
