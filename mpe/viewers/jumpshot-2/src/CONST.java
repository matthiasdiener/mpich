/**
 * This is a static class which stores all global constants.
 */
class CONST {
  /*  public static final int CLOG_BLOCK_SIZE = 1024;  */
  public static final int CLOG_BLOCK_SIZE = 65536;
  public static final int CLOG_ENDLOG   = -2;
  public static final int CLOG_ENDBLOCK = -1;
  public static final int CLOG_UNDEF    =  0;
  public static final int CLOG_RAWEVENT =  1;
  public static final int CLOG_MSGEVENT =  2;
  public static final int CLOG_COLLEVENT=  3;
  public static final int CLOG_COMMEVENT=  4;
  public static final int CLOG_EVENTDEF =  5;
  public static final int CLOG_STATEDEF =  6;
  public static final int CLOG_SRCLOC   =  7;
  public static final int CLOG_SHIFT    =  8;
  public static final int LOG_MESG_SEND = -101;
  public static final int LOG_MESG_RECV = -102;
  public static final int LOG_CONST_DEF = -201;
  public static final int TIMELINES = 0;
  public static final int MTN_RANGES = 1;
  public static final String DSTR [] = {"Time Lines", "Mountain Ranges"};
}
