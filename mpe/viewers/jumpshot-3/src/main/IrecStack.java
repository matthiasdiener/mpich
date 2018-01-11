import java.util.Stack;

public class IrecStack extends Stack
{
    private short    intvltype;
    private double   earliestBegTime;
    private double   latestEndTime;

    public IrecStack()
    {
        super();
        intvltype        = -1;
        earliestBegTime  = Double.NaN;
        latestEndTime    = Double.NaN;
    }

    public IrecStack( short   in_intvltype )
    {
        super();
        intvltype        = in_intvltype;
        earliestBegTime  = Double.NaN;
        latestEndTime    = Double.NaN;
    }

    public short GetIntvlType()
    {
        return( intvltype );
    }
}
