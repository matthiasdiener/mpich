#if LOG 
#define MPE_LOG_SEND( to, tag, size ) \
  MPE_Log_send( to, tag, size )

#define MPE_LOG_RECEIVE( from, tag, size ) \
  MPE_Log_receive( from, tag, size )

#define MPE_LOG_EVENT( event, data, str ) \
  MPE_Log_event( event, data, str )

#define MPE_DESCRIBE_STATE( start, end, name , color ) \
  MPE_Describe_state( start, end, name , color )

#define MPE_DESCRIBE_EVENT( event, name ) \
  MPE_Describe_event( event, name )

#define MPE_INIT_LOG() \
  MPE_Init_log()

#define MPE_FINISH_LOG( file ) \
  MPE_Finish_log( file )

#else
#define MPE_LOG_SEND( to, tag, size ) {}
#define MPE_LOG_RECEIVE( from, tag, size ) {}
#define MPE_LOG_EVENT( event, data, str ) {}
#define MPE_DESCRIBE_STATE( start, end, name , color ) {}
#define MPE_DESCRIBE_EVENT( event, name ) {}
#define MPE_INIT_LOG() {}
#define MPE_FINISH_LOG( file ) {}
#endif
