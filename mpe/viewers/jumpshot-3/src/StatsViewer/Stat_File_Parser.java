import java.io.*;
import java.util.*;

public class Stat_File_Parser
{
    private RandomAccessFile file_handle;
    private String           x_labels[], y_labels[], title;
    private String           buffer_line = null;
    private String           field_delimiter = "\t";
    private char             table_delimiter = '\u000c';

    public Stat_File_Parser(String file_name)
    throws IOException
    {
	file_handle = new RandomAccessFile(file_name,"r");
    }
    
    private String readLine()
    throws IOException
    {
        if ( buffer_line != null )
            if ( buffer_line.charAt(0) != table_delimiter )
                buffer_line = file_handle.readLine();
            else
                buffer_line = buffer_line.trim();
        else
            buffer_line = file_handle.readLine();

        return buffer_line;
    }

    public Statistics_Data readGraphData()
    throws IOException
    {
        String input;
        int    line_number;
	int    num_x_values, return_value;

	Vector x_vals_grp   = new Vector();
        Vector y_label_grp  = new Vector();

	line_number = 0;
	try {
	    input = readLine();
	}
	catch(IOException e) {
	    System.out.println("File_Parser:30:" + e.getMessage());
	    throw e;
	}
	
	if(input == null) {
	    file_handle.close();
	    return null;
	}

	num_x_values = processTitleAndXLabels(input);

	if(num_x_values == 0) {
	    file_handle.close();
	    return null;
	}

	try {
	    line_number++;
	    input = readLine();
	}
	catch(IOException e) {
	    System.out.println("File_Parser: Unable to read line " + 
			       line_number + " from file.\n");
	    try {
		file_handle.close();
	    }
	    catch(IOException ee){}
	    throw e;
	}

	while(input != null) {
	    if(Character.isLetterOrDigit(input.charAt(0))) {
		try {
		    return_value = processData(input, y_label_grp, x_vals_grp);
		}
		catch(NumberFormatException e) {
		    System.out.println("FileParser: Data values not in" +
				       " correct format.\n");
		    try {
			file_handle.close();
		    }
		    catch(IOException ee){}
		    throw new IOException();
		}
		if(return_value != num_x_values){
		    System.out.println("FileParser: Inconsistent data. Line" +
				       " number " + line_number);
		    try {
			file_handle.close();
		    }
		    catch(IOException ee){}
		    throw new IOException();
		}
	
		try {
		    line_number++;
		    input = readLine();
		}
		catch(IOException e) {
		    System.out.println("File_Parser: Unable to read line " + 
				       line_number + " from file.\n");
		    try {
			file_handle.close();
		    }
		    catch(IOException ee){}
		    throw new IOException();
		} 
	    }
	    else break;
	}   //  Endof while( input != null )
	

        double  data[][];
        int     ii, jj;

        y_labels = new String[ y_label_grp.size() ];
        for ( jj = 0; jj < y_labels.length; jj++ )
            y_labels[ jj ] = (String) y_label_grp.elementAt( jj );

	if ( x_labels.length > 0 && y_labels.length > 0 ) {
	    data = new double[x_labels.length] [y_labels.length];
	
	    for (ii = 0; ii < data.length; ii++)
	        for (jj = 0; jj < (data[ii].length); jj++)
    	            data[ii][jj] = ((double[])x_vals_grp.elementAt(jj))[ii];

   	    return new Statistics_Data(title, data, y_labels, x_labels);
        }
        else
            return null;
    }

    private int processTitleAndXLabels(String input)
    {
        StringTokenizer token_strs = new StringTokenizer( input,
                                                          field_delimiter );
        int count = token_strs.countTokens();
        if ( count > 1 ) {
            title = token_strs.nextToken().trim();
            x_labels = new String[ count - 1 ];
            for ( int idx = 0; idx < count-1; idx++ )
                x_labels[ idx ] = token_strs.nextToken().trim();
            return count-1;
        }
        else
            return 0;
    }
    
    private int processData(String input, Vector labels, Vector x_vals_grp)
    {
        String x_val_str, label_str;

        StringTokenizer token_strs = new StringTokenizer( input, 
                                                          field_delimiter );
        int count = token_strs.countTokens();
        if ( count > 1 ) {
            label_str = token_strs.nextToken().trim();
            labels.addElement( label_str );
            double x_vals[] = new double[ count-1 ];
            for ( int idx = 0; idx < count-1; idx++ ) {
                x_val_str = token_strs.nextToken().trim();
                x_vals[ idx ] = Double.valueOf( x_val_str ).doubleValue();
            }
            x_vals_grp.addElement( x_vals );
            return count-1;
        }
        else
            return 0;
    }

}
