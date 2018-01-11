/*
 * st_ibmds2.c
 *
 * Context loading using dseg trick on IBM
 * NOTE: This file CANNOT be linked in as shared. It must be bound
 * statically to each executable.
 */

static char *rcsid = "$Header: /nfs/globus1/src/master/nexus_source/st_ibmds2.c,v 1.10 1996/10/07 04:40:17 tuecke Exp $";

#include "internal.h"
#include <a.out.h>
#undef FREAD
#undef FWRITE
#include <ldfcn.h>
#include <sys/ldr.h>

#include "st_ibmds.h"


extern char _text[];
extern char _etext[];
extern char _data[];
extern char _edata[];
extern char _end[];

ContextInfo context_info;

extern void (*_nx_ds_context_init)(void);
extern void (*_nx_ds_context_destroy)(void);
extern void (*_nx_ds_startup_trick)(void);

extern nexus_bool_t  _nx_nexus_started;

extern int NexusBoot(void);
extern PFVSTR __cdtors[];

/* Check to see if ptr is located in the data segment of the
 * current process
 */
#define IsDataRef(ptr) \
  ((_data <= (char *) (ptr)) && ((char *) (ptr) < _end))

static print_context_info(ContextInfo * contextPtr,FILE *ostream) 
{
  nexus_printf("_text %x _etext %x _data %x _edata %x _end %x\n",
	 _text,_etext,_data,_edata,_end);

  fprintf(ostream,"File Name: %s\n",contextPtr->file_name);
  fprintf(ostream,"Data Segment: %x\n",contextPtr->data_segment);
  fprintf(ostream,"Data Segment Size: %d\n",contextPtr->dataseg_size);
  fprintf(ostream,"TOC:\n");
  fprintf(ostream,"  offset: %d\n",contextPtr->toc_offset);
  fprintf(ostream,"  size: %d\n",contextPtr->toc_size);
  fprintf(ostream,"Relocations: %d\n",contextPtr->relocation_cnt);
}

/* The first context loaded is designated the master
 * context.  This routine initializes the context_info
 * data structure from this process.  
 */
static ContextInfo * init_master_context(char * file) {
  struct aouthdr ahdr;
  LDFILE *ldPointer = NULL;
  SCNHDR data_scn, bss_scn, text_scn;
  long data_fptr;
  int i;

  /* I'm the master */

  nexus_debug_printf(2, ("init_master_context(): entering\n"));

  context_info.master_context = &context_info;
#ifdef NEXUS_CPP_HACK
  context_info.dtorstainted = 0;
#endif

  /* Save the name of the file */
  context_info.file_name = malloc(strlen(file) + 1);
  strcpy(context_info.file_name,file);

  nexus_debug_printf(2, ("opening executable file\n"));
  nexus_debug_printf(2, ("file:%s ldPointer:%x\n",file,ldPointer));

  /* Open up the executable file and get the header */
  if ((ldPointer = ldopen(file,ldPointer)) == NULL) { 
      nexus_printf("ldopen failed for %s\n",file);
    return;
  }

  nexus_debug_printf(2, ("seeking to optional header\n"));

  if (ldohseek(ldPointer) == FAILURE) {
      nexus_printf("ldohseek failed for %s\n",file);
/*    fprintf(stderr,"ldohseed failed for %s\n",file); */
    return;
  };

  nexus_debug_printf(2, ("Reading aouthdr\n"));

  FREAD((char *) &ahdr,sizeof(ahdr),1,ldPointer);

  /* Save information about the data segment and the 
   * Table of contents.  The table of contents is the
   * last thing in the initialized data segment, so we
   * compute the TOC size from the data segment size
   * and the toc_offset.  The loader seems to do some
   * alignment so we use _data and _edata to compute
   * the size of the table of contents.
   */
  context_info.data_segment = _data;
  context_info.dataseg_size = ahdr.bsize + ahdr.dsize;
  context_info.toc_offset = ahdr.o_toc;
  context_info.toc = _data + context_info.toc_offset;
  context_info.toc_size = ((_edata - _data) - ahdr.o_toc)/4;

  nexus_debug_printf(2, ("bsize:%x dsize:%x toc_offset:%x toc:%x\n",ahdr.bsize, ahdr.dsize, ahdr.o_toc, context_info.toc));

  /* Get header for data segment */
  if (ldshread(ldPointer,ahdr.o_sndata,&data_scn) == FAILURE) {
    fprintf(stderr,"ldshread failed on data segment\n");
    return;
  };

  nexus_debug_printf(2, ("s_nreloc:%x %d toc_size:%x %d\n",data_scn.s_nreloc,data_scn.s_nreloc,context_info.toc_size, context_info.toc_size));

  /* Save the location in the file of the location of initialized data. */ 
  context_info.dataFP = data_scn.s_scnptr;

  /* Allocate space to store offsets to data segment relocations
   * that are not part of the TOC.
   */
/*
 * This is completely fucked up...
 * first of all, because of padding and alignment, this does not run
 * all of the non-TOC relocations which causes some function descriptors
 * not to be relocated.  This is just peachy-keen when you try to
 * call those functions.
 * secondly, it relocates everything in the TOC blindly preventing
 * the use of static storage in the TOC when if it read the relocations
 * it could just run them and relocate only what is needed.
 * 
 context_info.relocation_cnt = data_scn.s_nreloc-context_info.toc_size;
 */

  context_info.relocation_cnt = data_scn.s_nreloc;
  context_info.relocations = malloc(context_info.relocation_cnt*sizeof(void *));

  /* Seek to relocation data for data segment */
  if (ldrseek(ldPointer,ahdr.o_sndata) == FAILURE) {
    fprintf(stderr,"ldrseek failed on data segment\n");
    return;
  };

  nexus_debug_printf(2, ("Reading relocation entries for dseg\n"));

  /* Save the offsets to the locations in the data segment
   * that need to be relocated that are not part of the
   * TOC
   */
  for (i = 0 ; i < context_info.relocation_cnt ; i++) {
    struct reloc reloc_entry;
    FREAD((char *) & reloc_entry, RELSZ,1,ldPointer);
    context_info.relocations[i] = reloc_entry.r_vaddr - data_scn.s_paddr;
  }

  /* Check to make sure that there are no relocations in other sections */ 
  if (ldshread(ldPointer,ahdr.o_snbss,&bss_scn) == FAILURE) {
    fprintf(stderr,"ldshread failed for bss segment\n");
    return;
  };
  if (bss_scn.s_nreloc) {
    fprintf(stderr,"ldshread failed for bss segment\n");
    return;
  }

  ldclose(ldPointer);
  /* Store entry point to loaded program */
/*  context_info.entry_point = _nx_user_context_entry; */
   
  return &context_info;
}

