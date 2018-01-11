typedef  unsigned int  bswp_uint32_t;


#if ! defined( WORDS_BIGENDIAN )
void bswp_byteswap( const bswp_uint32_t   Nelem,
		    const bswp_uint32_t   elem_sz,
		    char           *bytes );
#endif

bswp_uint32_t bswp_fwrite( const void           *src,
                           const bswp_uint32_t   elem_sz,
                           const bswp_uint32_t   Nelem,
                                 FILE           *outfd );

bswp_uint32_t bswp_fread(       void           *dest,
                          const bswp_uint32_t   elem_sz,
                          const bswp_uint32_t   Nelem,
                                FILE           *infd );
