//Record just keeps together a RAW record and its associated header
class RECORD {
  CLOG_HEADER header;
  Object data;
  
  //Constructor
  public RECORD (CLOG_HEADER header, Object data) {
    this.header = header;
    this.data = data;
  }
}