/* relocate_dseg scans a data segment and relocates
 * references reletive to the base of the data segment.
 * The relocation is performed by taking the relocated address
 * in the master context, subtract out the base of the master
 * data segment and add in the base of the new data segment.
 *
 * NB:
 *    THIS ROUTINE ASSUMES THAT THE TOC CONTAINS ONLY ADDRESSES.
 *    THIS WILL NOT BE TRUE IF COMPILER OR LOADER OPTIONS SPECIFY
 *    THE TOC BE USED TO STORE SCALAR DATA>
*/
static relocate_dseg(dest)
     char * dest;
{
  int i = 0;
  char ** srcPtr;
  char ** destPtr;

  /* Relocate non-TOC symbols.  Offsets are stored in the context_info */
  for (i = 0; i < context_info.relocation_cnt ; i++) {
    srcPtr = (char **) (_data + context_info.relocations[i]);
    destPtr = (char **) (dest + context_info.relocations[i]);
    if (IsDataRef(*srcPtr)) {
	*destPtr = *srcPtr - _data + dest;
    }
    else {
      *destPtr = *srcPtr;
    }
  }
/*
  srcPtr = (char **) (_data + context_info.toc_offset);
  destPtr = (char **) (dest + context_info.toc_offset);

  for ( i = 0; i < context_info.toc_size ; i++,srcPtr++,destPtr++) {
    if (IsDataRef(*srcPtr)) 
      *destPtr = *srcPtr - _data + dest;
    else 
      *destPtr = *srcPtr;
    nexus_debug_printf(2, ("T %d %x/%x %x-%x \n", i, srcPtr, destPtr, *srcPtr, *destPtr));

  }
  */
}

/* Load and initialize a data segment from the executable
 * file.
 */
static load_dseg(dseg )
char * dseg;
{
  struct aouthdr ahdr;
  LDFILE *ldPointer = NULL;
  if ((ldPointer = ldopen(context_info.file_name,ldPointer)) == NULL) {
    fprintf(stderr,"ldopen failed for %s\n",context_info.file_name);
    return;
  }

  /* Read in the initialized data segment */
  FSEEK(ldPointer, context_info.dataFP, BEGINNING);
  FREAD(dseg, (_edata - _data), 1, ldPointer);

  ldclose(ldPointer);
  
  /* Initialize the BSS */
  bzero(dseg + (int) (_edata - _data),_end - _edata);
}

static void validate_dseg(dseg) 
     char * dseg;
{
  int i = 0;
  char ** src_ptr = (char **) _data;
  char ** dest_ptr = (char **) dseg;

  for ( i = 0; i < context_info.dataseg_size/4 ; i++,src_ptr++,dest_ptr++) {
    if (*src_ptr != *dest_ptr) {
      if ((char **) (_data + context_info.toc_offset) <= src_ptr &&
	  src_ptr < ((char **) (_data + context_info.toc_offset)) + context_info.toc_size)
	printf("* Data segment differance %x/%x %x-%x\n",
	       src_ptr,dest_ptr,*src_ptr,*dest_ptr);
      else
	printf(" Data segment differance %x/%x %x-%x\n",
	       src_ptr,dest_ptr,*src_ptr,*dest_ptr);
    }
  }
}

