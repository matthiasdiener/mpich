/* DEBUG          : Turn on Debugging and extra redundant checking code       */
#undef DEBUG

/* NOCPUID        : Don't write the interval record's CPU-ID field to the disk*/
#undef NOCPUID

/* NOINSTRUCTION  : Don't write the interval record's instr_addr to the disk  */
#undef NOINSTRUCTION

/* CHECKTIMEORDER : Turn on the increasing time order checking code           */
#undef CHECKTIMEORDER

/* CHECKRECDEFS   : check the Record Defineition Table for bebits consistency */
#undef CHECKRECDEFS

/* CHECKPROFILE   : check the Display Profile for bebits consistency          */
#undef CHECKPROFILE

/* When CHECKRECDEFS and CHECKPROFILE are both turned on, the Display Profile */
/* and the Record Definition Table will be checked against each other for     */
/* consistency                                                                */

/* NO_ARROW_STAT  : Exclude arrows in statistics collection                   */
#undef NO_ARROW_STAT

/* COMPRESSION    : Turn on various header compression code                   */
#undef COMPRESSION

/* GCC3XX_VA_ARG_FIX : Turn on the workaround for the va_arg in gcc-2.96/3.xx */
#undef GCC3XX_VA_ARG_FIX

/* Enable the large file support, i.e. > 2GB for 32-bit OS */
#undef _LARGEFILE64_SOURCE

/* Enable 64-bit file pointer support for 32-bit OS */
#undef _FILE_OFFSET_BITS
