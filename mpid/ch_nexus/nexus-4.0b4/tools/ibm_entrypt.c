#include <stdio.h>
#include <a.out.h>
#include <ldfcn.h>

int patch(char *);
int verbose=0;
int nowrite=0;

int main(int argc, char **argv)
{
    int i;
    int rc;
    char filename[30];

    filename[0]='\0';

    for( i=1; i<argc; i++ ) {
	if( (argv[i])[0]=='-' ) {
	    switch( (argv[i])[1] ) {
	    case 'v':
		verbose=1;
		break;
	    case 'n':
		nowrite=1;
		break;
	    default:
		printf("invalid option %s\n",argv[i]);
		return -1;
	    }
	} else {  /* assume filename */
	    strcpy(filename, argv[i]);
	}
    }

    if(*filename=='\0') {  /* get filename */
	printf("File to patch:");
	fflush(stdout);
	scanf("%s",filename);
    }

    if( verbose ) printf("\nPatching %s\n",filename);

    rc = patch(filename);

    return rc;
}

#define DWINDOWSIZE 10

int patch(char *filename)
{
    int i;
    FILE *rwfiledesc;

    LDFILE *ldPointer = NULL;

    struct aouthdr ahdr;

    SCNHDR loaderhead;
    LDHDR  ldhead;
    LDSYM  *ploadersyms;
    char   *loaderstrings;
    long   altentry=0;
    long   altentryoffset=0;
    long   targetoffset=0;

    SCNHDR datahead;
    long   ldatawindow[DWINDOWSIZE];

    if ((ldPointer = ldopen(filename,ldPointer)) == NULL ) {
	printf("ldopen failed\n");
	return -1;
    }

    if (ldohseek(ldPointer) == FAILURE) {
	printf("ldohseek failed\n");
	return -1;
    }

    FREAD((char *) &ahdr,sizeof(ahdr),1,ldPointer);

    if( verbose ) printf("entry pt: %x\n",ahdr.entry);

    /*
     * deal with the .loader section
     *
     */

    if(ldnshread(ldPointer,_LOADER,&loaderhead) == FAILURE) {
	printf("ldnshread of .loader section header failed\n");
    }

    if( verbose ) printf("RAWptr: %x\n",loaderhead.s_scnptr);

    FSEEK(ldPointer, loaderhead.s_scnptr, BEGINNING);

    FREAD((char *) &ldhead, LDHDRSZ,1,ldPointer);

    if( verbose ) {
	printf("***Loader Section***\n");
	printf("Version #:    %x\n",ldhead.l_version);
	printf("#SYMtableEnt: %x\n",ldhead.l_nsyms);
	printf("#RELOCent:    %x\n",ldhead.l_nreloc);
	printf("LENidSTR:     %x\n",ldhead.l_istlen);
	printf("#IMPfilID:    %x\n",ldhead.l_nimpid);
	printf("OFFidSTR:     %x\n",ldhead.l_impoff);
	printf("LENstrTBL:    %x\n",ldhead.l_stlen);
	printf("OFFstrTBL:    %x\n",ldhead.l_stoff);
    }

    ploadersyms = (LDSYM *)malloc(LDSYMSZ*ldhead.l_nsyms);

    FREAD((char *) ploadersyms, LDSYMSZ, ldhead.l_nsyms, ldPointer);

    if( ldhead.l_stlen ) {
	loaderstrings = (char *)malloc(ldhead.l_stlen);

	FSEEK(ldPointer, loaderhead.s_scnptr + ldhead.l_stoff, BEGINNING);
	FREAD((char *) loaderstrings, ldhead.l_stlen, 1, ldPointer);
    }
	

    for (i=0; i<ldhead.l_nsyms; i++) {
	if( !ploadersyms[i].l_zeroes ) {
	    if (!strcmp(&loaderstrings[ploadersyms[i].l_offset],
		       "_nx_thd_context_entry")) {
		altentry = ploadersyms[i].l_value;
	    }
	} else {
	    if (!strcmp(&ploadersyms[i].l_name, "_nx_thd_context_entry")) {
		altentry = ploadersyms[i].l_value;
	    }
	}
    }
    if( verbose ) printf("alternate entry point: %x\n",altentry);

    altentryoffset = altentry - ahdr.entry;

    if( verbose ) printf("alternate entry point offset: %x\n", altentryoffset);

    /*
     * now that we have the entry point and the alternate entry point
     * we can plant the offset between the two into an empty
     * function descriptor environment field
     *
     */

    /* 1. Seek into the data segment to the function descriptor 
     *    for the entry point
     * 2. Determine if entry point function descriptor is 2 or 3 words
     *    word[2] == NULL  ==> 3 words
     *    (word[1] == word[3]) && (word[4] == NULL) ==> 2 words
     *    note: all other function descriptors are 3 words
     * 3. Place offset between entry point and auxilary entry point
     *    into first NULL word
     */

    if(ldnshread(ldPointer,_DATA,&datahead) == FAILURE) {
	printf("ldnshread of .data section header failed\n");
    }

    FSEEK(ldPointer, datahead.s_scnptr + ahdr.entry, BEGINNING);
    FREAD((char *) ldatawindow, sizeof(long), DWINDOWSIZE, ldPointer);

    if( verbose )
	for( i=0; i<DWINDOWSIZE; i++ ) printf("%x\n",ldatawindow[i]);

    if( ldatawindow[2] == 0 ) {
	targetoffset = datahead.s_scnptr + ahdr.entry + 8;
	i=2;
    } else if( (ldatawindow[1] == ldatawindow[3]) &&
	       (ldatawindow[4] == 0) ) {
	targetoffset = datahead.s_scnptr + ahdr.entry + 16;
	i=4;
    } else {
	printf("error: can't determine structure of function descriptor for entry point\n");
	return -1;
    }

    ldclose(ldPointer);

    if( verbose ) {
	printf("overwriting %x at offset %x[%d] with %x\n", ldatawindow[i],
	       targetoffset, i, altentryoffset);
    }

    if( !nowrite ) {
	rwfiledesc = fopen(filename,"rb+");
	fseek(rwfiledesc, targetoffset, 0);
	fwrite((void *) &altentryoffset, sizeof(long), 1, rwfiledesc);
	fclose(rwfiledesc);
    }

    if( verbose ) printf("Patch complete for: %s\n",filename);

    return 0;
}