static void * clone_dseg(void) {
  char * dseg = malloc(context_info.dataseg_size);
  ContextInfo * my_info = &context_info;
  ContextInfo * contextPtr;
  /* Fill in initialized data */
  load_dseg(dseg);
  /* Relocate global references */ 
  relocate_dseg(dseg);
   /* validate_dseg(dseg);  */

  /* Get and initialize the context_info structure in the
     new data segment */
  contextPtr = (ContextInfo *)
    (((char *) &context_info) - context_info.data_segment + dseg);

  *contextPtr = *my_info;
  contextPtr->data_segment = dseg;
  contextPtr->toc = dseg + my_info->toc_offset;
  return contextPtr;
}

int destroy_context(ContextInfo *context)
{
  int i;

  nexus_debug_printf(2, ("destroy_context(): entering\n"));

#ifdef NEXUS_CPP_HACK
  /* Run C++ destructors */

  if( context->master_context->dtorstainted==1 || 
      context->master_context != context ) {
      i=context->cdtorcount;
      while( i!=0 ) {
	  i--;
	  if(context->cdtorfuncs[i].term != NULL &&
	      context->cdtorfuncs[i].init != (PFV)-1)
	      (*(context->cdtorfuncs[i].term))();
      }
      context->master_context->dtorstainted=1;
  }
#endif

  context->master_context->context_count -= 1;

  /* we want to free the data segment if this isn't a master context
   * and then unload the module if the reference count drops to 0
   */
  if (context->master_context != context) {
      free(context->data_segment);
  }
  return context->master_context->context_count;
}

/* This is the entry point for dynamically loaded contexts */
ContextInfo * _nx_thd_context_entry(char * name) { 
  ContextInfo *contextPtr;

#ifdef NEXUS_CPP_HACK
  PFVSTR currcdtor;
  int i;
#endif

  nexus_debug_printf(1, ("_nx_thd_context_entry(): entry point executing\n"));

  /* If this is the first time the load() has been called on this context,
   * master_context == 0
   *
   */
  if (context_info.master_context) {
      nexus_debug_printf(2, ("_nx_thd_context_entry(): cloning dseg\n"));
      contextPtr = clone_dseg();
  }
  else {
      /*
       * If _nx_nexus_started == TRUE, then nexus_start was called
       * on this process, so this must be a request for a cloned
       * data segment, but since master_context hasn't been initialized,
       * we must do that first
       */
      nexus_debug_printf(2, ("_nx_thd_context_entry(): init_master_context\n"));
      contextPtr = init_master_context(name);
      if (_nx_nexus_started == NEXUS_TRUE) {
	  nexus_debug_printf(2, ("_nx_thd_context_entry(): cloning dseg\n"));
	  contextPtr = clone_dseg();
	  context_info.master_context->context_count += 1;
      }
  }
  context_info.master_context->context_count += 1;

  /* print_context_info(&context_info,stdout); */

  set_context(contextPtr);
  contextPtr->SwitchContext = NULL;
  contextPtr->NexusBoot = NexusBoot;
  contextPtr->killfunc = destroy_context;

  {
      int ii;
      nexus_debug_printf(2, ("destroy_context: %x:%x\n",(contextPtr->killfunc),destroy_context));
      for(ii=0; ii<5; ii++) {
	  nexus_debug_printf(2, ("%x\n", ((int *) (contextPtr->killfunc))[ii]));
      }

      nexus_debug_printf(2, ("NexusBoot: %x:%x\n",(contextPtr->NexusBoot),NexusBoot));
      for(ii=0; ii<5; ii++) {
	  nexus_debug_printf(2, ("%x\n", ((int *) (contextPtr->NexusBoot))[ii]));
      }
  }

#ifdef NEXUS_CPP_HACK

  /* Run through the list of constructors
   * we need to run through this list even if we aren't calling the functions
   * just in case we have to run the destructors by hand
   */

  nexus_debug_printf(2, ("Preparing to run constructors.\n"));

  contextPtr->cdtorfuncs = __cdtors;
  i=0;

  while( contextPtr->cdtorfuncs[i].init!=NULL || 
	 contextPtr->cdtorfuncs[i].term!=NULL ) {
      if(contextPtr->cdtorfuncs[i].init != NULL &&
	 contextPtr->cdtorfuncs[i].init != (PFV)-1 &&
	 contextPtr->master_context != contextPtr ) {
	  (*(contextPtr->cdtorfuncs[i].init))();
      }
      i++;
  }

  contextPtr->cdtorcount = i;
#endif

  nexus_debug_printf(2, ("_nx_thd_context_entry(): restoring master context\n"));

  set_context(contextPtr->master_context);

  nexus_debug_printf(2, ("_nx_thd_context_entry(): returning\n"));

  return contextPtr;
}

void set_context(ContextInfo *contextPtr) {
    
  settocbase_(contextPtr->toc);
}
