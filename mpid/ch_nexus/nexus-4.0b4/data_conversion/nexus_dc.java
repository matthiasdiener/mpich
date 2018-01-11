public class nexus_dc
{
    private static final String rcsid = "$Header: /nfs/globus1/src/master/nexus_dc/nexus_dc.java,v 1.1 1996/08/15 22:42:03 geisler Exp $";
    /*
     * little endian, with unsigned
     *  8 bit byte
     *  8 bit char
     * 16 bit short
     * 32 bit integer
     * 32 bit long
     * 32 bit pointer
     * 32 bit IEEE float
     * 64 bit IEEE double
     */
    public static final int NEXUS_DC_FORMAT_32BIT_LE = 1;

    /*
     * big endian, with unsigned
     *  8 bit byte
     *  8 bit char
     * 16 bit short
     * 32 bit integer
     * 32 bit long
     * 32 bit pointer
     * 32 bit IEEE float
     * 64 bit IEEE double
     */
    public static final int NEXUS_DC_FORMAT_32BIT_BE = 2;

    /*
     * little endian, with unsigned
     *  8 bit byte
     *  8 bit char
     * 16 bit short
     * 32 bit integer
     * 64 bit long
     * 64 bit pointer
     * 32 bit IEEE float
     * 64 bit IEEE double
     */
    public static final int NEXUS_DC_FORMAT_64BIT_LE = 3;

    /*
     * big endian, with unsigned
     *  8 bit byte
     *  8 bit char
     * 16 bit short
     * 32 bit integer
     * 64 bit long
     * 64 bit pointer
     * 32 bit IEEE float
     * 64 bit IEEE double
     */
    public static final int NEXUS_DC_FORMAT_64BIT_BE = 4;

    /*
     * big endian, with unsigned?
     *  8 bit byte
     *  8 bit char
     * 64 bit short
     * 64 bit integer
     * 64 bit long
     * 64 bit pointer
     * 32 bit Cray float
     * 64 bit Cray double
     */
    public static final int NEXUS_DC_FORMAT_CRAYC90 = 5;

    /*
     * big endian, without unsigned
     *  8 bit byte
     * 16 bit char
     * 16 bit short
     * 32 bit integer
     * 64 bit long
     * no pointer
     * 32 bit IEEE float
     * 64 bit IEEE double
     */
    public static final int NEXUS_DC_FORMAT_JAVA = 6;

    /*
     * little endian, with unsigned
     *  8 bit byte
     *  8 bit char
     * 16 bit short
     * 32 bit int
     * 32 bit long
     * 32 bit pointer
     * 32 bit VAX float
     * 64 bit VAX double
     */
    /* currently NEXUS_DC_FORMAT_VAX behaves like NEXUS_DC_FORMAT_32BIT_LE */
    public static final int NEXUS_DC_FORMAT_VAX = 7;

    public static final int NEXUS_DC_FORMAT_UNKNOWN = 99;

    private void SwapByte(byte a, byte b)
    {
	a ^= b;
	b ^= a;
	a ^= b;
    }

    public int nexus_dc_sizeof_byte(int count)    { return count; }
    public int nexus_dc_sizeof_char(int count)    { return count * 2; }
    public int nexus_dc_sizeof_u_char(int count)  { return count * 2; }
    public int nexus_dc_sizeof_short(int count)   { return count * 2; }
    public int nexus_dc_sizeof_u_short(int count) { return count * 2; }
    public int nexus_dc_sizeof_int(int count)     { return count * 4; }
    public int nexus_dc_sizeof_u_int(int count)   { return count * 4; }
    public int nexus_dc_sizeof_long(int count)    { return count * 8; }
    public int nexus_dc_sizeof_u_long(int count)  { return count * 8; }
    public int nexus_dc_sizeof_float(int count)   { return count * 4; }
    public int nexus_dc_sizeof_double(int count)  { return count * 8; }

    public void nexus_dc_put_byte(byte[] buffer,
    			      int position,
			      byte[] data,
			      int count)
    {
	for (int i = 0; i < count; i++)
	{
	    buffer[position++] = data[i];
	}
    }

    public void nexus_dc_put_char(byte[] buffer,
			      int position,
			      char[] data,
			      int count)
    {
	for (int i = 0; i < count; i++)
	{
	    buffer[position++] = (byte)(((short)data[i]) >>> 8);
	    buffer[position++] = (byte)(((short)data[i]) & 0xff);
	}
    }

    public void nexus_dc_put_u_char(byte[] buffer,
				int position,
				char[] data,
				int count)
    {
	for (int i = 0; i < count; i++)
	{
	    buffer[position++] = (byte)(((short)data[i]) >>> 8);
	    buffer[position++] = (byte)(((short)data[i]) & 0xff);
	}
    }
    
    public void nexus_dc_put_short(byte[] buffer,
			       int position,
			       short[] data,
			       int count)
    {
	for (int i = 0; i < count; i++)
	{
	    buffer[position++] = (byte) (data[i] >>> 8);
	    buffer[position++] = (byte) (data[i] & 0xff);
	}
    }
    
    public void nexus_dc_put_u_short(byte[] buffer,
				 int position,
				 short[] data,
				 int count)
    {
	for (int i = 0; i < count; i++)
	{
	    buffer[position++] = (byte) (data[i] >>> 8);
	    buffer[position++] = (byte) (data[i] & 0xff);
	}
    }
    
    public void nexus_dc_put_int(byte[] buffer,
			     int position,
			     int[] data,
			     int count)
    {
	for (int i = 0; i < count; i++)
	{
	    buffer[position++] = (byte) (data[i] >>> 24);
	    buffer[position++] = (byte) ((data[i] >>> 16) & 0xff);
	    buffer[position++] = (byte) ((data[i] >>> 8) & 0xff);
	    buffer[position++] = (byte) (data[i] & 0xff);
	}
    }
    
    public void nexus_dc_put_u_int(byte[] buffer,
			       int position,
			       int[] data,
			       int count)
    {
	for (int i = 0; i < count; i++)
	{
	    buffer[position++] = (byte) (data[i] >>> 24);
	    buffer[position++] = (byte) ((data[i] >>> 16) & 0xff);
	    buffer[position++] = (byte) ((data[i] >>> 8) & 0xff);
	    buffer[position++] = (byte) (data[i] & 0xff);
	}
    }
    
    public void nexus_dc_put_long(byte[] buffer,
			      int position,
			      long[] data,
			      int count)
    {
	for (int i = 0; i < count; i++)
	{
	    buffer[position++] = (byte) (data[i] >>> 56);
	    buffer[position++] = (byte) ((data[i] >>> 48) & 0xff);
	    buffer[position++] = (byte) ((data[i] >>> 40) & 0xff);
	    buffer[position++] = (byte) ((data[i] >>> 32) & 0xff);
	    buffer[position++] = (byte) ((data[i] >>> 24) & 0xff);
	    buffer[position++] = (byte) ((data[i] >>> 16) & 0xff);
	    buffer[position++] = (byte) ((data[i] >>> 8) & 0xff);
	    buffer[position++] = (byte) (data[i] & 0xff);
	}
    }
    
    public void nexus_dc_put_u_long(byte[] buffer,
				int position,
				long[] data,
				int count)
    {
	for (int i = 0; i < count; i++)
	{
	    buffer[position++] = (byte) (data[i] >>> 56);
	    buffer[position++] = (byte) ((data[i] >>> 48) & 0xff);
	    buffer[position++] = (byte) ((data[i] >>> 40) & 0xff);
	    buffer[position++] = (byte) ((data[i] >>> 32) & 0xff);
	    buffer[position++] = (byte) ((data[i] >>> 24) & 0xff);
	    buffer[position++] = (byte) ((data[i] >>> 16) & 0xff);
	    buffer[position++] = (byte) ((data[i] >>> 8) & 0xff);
	    buffer[position++] = (byte) (data[i] & 0xff);
	}
    }
    
    public void nexus_dc_put_float(byte[] buffer,
			       int position,
			       float[] data,
			       int count)
    {
	int tmp;

	for (int i = 0; i < count; i++)
	{
	    tmp = Float.floatToIntBits(data[i]);

	    buffer[position++] = (byte) (tmp >>> 24);
	    buffer[position++] = (byte) ((tmp >>> 16) & 0xff);
	    buffer[position++] = (byte) ((tmp >>> 8) & 0xff);
	    buffer[position++] = (byte) (tmp & 0xff);
	}
    }
    
    public void nexus_dc_put_double(byte[] buffer,
				int position,
				double[] data,
				int count)
    {
	long tmp;

	for (int i = 0; i < count; i++)
	{
	    tmp = Double.doubleToLongBits(data[i]);

	    buffer[position++] = (byte) (tmp >>> 56);
	    buffer[position++] = (byte) ((tmp >>> 48) & 0xff);
	    buffer[position++] = (byte) ((tmp >>> 40) & 0xff);
	    buffer[position++] = (byte) ((tmp >>> 32) & 0xff);
	    buffer[position++] = (byte) ((tmp >>> 24) & 0xff);
	    buffer[position++] = (byte) ((tmp >>> 16) & 0xff);
	    buffer[position++] = (byte) ((tmp >>> 8) & 0xff);
	    buffer[position++] = (byte) (tmp & 0xff);
	}
    }
    
    public void nexus_dc_get_byte(byte[] buffer,
			      int position,
			      int format,
			      byte[] data,
			      int count)
    {
	for (int i = 0; i < count; i++)
	{
	    data[i] = buffer[i];
	}
    }
    
    public void nexus_dc_get_char(byte[] buffer,
			      int position,
			      int format,
			      char[] data,
			      int count)
    {
	if (format == NEXUS_DC_FORMAT_JAVA)
	{
	    short tmp;

	    for (int i = 0; i < count; i++)
	    {
		tmp = (short) (((short)buffer[position]) << 8
			| (((short)buffer[position + 1]) & 0xff));
		data[i] = (char)tmp;
		position += 2;
	    }
	}
	else
	{
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (char)buffer[position++];
	    }
	}
    }
    
    public void nexus_dc_get_u_char(byte[] buffer,
			        int position,
			        int format,
			        char[] data,
			        int count)
    {
	if (format == NEXUS_DC_FORMAT_JAVA)
	{
	    short tmp;

	    for (int i = 0; i < count; i++)
	    {
		tmp = (short) (((short)buffer[position]) << 8
			| (((short)buffer[position + 1]) & 0xff));
		data[i] = (char)tmp;
		position += 2;
	    }
	}
	else
	{
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (char)buffer[position++];
	    }
	}
    }
    
    public void nexus_dc_get_short(byte[] buffer,
			       int position,
			       int format,
			       short[] data,
			       int count)
    {
	switch (format)
	{
	  case NEXUS_DC_FORMAT_VAX:
	  case NEXUS_DC_FORMAT_32BIT_LE:
	  case NEXUS_DC_FORMAT_64BIT_LE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (short) (((short)buffer[position + 1]) << 8
			   | (((short)buffer[position]) & 0xff));
		
		position += 2;
	    }
	    break;
	  case NEXUS_DC_FORMAT_JAVA:
	  case NEXUS_DC_FORMAT_CRAYC90:
	  case NEXUS_DC_FORMAT_32BIT_BE:
	  case NEXUS_DC_FORMAT_64BIT_BE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (short) (((short)buffer[position]) << 8
			   | (((short)buffer[position + 1]) & 0xff));
		
		position += 2;
	    }
	    break;
	  case NEXUS_DC_FORMAT_UNKNOWN:
	    // dunno
	    break;
	  default:
	    // throw UnknownTypeError;
	}
    }
    
    public void nexus_dc_get_u_short(byte[] buffer,
			         int position,
			         int format,
			         short[] data,
			         int count)
    {
	switch (format)
	{
	  case NEXUS_DC_FORMAT_VAX:
	  case NEXUS_DC_FORMAT_32BIT_LE:
	  case NEXUS_DC_FORMAT_64BIT_LE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (short) (((short)buffer[position + 1]) << 8
			   | (((short)buffer[position]) & 0xff));
		
		position += 2;
	    }
	    break;
	  case NEXUS_DC_FORMAT_JAVA:
	  case NEXUS_DC_FORMAT_CRAYC90:
	  case NEXUS_DC_FORMAT_32BIT_BE:
	  case NEXUS_DC_FORMAT_64BIT_BE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (short) (((short)buffer[position]) << 8
			   | (((short)buffer[position + 1]) & 0xff));
		
		position += 2;
	    }
	    break;
	  case NEXUS_DC_FORMAT_UNKNOWN:
	    // dunno
	    break;
	  default:
	    // throw UnknownTypeError;
	}
    }
    
    public void nexus_dc_get_int(byte[] buffer,
			     int position,
			     int format,
			     int[] data,
			     int count)
    {
	switch (format)
	{
	  case NEXUS_DC_FORMAT_VAX:
	  case NEXUS_DC_FORMAT_32BIT_LE:
	  case NEXUS_DC_FORMAT_64BIT_LE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (int) (((int)buffer[position + 3]) << 24
			   | (((int)buffer[position + 2] << 16) & 0xff)
			   | (((int)buffer[position + 1] << 8) & 0xff)
			   | ((int)buffer[position] & 0xff));
		
		position += 4;
	    }
	    break;
	  case NEXUS_DC_FORMAT_JAVA:
	  case NEXUS_DC_FORMAT_CRAYC90:
	  case NEXUS_DC_FORMAT_32BIT_BE:
	  case NEXUS_DC_FORMAT_64BIT_BE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (int) (((int)buffer[position]) << 24
			   | (((int)buffer[position + 2] << 16) & 0xff)
			   | (((int)buffer[position + 3] << 8) & 0xff)
			   | ((int)buffer[position + 4] & 0xff));
		
		position += 4;
	    }
	    break;
	  case NEXUS_DC_FORMAT_UNKNOWN:
	    break;
	  default:
	    // throw UnknownTypeError
	}
    }
    
    public void nexus_dc_get_u_int(byte[] buffer,
			       int position,
			       int format,
			       int[] data,
			       int count)
    {
	switch (format)
	{
	  case NEXUS_DC_FORMAT_VAX:
	  case NEXUS_DC_FORMAT_32BIT_LE:
	  case NEXUS_DC_FORMAT_64BIT_LE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (int) (((int)buffer[position + 3]) << 24
			   | (((int)buffer[position + 2] << 16) & 0xff)
			   | (((int)buffer[position + 1] << 8) & 0xff)
			   | ((int)buffer[position] & 0xff));
		
		position += 4;
	    }
	    break;
	  case NEXUS_DC_FORMAT_JAVA:
	  case NEXUS_DC_FORMAT_CRAYC90:
	  case NEXUS_DC_FORMAT_32BIT_BE:
	  case NEXUS_DC_FORMAT_64BIT_BE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (int) (((int)buffer[position]) << 24
			   | (((int)buffer[position + 2] << 16) & 0xff)
			   | (((int)buffer[position + 3] << 8) & 0xff)
			   | ((int)buffer[position + 4] & 0xff));
		
		position += 4;
	    }
	    break;
	  case NEXUS_DC_FORMAT_UNKNOWN:
	    break;
	  default:
	    // throw UnknownTypeError
	}
    }
    
    public void nexus_dc_get_long(byte[] buffer,
			      int position,
			      int format,
			      long[] data,
			      int count)
    {
	switch (format)
	{
	  case NEXUS_DC_FORMAT_VAX:
	  case NEXUS_DC_FORMAT_32BIT_LE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (long) (((long)buffer[position + 3]) << 24
			   | (((long)buffer[position + 2] << 16) & 0xff)
			   | (((long)buffer[position + 1] << 8) & 0xff)
			   | ((long)buffer[position] & 0xff));
		
		position += 4;
	    }
	    break;
	  case NEXUS_DC_FORMAT_32BIT_BE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (long) (((long)buffer[position]) << 24
			   | (((long)buffer[position + 1] << 16) & 0xff)
			   | (((long)buffer[position + 2] << 8) & 0xff)
			   | ((long)buffer[position + 3] & 0xff));
		
		position += 4;
	    }
	    break;
	  case NEXUS_DC_FORMAT_64BIT_LE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (long) (((long)buffer[position + 7]) << 56
			   | (((long)buffer[position + 6] << 48) & 0xff)
			   | (((long)buffer[position + 5] << 40) & 0xff)
			   | (((long)buffer[position + 4] << 32) & 0xff)
			   | (((long)buffer[position + 3] << 24) & 0xff)
			   | (((long)buffer[position + 2] << 16) & 0xff)
			   | (((long)buffer[position + 1] << 8) & 0xff)
			   | ((long)buffer[position] & 0xff));
		
		position += 8;
	    }
	    break;
	  case NEXUS_DC_FORMAT_JAVA:
	  case NEXUS_DC_FORMAT_CRAYC90:
	  case NEXUS_DC_FORMAT_64BIT_BE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (long) (((long)buffer[position]) << 56
			   | (((long)buffer[position + 1] << 48) & 0xff)
			   | (((long)buffer[position + 2] << 40) & 0xff)
			   | (((long)buffer[position + 3] << 32) & 0xff)
			   | (((long)buffer[position + 4] << 24) & 0xff)
			   | (((long)buffer[position + 5] << 16) & 0xff)
			   | (((long)buffer[position + 6] << 8) & 0xff)
			   | ((long)buffer[position] & 0xff));
		
		position += 8;
	    }
	    break;
	  case NEXUS_DC_FORMAT_UNKNOWN:
	    break;
	  default:
	    // throw UnknownTypeError
	}
    }
    
    public void nexus_dc_get_u_long(byte[] buffer,
			        int position,
			        int format,
			        long[] data,
			        int count)
    {
	switch (format)
	{
	  case NEXUS_DC_FORMAT_VAX:
	  case NEXUS_DC_FORMAT_32BIT_LE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (long) (((long)buffer[position + 3]) << 24
			   | (((long)buffer[position + 2] << 16) & 0xff)
			   | (((long)buffer[position + 1] << 8) & 0xff)
			   | ((long)buffer[position] & 0xff));
		
		position += 4;
	    }
	    break;
	  case NEXUS_DC_FORMAT_32BIT_BE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (long) (((long)buffer[position]) << 24
			   | (((long)buffer[position + 1] << 16) & 0xff)
			   | (((long)buffer[position + 2] << 8) & 0xff)
			   | ((long)buffer[position + 3] & 0xff));
		
		position += 4;
	    }
	    break;
	  case NEXUS_DC_FORMAT_64BIT_LE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (long) (((long)buffer[position + 7]) << 56
			   | (((long)buffer[position + 6] << 48) & 0xff)
			   | (((long)buffer[position + 5] << 40) & 0xff)
			   | (((long)buffer[position + 4] << 32) & 0xff)
			   | (((long)buffer[position + 3] << 24) & 0xff)
			   | (((long)buffer[position + 2] << 16) & 0xff)
			   | (((long)buffer[position + 1] << 8) & 0xff)
			   | ((long)buffer[position] & 0xff));
		
		position += 8;
	    }
	    break;
	  case NEXUS_DC_FORMAT_JAVA:
	  case NEXUS_DC_FORMAT_CRAYC90:
	  case NEXUS_DC_FORMAT_64BIT_BE:
	    for (int i = 0; i < count; i++)
	    {
		data[i] = (long) (((long)buffer[position]) << 56
			   | (((long)buffer[position + 1] << 48) & 0xff)
			   | (((long)buffer[position + 2] << 40) & 0xff)
			   | (((long)buffer[position + 3] << 32) & 0xff)
			   | (((long)buffer[position + 4] << 24) & 0xff)
			   | (((long)buffer[position + 5] << 16) & 0xff)
			   | (((long)buffer[position + 6] << 8) & 0xff)
			   | ((long)buffer[position] & 0xff));
		
		position += 8;
	    }
	    break;
	  case NEXUS_DC_FORMAT_UNKNOWN:
	    break;
	  default:
	    // throw UnknownTypeError
	}
    }
    
    public void nexus_dc_get_float(byte[] buffer,
			       int position,
			       int format,
			       float[] data,
			       int count)
    {
	return;
    }
    
    public void nexus_dc_get_double(byte[] buffer,
			        int position,
			        int format,
			        double[] data,
			        int count)
    {
	return;
    }
    
    public void nexus_dc_check_lost_precision_byte(byte[] buffer,
					       int position,
					       int format,
					       int count,
					       int result)
    {
	result = -1;
    }

    public void nexus_dc_check_lost_precision_char(byte[] buffer,
					       int position,
					       int format,
					       int count,
					       int result)
    {
	result = -1;
    }

    public void nexus_dc_check_lost_precision_u_char(byte[] buffer,
					  	 int position,
					  	 int format,
					  	 int count,
					  	 int result)
    {
	result = -1;
    }

    public void nexus_dc_check_lost_precision_short(byte[] buffer,
					        int position,
					        int format,
					        int count,
					        int result)
    {
	return;
    }
    
    public void nexus_dc_check_lost_precision_u_short(byte[] buffer,
						  int position,
						  int format,
						  int count,
						  int result)
    {
	return;
    }
    
    public void nexus_dc_check_lost_precision_int(byte[] buffer,
					      int position,
					      int format,
					      int count,
					      int result)
    {
	return;
    }
    
    public void nexus_dc_check_lost_precision_u_int(byte[] buffer,
						int position,
						int format,
						int count,
						int result)
    {
	return;
    }
    
    public void nexus_dc_check_lost_precision_long(byte[] buffer,
					       int position,
					       int format,
					       int count,
					       int result)
    {
	result = -1;
    }
    
    public void nexus_dc_check_lost_precision_u_long(byte[] buffer,
						 int position,
						 int format,
						 int count,
						 int result)
    {
	result = -1;
    }
    
    public void nexus_dc_check_lost_precision_float(byte[] buffer,
						int position,
						int format,
						int count,
						int result)
    {
	return;
    }
    
    public void nexus_dc_check_lost_precision_double(byte[] buffer,
						 int position,
						 int format,
						 int count,
						 int result)
    {
	return;
    }
    
}
