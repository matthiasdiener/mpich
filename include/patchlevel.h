#define PATCHLEVEL 1.2
#define PATCHLEVEL_MAJOR 1
#define PATCHLEVEL_MINOR 2
#define PATCHLEVEL_SUBMINOR 2
#define PATCHLEVEL_RELEASE_KIND ""
#ifndef PATCHLEVEL_RELEASE_DATE 
#ifdef RELEASE_DATE
#define PATCHLEVEL_RELEASE_DATE RELEASE_DATE
#else
#define PATCHLEVEL_RELEASE_DATE "$Date: 2001/09/25 13:07:11$"
#endif
#endif

#define PATCHES_APPLIED "\
"
