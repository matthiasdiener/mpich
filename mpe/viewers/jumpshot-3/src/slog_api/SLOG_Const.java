class SLOG_Const
{
    public static final byte   INVALID_byte   = Byte.MIN_VALUE;
    public static final short  INVALID_short  = Short.MIN_VALUE;
    public static final int    INVALID_int    = Integer.MIN_VALUE;
    public static final long   INVALID_long   = Long.MIN_VALUE;
    public static final int    NULL_int       = 0;
    public static final int    NULL_iaddr     = 0;
    public static final long   NULL_fptr      = 0;
    public static final float  INVALID_float  = Float.MIN_VALUE;
    public static final double INVALID_double = Double.MIN_VALUE;
    public static final int    TRUE           = 1;
    public static final int    FALSE          = 0;

    public static final SLOG_bebits  beg_bebits
                                     = new SLOG_bebits( (byte) 1, (byte) 0 );
    public static final SLOG_bebits  mid_bebits
                                     = new SLOG_bebits( (byte) 0, (byte) 0 );
    public static final SLOG_bebits  end_bebits
                                     = new SLOG_bebits( (byte) 0, (byte) 1 );
    public static final SLOG_bebits  ful_bebits
                                     = new SLOG_bebits( (byte) 1, (byte) 1 );
}
