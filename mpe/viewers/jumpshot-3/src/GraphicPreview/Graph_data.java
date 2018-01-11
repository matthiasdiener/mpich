import java.awt.*;
import java.util.*;

public class Graph_data
{
    static int label_appendage = 1;
    static Vector used_colors = new Vector();
    private final static String global_label = "label";
    private String label;
    private double values[];
    private boolean enabled;
    private Color data_color;

    

    public Graph_data(String str, double aray[], boolean enabler, Color c)
    {
	label = new String(str);
	enabled = enabler;
        data_color = c;
        while ( data_color.equals( Color.white ) )
            data_color = generateNewColor();
	used_colors.addElement(data_color);
        values = aray;
    }

    public Graph_data(String str, double aray[])
    {
	this(str, aray, true, Graph_data.generateNewColor());
    }
    
    public Graph_data(double aray[])
    {
	this(Graph_data.global_label+
	     Integer.toString(Graph_data.label_appendage++), aray, true,
	     Graph_data.generateNewColor());
    }

    public double getElementAt(int i)
    {
	if (i<values.length)
	    return values[i];
	else
	    return 0;
    }
    
    private static Color generateNewColor()
    {
	int red_value, green_value, blue_value;
	Color temp_color;
        int count = 0;

        do {
	    red_value   = (int) ( Math.random() * 256 );
 	    green_value = (int) ( Math.random() * 256 );
	    blue_value  = (int) ( Math.random() * 256 );

	    temp_color  = new Color(red_value,green_value,blue_value);

            count++;
	} while ( used_colors.contains(temp_color) && count < 1000 );

	return temp_color;
    }

    public Color getColor()
    {
	return data_color;
    }
    
    public String getLabel()
    {
	return new String(label);
    }

    public boolean isEnabled()
    {
	return enabled;
    }

    public void setEnabled(boolean enabler)
    {
	enabled = enabler;
    }

    public void changeColor(Color c)
    {
	data_color = new Color(c.getRed(),c.getGreen(),c.getBlue());
    }
    
    public int getLength()
    {
	return values.length;
    }
    public static void resetUsedColors()
    {
	used_colors = new Vector();
    }
}
