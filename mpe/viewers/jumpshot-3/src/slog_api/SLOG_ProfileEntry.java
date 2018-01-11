import java.io.*;
import java.util.*;

public class SLOG_ProfileEntry
{
    public short        intvltype;
    public SLOG_bebits  bebits;
    public String       classtype;
    public String       label;
    public String       color;
    public Vector       arg_labels;   // Vector of argument labels String

    // public boolean is_var_irec;
    // public short   Nassocs;
    // public short   Nargs;

    public SLOG_ProfileEntry( String line )
    throws NoSuchElementException, IndexOutOfBoundsException
    {
        short  N_arg_labels;
        int    Nargs;
        // String desc_str;
        StringTokenizer desc_strtoken;

        StringTokenizer token_strs = new StringTokenizer( line, "\t" );
        int count = token_strs.countTokens();
        if ( count >= 6 ) {
            intvltype    = Short.parseShort( token_strs.nextToken().trim() );
            bebits       = new SLOG_bebits();
            bebits.decode( Byte.parseByte( token_strs.nextToken().trim() ) );
            classtype    = token_strs.nextToken().trim();
            label        = token_strs.nextToken().trim();
            color        = token_strs.nextToken().trim();
            // is_var_irec  = token_strs.nextToken().equals( "true" );
            // Nassocs      = Short.parseShort( token_strs.nextToken() );
            // Nargs        = Short.parseShort( token_strs.nextToken() );
            N_arg_labels = Short.parseShort( token_strs.nextToken().trim() );
            switch( N_arg_labels ) {
            case 0 :
                break;
            case 1 :
                arg_labels = new Vector();
                Nargs = Integer.parseInt( token_strs.nextToken().trim() );
                for ( int ii = 0; ii < Nargs; ii++ ) {
                    // desc_str = new String( token_strs.nextToken().trim() );
                    // arg_labels.addElement( desc_str );
                    desc_strtoken = new StringTokenizer(
                                              token_strs.nextToken().trim(),
                                              "\0" );
                    arg_labels.addElement( desc_strtoken.nextToken().trim() );
                }
                break;
            default :
                throw new IndexOutOfBoundsException(
                          "Incorrect N_arg_labels = " + N_arg_labels );
            }
        }
        else {
            throw new IndexOutOfBoundsException(
                      "Incorrect number of tokens = "
                      + token_strs.countTokens() );
        }
    }

    public String toString()
    {
        StringBuffer representation = new StringBuffer( "[ " );
        representation.append( intvltype   + ", " );
        representation.append( bebits      + " : " );
        representation.append( classtype   + ", " );
        representation.append( label       + ", " );
        representation.append( color       + " " );
        // representation.append( is_var_irec + " " );
        // representation.append( Nassocs     + " " );
        // representation.append( Nargs       + " " );
        if ( arg_labels != null )
            representation.append( "; " + arg_labels + " " );
        representation.append( "]" );
        return representation.toString();
    }
}
