
	.section	".text",#alloc,#execinstr
	.file	"shmempriv.c"

	.section	".data",#alloc,#write
	.global	MPID_shmem
	.align	4
	.global MPID_shmem
MPID_shmem:
	.word	0
	.type	MPID_shmem,#object
	.size	MPID_shmem,4
	.global	MPID_myid
	.global MPID_myid
MPID_myid:
	.word	-1
	.type	MPID_myid,#object
	.size	MPID_myid,4
	.global	MPID_numids
	.global MPID_numids
MPID_numids:
	.word	0
	.type	MPID_numids,#object
	.size	MPID_numids,4
	.global	MPID_local
	.global MPID_local
MPID_local:
	.word	0
	.type	MPID_local,#object
	.size	MPID_local,4
	.global	MPID_incoming
	.global MPID_incoming
MPID_incoming:
	.word	0
	.type	MPID_incoming,#object
	.size	MPID_incoming,4
	.align	4
to_free:
	.word	0
	.type	to_free,#object
	.size	to_free,4
	.align	4
.L644:
	.word	0
	.type	.L644,#object

	.section	".data1",#alloc,#write
	.align	4
.L519:
	.ascii	"-np\0"
	.align	4
.L523:
	.ascii	"Missing argument to -np for number of processes\n\0"
	.align	4
.L531:
	.ascii	"Invalid number of processes (%d) invalid\n\0"
	.align	4
.L532:
	.ascii	"MPI_GLOBMEMSIZE\0"
	.align	4
.L537:
	.ascii	"Could not allocate shared memory (%d bytes)!\n\0"
	.align	4
.L681:
	.ascii	"Could not get any shared memory for long message!\0"

	.section	".bss",#alloc,#write
	.local	FreePktsTail
	.common	FreePktsTail,128,4
	.local	FreePkts
	.common	FreePkts,128,4
	.local	MPID_pktflush
	.common	MPID_pktflush,4,4
	.common	MPID_lshmem,512,4

	.section	".text",#alloc,#execinstr
/* 000000	   0 */		.align	4
!
! SUBROUTINE MPID_GetIntParameter
!
! OFFSET    SOURCE LINE	LABEL	INSTRUCTION

                       	.global MPID_GetIntParameter
                       MPID_GetIntParameter:
/* 000000	     */		save	%sp,-96,%sp
/* 0x0004	     */		sethi	%hi(.L_mcount_counters),%o0
/* 0x0008	     */		call	_mcount,1	! Result = %g0
/* 0x000c	     */		add	%o0,%lo(.L_mcount_counters),%o0
! FILE shmempriv.c

!    1		      !/*
!    2		      !   This file contains routines that are private and unique to the ch_shmem
!    3		      !   implementation
!    4		      ! */
!    6		      !#include "mpid.h"
!    7		      !#include "mpiddev.h"
!    9		      !/* MPID_shmem is not volatile but its contents are */
!   10		      !MPID_SHMEM_globmem *MPID_shmem = 0;
!   11		      !/* LOCAL copy of some of MPID_shmem */
!   12		      !MPID_SHMEM_lglobmem MPID_lshmem;
!   13		      !int                 MPID_myid = -1;
!   14		      !int                 MPID_numids = 0;
!   15		      !MPID_PKT_T          *MPID_local = 0;
!   16		      !VOLATILE MPID_PKT_T **MPID_incoming = 0;
!   17		      !static int	    MPID_pktflush;
!   19		      !#if defined (MPI_cspp)
!   20		      !/* These are special fields for the Convex SPP NUMA */
!   21		      !unsigned int			procNode[MPID_MAX_PROCS];
!   22		      !unsigned int			numCPUs[MPID_MAX_NODES];
!   23		      !unsigned int			numNodes;
!   24		      !int				MPID_myNode;
!   25		      !int                 		masterid;
!   26		      !extern int			cnx_yield;
!   27		      !extern int			cnx_debug;
!   28		      !extern char			*cnx_exec;
!   29		      !#endif
!   31		      !void				MPID_SHMEM_lbarrier  ANSI_ARGS((void));
!   32		      !void                            MPID_SHMEM_FreeSetup ANSI_ARGS((void));
!   34		      !void MPID_SHMEM_FlushPkts ANSI_ARGS((void));
!   36		      !/*
!   37		      !   Get an integer from the environment; otherwise, return defval.
!   38		      ! */
!   39		      !int MPID_GetIntParameter( name, defval )
!   40		      !char *name;
!   41		      !int  defval;
!   42		      !{
!   43		      !    extern char *getenv();
!   44		      !    char *p = getenv( name );

/* 0x0010	  44 */		call	getenv,1	! Result = %o0
/* 0x0014	     */		or	%g0,%i0,%o0
/* 0x0018	     */		orcc	%g0,%o0,%g0

!   46		      !    if (p) 

/* 0x001c	  46 */		be	.L77000003
/* 0x0020	     */		nop
                       .L77000002:

!   47		      !	return atoi(p);

/* 0x0024	  47 */		call	atoi,1	! Result = %o0	! (tail call)
/* 0x0028	     */		restore	%g0,%o0,%o0
                       .L77000003:
/* 0x002c	     */		ret
/* 0x0030	     */		restore	%g0,%i1,%o0
/* 0x0034	   0 */		.type	MPID_GetIntParameter,2
/* 0x0034	     */		.size	MPID_GetIntParameter,(.-MPID_GetIntParameter)

	.section	".text",#alloc,#execinstr
/* 000000	   0 */		.align	4
!
! SUBROUTINE MPID_SHMEM_init
!
! OFFSET    SOURCE LINE	LABEL	INSTRUCTION

                       	.global MPID_SHMEM_init
                       MPID_SHMEM_init:
/* 000000	     */		save	%sp,-104,%sp
/* 0x0004	     */		sethi	%hi(.L_mcount_counters+4),%o0
/* 0x0008	     */		call	_mcount,1	! Result = %g0
/* 0x000c	     */		add	%o0,%lo(.L_mcount_counters+4),%o0

!   48		      !    return defval;
!   49		      !}
!   51		      !void MPID_SHMEM_init( argc, argv )
!   52		      !int  *argc;
!   53		      !char **argv;
!   54		      !{
!   55		      !    int numprocs, i;
!   56		      !    int cnt, j, pkts_per_proc;
!   57		      !    int memsize;
!   59		      !#if defined (MPI_cspp)
!   61		      !    extern char *getenv();
!   62		      !    char *envVarBuf;
!   63		      !    unsigned int mmapRound;
!   64		      !    cnx_node_t myNode;
!   65		      !    unsigned int totalCPUs;
!   66		      !    unsigned int curNode, numCurNode;
!   68		      !    MPID_SHMEM_setflags();
!   70		      !    MPID_SHMEM_getSCTopology(&myNode, &numNodes, &totalCPUs, numCPUs);
!   71		      !    if (cnx_debug) {
!   72		      !	printf("CNXDB: %d nodes, %d CPUs\n", numNodes, totalCPUs);
!   73		      !	printf("CNXDB: root node = %d\n", myNode);
!   74		      !	for (i = 0; i < numNodes; ++i) {
!   75		      !	    printf("CNXDB: node %d -> %d CPUs\n", i, numCPUs[i]);
!   76		      !	}
!   77		      !    }
!   79		      !#endif
!   81		      !/* Make one process the default */
!   83		      !    numprocs = 1;
!   84		      !    for (i=1; i<*argc; i++) {

/* 0x0010	  84 */		ld	[%i0],%o0 ! volatile
/* 0x0014	  54 */		or	%g0,%i0,%i5
/* 0x0018	     */		or	%g0,%i1,%i4
/* 0x001c	  83 */		or	%g0,1,%i2
/* 0x0020	  84 */		cmp	%o0,1
/* 0x0024	     */		ble	.L77000014
/* 0x0028	     */		or	%g0,1,%i3
                       .L77000041:
/* 0x002c	     */		sethi	%hi(.L519),%o0
/* 0x0030	     */		add	%i1,4,%i1
/* 0x0034	     */		add	%o0,%lo(.L519),%i0

!   85		      !	if (strcmp( argv[i], "-np" ) == 0) {

/* 0x0038	  85 */		ld	[%i1],%o0 ! volatile
                       .L900000236:
/* 0x003c	  85 */		call	strcmp,2	! Result = %o0
/* 0x0040	     */		or	%g0,%i0,%o1
/* 0x0044	     */		cmp	%o0,0
/* 0x0048	     */		bne	.L900000238
/* 0x004c	     */		add	%i1,4,%i1
                       .L77000009:

!   86		      !	    /* Need to remove both args and check for missing value for -np */
!   87		      !	    if (i + 1 == *argc) {

/* 0x0050	  87 */		ld	[%i5],%o0 ! volatile
/* 0x0054	     */		add	%i3,1,%o1
/* 0x0058	     */		cmp	%o1,%o0
/* 0x005c	     */		be	.L77000010
/* 0x0060	     */		sll	%i3,2,%l0
                       .L900000231:

!   88		      !		fprintf( stderr, 
!   89		      !			 "Missing argument to -np for number of processes\n" );
!   90		      !		exit( 1 );
!   91		      !	    }
!   92		      !	    numprocs = atoi( argv[i+1] );

/* 0x0064	  92 */		add	%i4,%l0,%i0
/* 0x0068	     */		add	%i0,4,%o0
/* 0x006c	     */		call	atoi,1	! Result = %o0
/* 0x0070	     */		ld	[%o0],%o0 ! volatile
/* 0x0074	     */		or	%g0,%o0,%i2

!   93		      !	    argv[i] = 0;

/* 0x0078	  93 */		st	%g0,[%l0+%i4] ! volatile

!   94		      !	    argv[i+1] = 0;
!   95		      !	    MPID_ArgSqueeze( argc, argv );

/* 0x007c	  95 */		or	%g0,%i5,%o0
/* 0x0080	     */		or	%g0,%i4,%o1
/* 0x0084	     */		call	MPID_ArgSqueeze,2	! Result = %g0
/* 0x0088	     */		st	%g0,[%i0+4] ! volatile

!   96		      !	    break;

/* 0x008c	  96 */		ba	.L900000237
/* 0x0090	     */		cmp	%i2,0
                       .L900000238:

!   97		      !	}
!   98		      !    }

/* 0x0094	  98 */		ld	[%i5],%o0 ! volatile
/* 0x0098	     */		add	%i3,1,%i3
/* 0x009c	     */		cmp	%i3,%o0
/* 0x00a0	     */		bl,a	.L900000236
/* 0x00a4	     */		ld	[%i1],%o0 ! volatile
                       .L77000014:

!  100		      !#if defined (MPI_cspp)
!  102		      !    envVarBuf = getenv("MPI_TOPOLOGY");
!  103		      !    MPID_SHMEM_processTopologyInfo(envVarBuf, myNode,
!  104		      !				   &numprocs, numNodes, numCPUs, 1);
!  106		      !    if (numprocs == 0) {
!  107		      !	fprintf(stderr, "no processes specified\n");
!  108		      !	exit(1);
!  109		      !    }
!  111		      !/* The environment variable MPI_GLOBMEMSIZE may be used to select memsize */
!  112		      !    memsize = MPID_GetIntParameter( "MPI_GLOBMEMSIZE", MPID_MAX_SHMEM );
!  114		      !    if (memsize < (sizeof(MPID_SHMEM_globmem) + numprocs * 65536))
!  115		      !	memsize = sizeof(MPID_SHMEM_globmem) + numprocs * 65536;
!  116		      !    mmapRound = sysconf(_SC_PAGE_SIZE) * numNodes;
!  117		      !    memsize = ((memsize + mmapRound - 1) / mmapRound) * mmapRound;
!  119		      !#else
!  121		      !    if (numprocs <= 0 || numprocs > MPID_MAX_PROCS) {

/* 0x00a8	 121 */		cmp	%i2,0
                       .L900000237:
/* 0x00ac	 121 */		ble	.L77000017
/* 0x00b0	     */		cmp	%i2,32
                       .L77000015:
/* 0x00b4	     */		bg	.L77000017
/* 0x00b8	     */		sethi	%hi(.L532),%o0
                       .L900000232:

!  122		      !	fprintf( stderr, "Invalid number of processes (%d) invalid\n", numprocs );
!  123		      !	exit( 1 );
!  124		      !    }
!  126		      !/* The environment variable MPI_GLOBMEMSIZE may be used to select memsize */
!  127		      !    memsize = MPID_GetIntParameter( "MPI_GLOBMEMSIZE", MPID_MAX_SHMEM );

/* 0x00bc	 127 */		add	%o0,%lo(.L532),%o0
/* 0x00c0	     */		call	MPID_GetIntParameter,2	! Result = %o0
/* 0x00c4	     */		sethi	%hi(0x400000),%o1

!  129		      !    if (memsize < sizeof(MPID_SHMEM_globmem) + numprocs * 128)

/* 0x00c8	 129 */		sethi	%hi(0x23000),%i1
/* 0x00cc	     */		sll	%i2,7,%o1
/* 0x00d0	     */		add	%i1,560,%i3
/* 0x00d4	     */		add	%o1,%i3,%o1
/* 0x00d8	 127 */		or	%g0,%o0,%o2
/* 0x00dc	 129 */		cmp	%o0,%o1
/* 0x00e0	     */		bcc	.L77000020
/* 0x00e4	     */		or	%g0,%i2,%o0
                       .L77000019:

!  130		      !	memsize = sizeof(MPID_SHMEM_globmem) + numprocs * 128;

/* 0x00e8	 130 */		or	%g0,%o1,%o2
                       .L77000020:

!  132		      !#endif
!  134		      !    p2p_init( numprocs, memsize );

/* 0x00ec	 134 */		call	p2p_init,2	! Result = %g0
/* 0x00f0	     */		or	%g0,%o2,%o1

!  136		      !    MPID_shmem = p2p_shmalloc(sizeof(MPID_SHMEM_globmem));

/* 0x00f4	 136 */		call	p2p_shmalloc,1	! Result = %o0
/* 0x00f8	     */		or	%g0,%i3,%o0
/* 0x00fc	     */		sethi	%hi(MPID_shmem),%i0
/* 0x0100	     */		st	%o0,[%i0+%lo(MPID_shmem)] ! volatile

!  138		      !    if (!MPID_shmem) {

/* 0x0104	 138 */		ld	[%i0+%lo(MPID_shmem)],%o1 ! volatile
/* 0x0108	     */		cmp	%o1,0
/* 0x010c	     */		be	.L77000021
/* 0x0110	     */		or	%g0,1,%o0
                       .L900000233:

!  139		      !	fprintf( stderr, "Could not allocate shared memory (%d bytes)!\n",
!  140		      !		 sizeof( MPID_SHMEM_globmem ) );
!  141		      !	exit(1);
!  142		      !    }
!  144		      !/* Initialize the shared memory */
!  146		      !    MPID_shmem->barrier.phase = 1;

/* 0x0114	 146 */		ld	[%i0+%lo(MPID_shmem)],%o1 ! volatile
/* 0x0118	     */		add	%i1,544,%o2

!  147		      !    MPID_shmem->barrier.cnt1  = numprocs;
!  148		      !    MPID_shmem->barrier.cnt2  = 0;
!  149		      !    MPID_shmem->barrier.size  = numprocs;
!  151		      !    p2p_lock_init( &MPID_shmem->globlock );
!  152		      !    cnt	      = 0;    /* allocated packets */

/* 0x011c	 152 */		or	%g0,0,%l4
/* 0x0120	 146 */		st	%o0,[%o1+%o2] ! volatile
/* 0x0124	 147 */		add	%i1,548,%o1
/* 0x0128	     */		ld	[%i0+%lo(MPID_shmem)],%o0 ! volatile
/* 0x012c	     */		st	%i2,[%o0+%o1] ! volatile
/* 0x0130	 148 */		add	%i1,552,%o1
/* 0x0134	     */		ld	[%i0+%lo(MPID_shmem)],%o0 ! volatile
/* 0x0138	     */		st	%g0,[%o0+%o1] ! volatile
/* 0x013c	 149 */		add	%i1,540,%o1
/* 0x0140	     */		ld	[%i0+%lo(MPID_shmem)],%o0 ! volatile
/* 0x0144	     */		st	%i2,[%o0+%o1] ! volatile
/* 0x0148	 151 */		or	%g0,1,%o1
/* 0x014c	     */		ld	[%i0+%lo(MPID_shmem)],%o0 ! volatile
/* 0x0150	     */		add	%o0,1536,%o0
/* 0x0154	     */		call	mutex_init,3	! Result = %g0
/* 0x0158	     */		or	%g0,0,%o2

!  154		      !#if defined (MPI_cspp)
!  155		      !    for (i = j = 0; i < numNodes; ++i) {
!  156		      !	MPID_shmem->globid[i] = j;
!  157		      !	j += numCPUs[i];
!  158		      !	p2p_lock_init(&(MPID_shmem->globid_lock[i]));
!  159		      !    }
!  160		      !#else
!  161		      !    MPID_shmem->globid = 0;

/* 0x015c	 161 */		ld	[%i0+%lo(MPID_shmem)],%o0 ! volatile
/* 0x0160	     */		add	%i1,536,%o1

!  162		      !#endif
!  164		      !/* The following is rough if numprocs doesn't divide the MAX_PKTS */
!  165		      !    pkts_per_proc = MPID_SHMEM_MAX_PKTS / numprocs;
!  167		      !    pkts_per_proc = 4;
!  168		      !/*
!  169		      ! * Determine the packet flush count at runtime.
!  170		      ! * (delay the harsh reality of resource management :-) )
!  171		      ! */
!  172		      !    MPID_pktflush = (pkts_per_proc > numprocs) ? pkts_per_proc / numprocs : 1;

/* 0x0164	 172 */		cmp	%i2,4
/* 0x0168	 161 */		st	%g0,[%o0+%o1] ! volatile
/* 0x016c	 172 */		bge	.L77000024
/* 0x0170	     */		or	%g0,0,%l6
                       .L77000023:
/* 0x0174	     */		or	%g0,4,%o0
/* 0x0178	     */		call	.div,2	! Result = %o0
/* 0x017c	     */		or	%g0,%i2,%o1
/* 0x0180	     */		ba	.L77000025
/* 0x0184	     */		or	%g0,%o0,%o1
                       .L77000024:
/* 0x0188	     */		or	%g0,1,%o1
                       .L77000025:
/* 0x018c	     */		sethi	%hi(MPID_pktflush),%o0

!  174		      !#if defined (MPI_cspp)
!  175		      !    if (cnx_debug) printf("CNXDB: packet flush count = %d\n", MPID_pktflush);
!  176		      !#endif
!  178		      !    for (i=0; i<numprocs; i++) {

/* 0x0190	 178 */		cmp	%i2,0
/* 0x0194	     */		ble	.L77000033
/* 0x0198	     */		st	%o1,[%o0+%lo(MPID_pktflush)] ! volatile
                       .L77000039:
/* 0x019c	     */		sethi	%hi(MPID_lshmem),%o0
/* 0x01a0	     */		add	%o0,%lo(MPID_lshmem),%i1
/* 0x01a4	     */		sethi	%hi(MPID_lshmem+128),%o0
/* 0x01a8	     */		add	%o0,%lo(MPID_lshmem+128),%o0
/* 0x01ac	     */		st	%o0,[%sp+92]
/* 0x01b0	     */		sethi	%hi(MPID_lshmem+256),%o0
/* 0x01b4	     */		add	%o0,%lo(MPID_lshmem+256),%o0
/* 0x01b8	     */		st	%o0,[%sp+96]
/* 0x01bc	     */		sethi	%hi(MPID_lshmem+384),%o0
/* 0x01c0	     */		add	%o0,%lo(MPID_lshmem+384),%o0
/* 0x01c4	     */		st	%o0,[%sp+100]
/* 0x01c8	     */		sethi	%hi(0x1400),%o0
/* 0x01cc	     */		add	%o0,536,%i3
/* 0x01d0	     */		sethi	%hi(0x2400),%o0
/* 0x01d4	     */		add	%o0,536,%l7
/* 0x01d8	     */		sethi	%hi(0x2800),%o0
/* 0x01dc	     */		add	%o0,560,%l5
/* 0x01e0	     */		sethi	%hi(0x1000),%o0
/* 0x01e4	     */		add	%o0,96,%i5
/* 0x01e8	     */		add	%i0,%lo(MPID_shmem),%i0
/* 0x01ec	     */		or	%g0,0,%l0
/* 0x01f0	     */		or	%g0,0,%l2
/* 0x01f4	     */		or	%g0,0,%l1
/* 0x01f8	     */		or	%g0,0,%l3
/* 0x01fc	     */		or	%g0,%i5,%i4

!  179		      !	/* setup the local copy of the addresses of objects in MPID_shmem */
!  180		      !	MPID_lshmem.availlockPtr[i]	   = &MPID_shmem->availlock[i];

/* 0x0200	 180 */		ld	[%i0],%o0 ! volatile
                       .L900000234:
/* 0x0204	 180 */		add	%o0,%l1,%o0
/* 0x0208	     */		st	%o0,[%l2+%i1] ! volatile

!  181		      !	MPID_lshmem.incominglockPtr[i] = &MPID_shmem->incominglock[i];

/* 0x020c	 181 */		ld	[%i0],%o0 ! volatile

!  182		      !	MPID_lshmem.incomingPtr[i]	   = &MPID_shmem->incoming[i];
!  183		      !	MPID_lshmem.availPtr[i]	   = &MPID_shmem->avail[i];
!  185		      !	/* Initialize the shared memory data structures */
!  186		      !	MPID_shmem->incoming[i].head     = 0;
!  187		      !	MPID_shmem->incoming[i].tail     = 0;
!  189		      !    /* Setup the avail list of packets */
!  190		      !	MPID_shmem->avail[i].head = MPID_shmem->pool + cnt;
!  191		      !	for (j=0; j<pkts_per_proc; j++) {

/* 0x0210	 191 */		or	%g0,0,%o3
/* 0x0214	 181 */		ld	[%sp+92],%o1
/* 0x0218	     */		add	%o0,%l1,%o0
/* 0x021c	     */		add	%o0,768,%o0

!  192		      !	    MPID_shmem->pool[cnt+j].head.next = 
!  193		      !		((MPID_PKT_T *)MPID_shmem->pool) + cnt + j + 1;

/* 0x0220	 193 */		or	%g0,0,%o5
/* 0x0224	 181 */		st	%o0,[%l2+%o1] ! volatile
/* 0x0228	 193 */		add	%l4,4,%o2
/* 0x022c	 182 */		ld	[%i0],%o0 ! volatile
/* 0x0230	     */		ld	[%sp+96],%o1
/* 0x0234	     */		add	%o0,%l3,%o0
/* 0x0238	     */		add	%o0,1560,%o0
/* 0x023c	     */		st	%o0,[%l2+%o1] ! volatile
/* 0x0240	 183 */		ld	[%i0],%o0 ! volatile
/* 0x0244	     */		ld	[%sp+100],%o1
/* 0x0248	     */		add	%o0,%l3,%o0
/* 0x024c	     */		add	%o0,%i3,%o0
/* 0x0250	     */		st	%o0,[%l2+%o1] ! volatile
/* 0x0254	 186 */		ld	[%i0],%o0 ! volatile
/* 0x0258	     */		add	%o0,%l3,%o0
/* 0x025c	     */		st	%g0,[%o0+1560] ! volatile
/* 0x0260	 187 */		ld	[%i0],%o0 ! volatile
/* 0x0264	     */		add	%o0,%l3,%o0
/* 0x0268	     */		st	%g0,[%o0+1564] ! volatile
/* 0x026c	 190 */		ld	[%i0],%o0 ! volatile
/* 0x0270	     */		ld	[%i0],%o1 ! volatile
/* 0x0274	     */		add	%o0,%l0,%o0
/* 0x0278	     */		add	%o0,%l7,%o0
/* 0x027c	     */		add	%o1,%l3,%o1
/* 0x0280	     */		st	%o0,[%i3+%o1] ! volatile
/* 0x0284	 193 */		sll	%l4,5,%o0
/* 0x0288	     */		add	%o0,%l4,%o0
/* 0x028c	     */		sll	%o0,2,%o0
/* 0x0290	     */		sub	%o0,%l4,%o0
/* 0x0294	     */		sll	%o0,3,%o4
                       .L77000029:
/* 0x0298	     */		ld	[%i0],%o0 ! volatile
                       .L900000235:
/* 0x029c	     */		ld	[%i0],%o1 ! volatile
/* 0x02a0	     */		add	%o0,%o5,%o0
/* 0x02a4	     */		add	%o1,%o4,%o1
/* 0x02a8	     */		add	%o0,%l5,%o0
/* 0x02ac	     */		add	%o1,%l7,%o1

!  194		      !/*	    MPID_shmem->pool[cnt+j].head.src = i; */
!  195		      !	    MPID_shmem->pool[cnt+j].head.owner = i;
!  196		      !	}

/* 0x02b0	 196 */		add	%o3,1,%o3
/* 0x02b4	 193 */		st	%o0,[%o1+4] ! volatile
/* 0x02b8	 196 */		add	%o5,1048,%o5
/* 0x02bc	 195 */		ld	[%i0],%o0 ! volatile
/* 0x02c0	 196 */		cmp	%o3,4
/* 0x02c4	 195 */		add	%o0,%o4,%o0
/* 0x02c8	     */		add	%o0,%l7,%o0
/* 0x02cc	 196 */		add	%o4,1048,%o4
/* 0x02d0	 195 */		st	%l6,[%o0+8] ! volatile
/* 0x02d4	 196 */		bl,a	.L900000235
/* 0x02d8	     */		ld	[%i0],%o0 ! volatile
                       .L77000031:

!  197		      !	/* Clear the last "next" pointer */
!  198		      !	MPID_shmem->pool[cnt+pkts_per_proc-1].head.next = 0;

/* 0x02dc	 198 */		ld	[%i0],%o0 ! volatile

!  199		      !	cnt += pkts_per_proc;

/* 0x02e0	 199 */		or	%g0,%o2,%l4
/* 0x02e4	 198 */		add	%o0,%l7,%o0
/* 0x02e8	     */		add	%o0,%i5,%o0

!  201		      !	p2p_lock_init( MPID_shmem->availlock + i );

/* 0x02ec	 201 */		or	%g0,1,%o1

!  202		      !	p2p_lock_init( MPID_shmem->incominglock + i );
!  203		      !    }

/* 0x02f0	 203 */		add	%l6,1,%l6
/* 0x02f4	 198 */		st	%g0,[%o0-1044] ! volatile
/* 0x02f8	 199 */		add	%i5,%i4,%i5
/* 0x02fc	 201 */		ld	[%i0],%o0 ! volatile
/* 0x0300	 199 */		add	%l5,%i4,%l5
/* 0x0304	 201 */		add	%o0,%l1,%o0
/* 0x0308	     */		call	mutex_init,3	! Result = %g0
/* 0x030c	     */		or	%g0,0,%o2
/* 0x0310	 202 */		ld	[%i0],%o0 ! volatile
/* 0x0314	     */		or	%g0,1,%o1
/* 0x0318	     */		add	%o0,%l1,%o0
/* 0x031c	     */		add	%o0,768,%o0
/* 0x0320	     */		call	mutex_init,3	! Result = %g0
/* 0x0324	     */		or	%g0,0,%o2
/* 0x0328	 199 */		add	%l0,%i4,%l0
/* 0x032c	 203 */		add	%l3,128,%l3
/* 0x0330	     */		add	%l1,24,%l1
/* 0x0334	     */		add	%l2,4,%l2
/* 0x0338	     */		cmp	%l6,%i2
/* 0x033c	     */		bl,a	.L900000234
/* 0x0340	     */		ld	[%i0],%o0 ! volatile
                       .L77000033:

!  205		      !#if defined (MPI_cspp)
!  206		      !/*
!  207		      ! * Place processes on nodes.
!  208		      ! */
!  209		      !    for (i = 0, curNode = numCurNode = 0; i < numprocs; ++i) {
!  211		      !	while (numCurNode >= numCPUs[curNode]) {
!  212		      !	    if (++curNode == numNodes) {
!  213		      !		fprintf(stderr,
!  214		      !			"Cannot place proc %d (out of %) on a node!\n",
!  215		      !			i, numprocs);
!  216		      !		exit(1);
!  217		      !	    }
!  218		      !	    numCurNode = 0;
!  219		      !	}
!  221		      !	procNode[i] = curNode;
!  222		      !	if (cnx_debug) printf("CNXDB: rank %d -> node %d\n", i, curNode);
!  223		      !	++numCurNode;
!  224		      !    }
!  226		      !#endif
!  228		      !    MPID_numids = numprocs;

/* 0x0344	 228 */		sethi	%hi(MPID_numids),%o0

!  229		      !    MPID_MyWorldSize = numprocs;
!  230		      !/* Above this point, there was a single process.  After the p2p_create_procs
!  231		      !   call, there are more */
!  232		      !    p2p_setpgrp();
!  234		      !#if defined (MPI_cspp)
!  235		      !    p2p_create_procs( numprocs );
!  237		      !    MPID_myNode = myNode = (int) MPID_SHMEM_getNodeId();
!  238		      !    p2p_lock(&(MPID_shmem->globid_lock[myNode]));
!  239		      !    MPID_myid = (MPID_shmem->globid[myNode])++;
!  240		      !    p2p_unlock(&(MPID_shmem->globid_lock[myNode]));
!  242		      !#else
!  243		      !    p2p_create_procs( numprocs - 1 );
!  245		      !    p2p_lock( &MPID_shmem->globlock );

/* 0x0348	 245 */		sethi	%hi(MPID_shmem),%l0
/* 0x034c	 228 */		st	%i2,[%o0+%lo(MPID_numids)] ! volatile
/* 0x0350	 229 */		sethi	%hi(MPID_MyWorldSize),%o0
/* 0x0354	 232 */		call	p2p_setpgrp,0	! Result = %g0
/* 0x0358	     */		st	%i2,[%o0+%lo(MPID_MyWorldSize)] ! volatile
/* 0x035c	 243 */		call	p2p_create_procs,1	! Result = %g0
/* 0x0360	     */		sub	%i2,1,%o0
/* 0x0364	 245 */		ld	[%l0+%lo(MPID_shmem)],%o0 ! volatile
/* 0x0368	     */		call	mutex_lock,1	! Result = %g0
/* 0x036c	     */		add	%o0,1536,%o0

!  246		      !    MPID_myid = MPID_shmem->globid++;

/* 0x0370	 246 */		sethi	%hi(0x23000),%o1
/* 0x0374	     */		ld	[%l0+%lo(MPID_shmem)],%o0 ! volatile
/* 0x0378	     */		add	%o1,536,%o1
/* 0x037c	     */		add	%o0,%o1,%o2
/* 0x0380	     */		sethi	%hi(MPID_myid),%l1
/* 0x0384	     */		ld	[%o2],%o3 ! volatile
/* 0x0388	     */		add	%o3,1,%o2
/* 0x038c	     */		st	%o2,[%o0+%o1] ! volatile
/* 0x0390	     */		st	%o3,[%l1+%lo(MPID_myid)] ! volatile

!  247		      !    p2p_unlock( &MPID_shmem->globlock );

/* 0x0394	 247 */		ld	[%l0+%lo(MPID_shmem)],%o0 ! volatile
/* 0x0398	     */		call	mutex_unlock,1	! Result = %g0
/* 0x039c	     */		add	%o0,1536,%o0

!  248		      !#endif
!  250		      !    MPID_MyWorldRank = MPID_myid;

/* 0x03a0	 250 */		ld	[%l1+%lo(MPID_myid)],%o0 ! volatile
/* 0x03a4	     */		sethi	%hi(MPID_MyWorldRank),%o1

!  251		      !    MPID_SHMEM_FreeSetup();

/* 0x03a8	 251 */		call	MPID_SHMEM_FreeSetup,0	! Result = %g0
/* 0x03ac	     */		st	%o0,[%o1+%lo(MPID_MyWorldRank)] ! volatile

!  253		      !    MPID_incoming = &MPID_shmem->incoming[MPID_myid].head;

/* 0x03b0	 253 */		ld	[%l1+%lo(MPID_myid)],%o0 ! volatile
/* 0x03b4	     */		ld	[%l0+%lo(MPID_shmem)],%o1 ! volatile
/* 0x03b8	     */		sll	%o0,7,%o0
/* 0x03bc	     */		add	%o1,%o0,%o0
/* 0x03c0	     */		add	%o0,1560,%o0
/* 0x03c4	     */		sethi	%hi(MPID_incoming),%o1
/* 0x03c8	     */		st	%o0,[%o1+%lo(MPID_incoming)] ! volatile
/* 0x03cc	     */		ret
/* 0x03d0	     */		restore	%g0,%g0,%g0
                       .L77000021:
/* 0x03d4	 140 */		sethi	%hi(_iob+32),%o0
/* 0x03d8	     */		sethi	%hi(.L537),%o1
/* 0x03dc	     */		add	%o0,%lo(_iob+32),%o0
/* 0x03e0	     */		add	%o1,%lo(.L537),%o1
/* 0x03e4	     */		call	fprintf,3	! Result = %g0
/* 0x03e8	     */		or	%g0,%i3,%o2
/* 0x03ec	 141 */		call	exit,1	! Result = %g0
/* 0x03f0	     */		or	%g0,1,%o0
/* 0x03f4	     */		ba	.L900000233
/* 0x03f8	     */		or	%g0,1,%o0
                       .L77000017:
/* 0x03fc	 122 */		sethi	%hi(_iob+32),%o0
/* 0x0400	     */		sethi	%hi(.L531),%o1
/* 0x0404	     */		add	%o0,%lo(_iob+32),%o0
/* 0x0408	     */		add	%o1,%lo(.L531),%o1
/* 0x040c	     */		call	fprintf,3	! Result = %g0
/* 0x0410	     */		or	%g0,%i2,%o2
/* 0x0414	 123 */		call	exit,1	! Result = %g0
/* 0x0418	     */		or	%g0,1,%o0
/* 0x041c	     */		ba	.L900000232
/* 0x0420	     */		sethi	%hi(.L532),%o0
                       .L77000010:
/* 0x0424	  89 */		sethi	%hi(_iob+32),%o0
/* 0x0428	     */		add	%o0,%lo(_iob+32),%o0
/* 0x042c	     */		call	fprintf,2	! Result = %g0
/* 0x0430	     */		add	%i0,4,%o1
/* 0x0434	  90 */		call	exit,1	! Result = %g0
/* 0x0438	     */		or	%g0,1,%o0
/* 0x043c	     */		ba	.L900000231
/* 0x0440	     */		sll	%i3,2,%l0
/* 0x0444	   0 */		.type	MPID_SHMEM_init,2
/* 0x0444	     */		.size	MPID_SHMEM_init,(.-MPID_SHMEM_init)

	.section	".text",#alloc,#execinstr
/* 000000	   0 */		.align	4
!
! SUBROUTINE MPID_SHMEM_lbarrier
!
! OFFSET    SOURCE LINE	LABEL	INSTRUCTION

                       	.global MPID_SHMEM_lbarrier
                       MPID_SHMEM_lbarrier:
/* 000000	     */		save	%sp,-96,%sp
/* 0x0004	     */		sethi	%hi(.L_mcount_counters+8),%o0
/* 0x0008	     */		call	_mcount,1	! Result = %g0
/* 0x000c	     */		add	%o0,%lo(.L_mcount_counters+8),%o0
/* 0x0010	     */		sethi	%hi(MPID_shmem),%o0
/* 0x0014	     */		add	%o0,%lo(MPID_shmem),%i0
/* 0x0018	     */		sethi	%hi(0x23000),%i1

!  255		      !#if defined (MPI_cspp)
!  256		      !    if (cnx_exec) cnx_start_tool(cnx_exec, argv[0]);
!  257		      !#endif
!  258		      !}
!  260		      !void MPID_SHMEM_lbarrier()
!  261		      !{
!  262		      !    VOLATILE int *cnt, *cntother;
!  264		      !/* Figure out which counter to decrement */
!  265		      !    if (MPID_shmem->barrier.phase == 1) {

/* 0x001c	 265 */		add	%i1,544,%o1
/* 0x0020	     */		ld	[%i0],%o0 ! volatile
/* 0x0024	     */		ld	[%o0+%o1],%o0 ! volatile
/* 0x0028	     */		cmp	%o0,1
/* 0x002c	     */		bne	.L77000046
/* 0x0030	     */		add	%i1,548,%o1
                       .L77000045:

!  266		      !	cnt	     = &MPID_shmem->barrier.cnt1;

/* 0x0034	 266 */		ld	[%i0],%o0 ! volatile
/* 0x0038	     */		add	%o0,%o1,%i3

!  267		      !	cntother = &MPID_shmem->barrier.cnt2;

/* 0x003c	 267 */		ld	[%i0],%o0 ! volatile
/* 0x0040	     */		add	%i1,552,%o1
/* 0x0044	     */		add	%o0,%o1,%i2
/* 0x0048	     */		ba	.L900000311
/* 0x004c	     */		ld	[%i0],%o0 ! volatile
                       .L77000046:

!  268		      !    }
!  269		      !    else {
!  270		      !	cnt	     = &MPID_shmem->barrier.cnt2;

/* 0x0050	 270 */		ld	[%i0],%o0 ! volatile
/* 0x0054	     */		add	%i1,552,%o1
/* 0x0058	     */		add	%o0,%o1,%i3

!  271		      !	cntother = &MPID_shmem->barrier.cnt1;

/* 0x005c	 271 */		ld	[%i0],%o0 ! volatile
/* 0x0060	     */		add	%i1,548,%o1
/* 0x0064	     */		add	%o0,%o1,%i2

!  272		      !    }
!  274		      !/* Decrement it atomically */
!  275		      !    p2p_lock( &MPID_shmem->globlock );

/* 0x0068	 275 */		ld	[%i0],%o0 ! volatile
                       .L900000311:
/* 0x006c	 275 */		call	mutex_lock,1	! Result = %g0
/* 0x0070	     */		add	%o0,1536,%o0

!  276		      !    *cnt = *cnt - 1;

/* 0x0074	 276 */		ld	[%i3],%o0 ! volatile
/* 0x0078	     */		sub	%o0,1,%o0
/* 0x007c	     */		st	%o0,[%i3] ! volatile

!  277		      !    p2p_unlock( &MPID_shmem->globlock );

/* 0x0080	 277 */		ld	[%i0],%o0 ! volatile
/* 0x0084	     */		call	mutex_unlock,1	! Result = %g0
/* 0x0088	     */		add	%o0,1536,%o0

!  278		      !    
!  279		      !/* Wait for everyone to to decrement it */
!  280		      !    while ( *cnt ) p2p_yield();

/* 0x008c	 280 */		ld	[%i3],%o0 ! volatile
/* 0x0090	     */		cmp	%o0,0
/* 0x0094	     */		be	.L900000310
/* 0x0098	     */		sethi	%hi(MPID_myid),%o0
                       .L77000049:
/* 0x009c	     */		call	p2p_yield,0	! Result = %g0
/* 0x00a0	     */		nop
/* 0x00a4	     */		ld	[%i3],%o0 ! volatile
/* 0x00a8	     */		cmp	%o0,0
/* 0x00ac	     */		bne	.L77000049
/* 0x00b0	     */		nop
                       .L77000051:

!  282		      !/* If process 0, change phase. Reset the OTHER counter*/
!  283		      !    if (MPID_myid == 0) {

/* 0x00b4	 283 */		sethi	%hi(MPID_myid),%o0
                       .L900000310:
/* 0x00b8	 283 */		ld	[%o0+%lo(MPID_myid)],%o0 ! volatile
/* 0x00bc	     */		cmp	%o0,0
/* 0x00c0	     */		bne	.L77000056
/* 0x00c4	     */		add	%i1,544,%o1
                       .L77000052:

!  284		      !	MPID_shmem->barrier.phase = ! MPID_shmem->barrier.phase;

/* 0x00c8	 284 */		ld	[%i0],%o0 ! volatile
/* 0x00cc	     */		ld	[%o0+%o1],%o0 ! volatile
/* 0x00d0	     */		cmp	%o0,0
/* 0x00d4	     */		bne	.L77000054
/* 0x00d8	     */		or	%g0,1,%o2
                       .L77000053:
/* 0x00dc	     */		ba	.L900000309
/* 0x00e0	     */		ld	[%i0],%o0 ! volatile
                       .L77000054:
/* 0x00e4	     */		or	%g0,0,%o2
/* 0x00e8	     */		ld	[%i0],%o0 ! volatile
                       .L900000309:
/* 0x00ec	     */		st	%o2,[%o0+%o1] ! volatile

!  285		      !	p2p_write_sync();

/* 0x00f0	 285 */		ld	[%i0],%o0 ! volatile
/* 0x00f4	     */		call	mutex_lock,1	! Result = %g0
/* 0x00f8	     */		add	%o0,1536,%o0
/* 0x00fc	     */		ld	[%i0],%o0 ! volatile
/* 0x0100	     */		call	mutex_unlock,1	! Result = %g0
/* 0x0104	     */		add	%o0,1536,%o0

!  286		      !	*cntother = MPID_shmem->barrier.size;

/* 0x0108	 286 */		ld	[%i0],%o0 ! volatile
/* 0x010c	     */		add	%i1,540,%o1
/* 0x0110	     */		add	%o0,%o1,%o0
/* 0x0114	     */		ld	[%o0],%o0 ! volatile
/* 0x0118	     */		st	%o0,[%i2] ! volatile
/* 0x011c	     */		ret
/* 0x0120	     */		restore	%g0,%g0,%g0
                       .L77000056:

!  287		      !    }
!  288		      !    else 
!  289		      !	while (! *cntother) p2p_yield();

/* 0x0124	 289 */		ld	[%i2],%o0 ! volatile
/* 0x0128	     */		cmp	%o0,0
/* 0x012c	     */		bne	.L77000062
/* 0x0130	     */		nop
                       .L77000058:
/* 0x0134	     */		call	p2p_yield,0	! Result = %g0
/* 0x0138	     */		nop
/* 0x013c	     */		ld	[%i2],%o0 ! volatile
/* 0x0140	     */		cmp	%o0,0
/* 0x0144	     */		be	.L77000058
/* 0x0148	     */		nop
                       .L77000062:
/* 0x014c	     */		ret
/* 0x0150	     */		restore	%g0,%g0,%g0
/* 0x0154	   0 */		.type	MPID_SHMEM_lbarrier,2
/* 0x0154	     */		.size	MPID_SHMEM_lbarrier,(.-MPID_SHMEM_lbarrier)

	.section	".text",#alloc,#execinstr
/* 000000	   0 */		.align	4
!
! SUBROUTINE MPID_SHMEM_finalize
!
! OFFSET    SOURCE LINE	LABEL	INSTRUCTION

                       	.global MPID_SHMEM_finalize
                       MPID_SHMEM_finalize:
/* 000000	     */		save	%sp,-96,%sp
/* 0x0004	     */		sethi	%hi(.L_mcount_counters+12),%o0
/* 0x0008	     */		call	_mcount,1	! Result = %g0
/* 0x000c	     */		add	%o0,%lo(.L_mcount_counters+12),%o0

!  290		      !}
!  292		      !void MPID_SHMEM_finalize()
!  293		      !{
!  294		      !    VOLATILE int *globid;
!  296		      !    fflush(stdout);

/* 0x0010	 296 */		sethi	%hi(_iob+16),%o0
/* 0x0014	     */		call	fflush,1	! Result = %g0
/* 0x0018	     */		add	%o0,%lo(_iob+16),%o0

!  297		      !    fflush(stderr);

/* 0x001c	 297 */		sethi	%hi(_iob+32),%o0
/* 0x0020	     */		call	fflush,1	! Result = %g0
/* 0x0024	     */		add	%o0,%lo(_iob+32),%o0

!  299		      !/* There is a potential race condition here if we want to catch
!  300		      !   exiting children.  We should probably have each child indicate a successful
!  301		      !   termination rather than this simple count.  To reduce this race condition,
!  302		      !   we'd like to perform an MPI barrier before clearing the signal handler.
!  304		      !   However, in the current code, MPID_xxx_End is called after most of the
!  305		      !   MPI system is deactivated.  Thus, we use a simple count-down barrier.
!  306		      !   Eventually, we the fast barrier routines.
!  307		      ! */
!  308		      !/* MPI_Barrier( MPI_COMM_WORLD ); */
!  309		      !    MPID_SHMEM_lbarrier();

/* 0x0028	 309 */		call	MPID_SHMEM_lbarrier,0	! Result = %g0
/* 0x002c	     */		nop

!  310		      !    p2p_clear_signal();

/* 0x0030	 310 */		call	p2p_clear_signal,0	! Result = %g0
/* 0x0034	     */		nop

!  312		      !/* Wait for everyone to finish 
!  313		      !   We can NOT simply use MPID_shmem->globid here because there is always the 
!  314		      !   possibility that some process is already exiting before another process
!  315		      !   has completed starting (and we've actually seen this behavior).
!  316		      !   Instead, we perform an additional MPI Barrier.
!  317		      !*/
!  318		      !    MPID_SHMEM_lbarrier();

/* 0x0038	 318 */		call	MPID_SHMEM_lbarrier,0	! Result = %g0
/* 0x003c	     */		nop

!  319		      !/* MPI_Barrier( MPI_COMM_WORLD ); */
!  320		      !#ifdef FOO
!  321		      !    globid = &MPID_shmem->globid;
!  322		      !    p2p_lock( &MPID_shmem->globlock );
!  323		      !    MPID_shmem->globid--;
!  324		      !    p2p_unlock( &MPID_shmem->globlock );
!  325		      !/* Note that this forces all jobs to spin until everyone has exited */
!  326		      !    while (*globid) p2p_yield(); /* MPID_shmem->globid) ; */
!  327		      !#endif
!  329		      !    p2p_cleanup();

/* 0x0040	 329 */		call	p2p_cleanup,0	! Result = %g0	! (tail call)
/* 0x0044	     */		restore	%g0,%g0,%g0
                       .L77000069:
/* 0x0048	     */		ret
/* 0x004c	     */		restore	%g0,%g0,%g0
/* 0x0050	   0 */		.type	MPID_SHMEM_finalize,2
/* 0x0050	     */		.size	MPID_SHMEM_finalize,(.-MPID_SHMEM_finalize)

	.section	".text",#alloc,#execinstr
/* 000000	   0 */		.align	4
!
! SUBROUTINE MPID_SHMEM_ReadControl
!
! OFFSET    SOURCE LINE	LABEL	INSTRUCTION

                       	.global MPID_SHMEM_ReadControl
                       MPID_SHMEM_ReadControl:
/* 000000	     */		save	%sp,-96,%sp
/* 0x0004	     */		sethi	%hi(.L_mcount_counters+16),%o0
/* 0x0008	     */		call	_mcount,1	! Result = %g0
/* 0x000c	     */		add	%o0,%lo(.L_mcount_counters+16),%o0

!  330		      !}
!  332		      !/* 
!  333		      !  Read an incoming control message.
!  335		      !  NOTE:
!  336		      !  This routine maintains and internal list of elements; this allows it to
!  337		      !  read from that list without locking it.
!  338		      ! */
!  339		      !/* #define BACKOFF_LMT 1048576 */
!  340		      !#define BACKOFF_LMT 1024
!  341		      !/* 
!  342		      !   This version assumes that the packets are dynamically allocated (not off of
!  343		      !   the stack).  This lets us use packets that live in shared memory.
!  345		      !   NOTE THE DIFFERENCES IN BINDINGS from the usual versions.
!  346		      ! */
!  347		      !int MPID_SHMEM_ReadControl( pkt, size, from )
!  348		      !MPID_PKT_T **pkt;
!  349		      !int        size, *from;
!  350		      !{
!  351		      !    MPID_PKT_T *inpkt;
!  352		      !    int        backoff, cnt;
!  353		      !    VOLATILE   MPID_PKT_T **ready;
!  355		      !    if (MPID_local) {

/* 0x0010	 355 */		sethi	%hi(MPID_local),%o1
/* 0x0014	     */		ld	[%o1+%lo(MPID_local)],%o0 ! volatile
/* 0x0018	     */		cmp	%o0,0
/* 0x001c	     */		be	.L900000513
/* 0x0020	     */		sethi	%hi(MPID_myid),%o0
                       .L77000072:

!  356		      !	inpkt      = (MPID_PKT_T *)MPID_local;

/* 0x0024	 356 */		ld	[%o1+%lo(MPID_local)],%i3 ! volatile

!  357		      !	MPID_local = MPID_local->head.next;

/* 0x0028	 357 */		ld	[%o1+%lo(MPID_local)],%o0 ! volatile
/* 0x002c	     */		ld	[%o0+4],%o0 ! volatile
/* 0x0030	     */		st	%o0,[%o1+%lo(MPID_local)] ! volatile
/* 0x0034	     */		ba	.L900000512
/* 0x0038	     */		st	%i3,[%i0] ! volatile
                       .L900000513:
/* 0x003c	     */		add	%o0,%lo(MPID_myid),%i1

!  358		      !    }
!  359		      !    else {
!  360		      !	if (!MPID_lshmem.incomingPtr[MPID_myid]->head) {

/* 0x0040	 360 */		sethi	%hi(MPID_lshmem+256),%o1
/* 0x0044	     */		ld	[%i1],%o0 ! volatile
/* 0x0048	     */		add	%o1,%lo(MPID_lshmem+256),%i3
/* 0x004c	     */		sll	%o0,2,%o0
/* 0x0050	     */		ld	[%o0+%i3],%o0 ! volatile
/* 0x0054	     */		ld	[%o0],%o0 ! volatile
/* 0x0058	     */		cmp	%o0,0
/* 0x005c	     */		bne,a	.L900000510
/* 0x0060	     */		ld	[%i1],%o0 ! volatile
                       .L77000074:

!  361		      !	    /* This code tries to let other processes run.  If there
!  362		      !	       are more physical processors than processes, then a simple
!  363		      !	       while (!MPID_shmem->incoming[MPID_myid].head);
!  364		      !	       might be better.
!  365		      !	       We might also want to do
!  367		      !	       VOLATILE MPID_PKT_T *msg_ptr = 
!  368		      !	       &MPID_shmem->incoming[MPID_myid].head;
!  369		      !	       while (!*msg_ptr) { .... }
!  371		      !	       This code should be tuned with vendor help, since it depends
!  372		      !	       on fine details of the hardware and system.
!  373		      !	       */
!  374		      !#if defined(MPI_cspp)
!  375		      !	    if (cnx_yield) {
!  376		      !#endif
!  377		      !		backoff = 1;
!  378		      !		ready = &MPID_lshmem.incomingPtr[MPID_myid]->head;

/* 0x0064	 378 */		ld	[%i1],%o0 ! volatile
/* 0x0068	 377 */		or	%g0,1,%i5
/* 0x006c	 378 */		sll	%o0,2,%o0
/* 0x0070	     */		ld	[%o0+%i3],%i4 ! volatile

!  379		      !/*		while (!MPID_lshmem.incomingPtr[MPID_myid]->head) { */
!  380		      !		while (!*ready) {

/* 0x0074	 380 */		ld	[%i4],%o0 ! volatile
/* 0x0078	     */		cmp	%o0,0
/* 0x007c	     */		bne,a	.L900000510
/* 0x0080	     */		ld	[%i1],%o0 ! volatile
                       .L77000076:

!  381		      !		    cnt	    = backoff;
!  382		      !		    while (cnt--) ;

/* 0x0084	 382 */		sub	%i5,1,%o0
                       .L900000509:
/* 0x0088	 382 */		cmp	%i5,0
/* 0x008c	     */		be,a	.L900000511
/* 0x0090	     */		sll	%i5,1,%i5
                       .L77000078:
/* 0x0094	     */		orcc	%g0,%o0,%g0
/* 0x0098	     */		bne	.L77000078
/* 0x009c	     */		sub	%o0,1,%o0
                       .L77000080:

!  383		      !		    backoff = 2 * backoff;

/* 0x00a0	 383 */		sll	%i5,1,%i5
                       .L900000511:

!  384		      !		    if (backoff > BACKOFF_LMT) backoff = BACKOFF_LMT;

/* 0x00a4	 384 */		cmp	%i5,1024
/* 0x00a8	     */		bg,a	.L77000082
/* 0x00ac	     */		or	%g0,1024,%i5
                       .L77000082:

!  385		      !		    if (MPID_lshmem.incomingPtr[MPID_myid]->head) break;

/* 0x00b0	 385 */		ld	[%i1],%o0 ! volatile
/* 0x00b4	     */		sll	%o0,2,%o0
/* 0x00b8	     */		ld	[%o0+%i3],%o0 ! volatile
/* 0x00bc	     */		ld	[%o0],%o0 ! volatile
/* 0x00c0	     */		cmp	%o0,0
/* 0x00c4	     */		bne,a	.L900000510
/* 0x00c8	     */		ld	[%i1],%o0 ! volatile
                       .L77000084:

!  386		      !		    /* Return the packets that we have before doing a
!  387		      !		       yield */
!  388		      !		    MPID_SHMEM_FlushPkts();

/* 0x00cc	 388 */		call	MPID_SHMEM_FlushPkts,0	! Result = %g0
/* 0x00d0	     */		nop

!  389		      !		    p2p_yield();

/* 0x00d4	 389 */		call	p2p_yield,0	! Result = %g0
/* 0x00d8	     */		nop

!  390		      !		}

/* 0x00dc	 390 */		ld	[%i4],%o0 ! volatile
/* 0x00e0	     */		cmp	%o0,0
/* 0x00e4	     */		be	.L900000509
/* 0x00e8	     */		sub	%i5,1,%o0
                       .L77000087:

!  391		      !#if defined(MPI_cspp)
!  392		      !	    } else {
!  393		      !		while (!MPID_lshmem.incomingPtr[MPID_myid]->head);
!  394		      !	    }
!  395		      !#endif
!  396		      !	}
!  397		      !	/* This code drains the ENTIRE list into a local list */
!  398		      !	p2p_lock( MPID_lshmem.incominglockPtr[MPID_myid] );

/* 0x00ec	 398 */		ld	[%i1],%o0 ! volatile
                       .L900000510:
/* 0x00f0	 398 */		sethi	%hi(MPID_lshmem+128),%o1
/* 0x00f4	     */		add	%o1,%lo(MPID_lshmem+128),%l0
/* 0x00f8	     */		sll	%o0,2,%o0
/* 0x00fc	     */		call	mutex_lock,1	! Result = %g0
/* 0x0100	     */		ld	[%o0+%l0],%o0 ! volatile

!  399		      !	inpkt          = (MPID_PKT_T *) *MPID_incoming;

/* 0x0104	 399 */		sethi	%hi(MPID_incoming),%o0

!  400		      !	MPID_local     = inpkt->head.next;

/* 0x0108	 400 */		sethi	%hi(MPID_local),%o2
/* 0x010c	 399 */		ld	[%o0+%lo(MPID_incoming)],%o1 ! volatile
/* 0x0110	     */		ld	[%o1],%i3 ! volatile
/* 0x0114	 400 */		ld	[%i3+4],%o1 ! volatile
/* 0x0118	     */		st	%o1,[%o2+%lo(MPID_local)] ! volatile

!  401		      !	*MPID_incoming = 0;
!  402		      !	MPID_lshmem.incomingPtr[MPID_myid]->tail = 0;

/* 0x011c	 402 */		sethi	%hi(MPID_lshmem+256),%o1
/* 0x0120	 401 */		ld	[%o0+%lo(MPID_incoming)],%o0 ! volatile
/* 0x0124	 402 */		add	%o1,%lo(MPID_lshmem+256),%o1
/* 0x0128	 401 */		st	%g0,[%o0] ! volatile
/* 0x012c	 402 */		ld	[%i1],%o0 ! volatile
/* 0x0130	     */		sll	%o0,2,%o0
/* 0x0134	     */		ld	[%o0+%o1],%o0 ! volatile
/* 0x0138	     */		st	%g0,[%o0+4] ! volatile

!  403		      !	p2p_unlock( MPID_lshmem.incominglockPtr[MPID_myid] );

/* 0x013c	 403 */		ld	[%i1],%o0 ! volatile
/* 0x0140	     */		sll	%o0,2,%o0
/* 0x0144	     */		call	mutex_unlock,1	! Result = %g0
/* 0x0148	     */		ld	[%o0+%l0],%o0 ! volatile

!  404		      !    }
!  406		      !/* Deliver this packet to the caller */
!  407		      !    *pkt  = inpkt;

/* 0x014c	 407 */		st	%i3,[%i0] ! volatile
                       .L900000512:

!  409		      !    *from = (*inpkt).head.src;

/* 0x0150	 409 */		ld	[%i3+12],%o0 ! volatile
/* 0x0154	     */		st	%o0,[%i2] ! volatile
/* 0x0158	     */		ret
/* 0x015c	     */		restore	%g0,0,%o0
/* 0x0160	   0 */		.type	MPID_SHMEM_ReadControl,2
/* 0x0160	     */		.size	MPID_SHMEM_ReadControl,(.-MPID_SHMEM_ReadControl)

	.section	".text",#alloc,#execinstr
/* 000000	   0 */		.align	4
!
! SUBROUTINE MPID_SHMEM_FreeSetup
!
! OFFSET    SOURCE LINE	LABEL	INSTRUCTION

                       	.global MPID_SHMEM_FreeSetup
                       MPID_SHMEM_FreeSetup:
/* 000000	     */		save	%sp,-96,%sp
/* 0x0004	     */		sethi	%hi(.L_mcount_counters+20),%o0
/* 0x0008	     */		call	_mcount,1	! Result = %g0
/* 0x000c	     */		add	%o0,%lo(.L_mcount_counters+20),%o0
/* 0x0010	     */		sethi	%hi(MPID_numids),%o0
/* 0x0014	     */		add	%o0,%lo(MPID_numids),%o1

!  411		      !    MPID_TRACE_CODE_PKT("Readpkt",*from,(*inpkt).head.mode);
!  413		      !    return MPI_SUCCESS;
!  414		      !}
!  417		      !/*
!  418		      !   Rather than free recv packets every time, we accumulate a few
!  419		      !   and then return them in a group.  
!  421		      !   This is useful when a processes sends several messages to the same
!  422		      !   destination.
!  423		      !   
!  424		      !   This keeps a list for each possible processor, and returns them
!  425		      !   all when MPID_pktflush are available FROM ANY SOURCE.
!  426		      ! */
!  427		      !static MPID_PKT_T *FreePkts[MPID_MAX_PROCS];
!  428		      !static MPID_PKT_T *FreePktsTail[MPID_MAX_PROCS];
!  429		      !static int to_free = 0;
!  431		      !void MPID_SHMEM_FreeSetup()
!  432		      !{
!  433		      !    int i;
!  434		      !    for (i=0; i<MPID_numids; i++) FreePkts[i] = 0;

/* 0x0018	 434 */		ld	[%o1],%o0 ! volatile
/* 0x001c	     */		cmp	%o0,0
/* 0x0020	     */		ble	.L77000100
/* 0x0024	     */		or	%g0,0,%o3
                       .L77000103:
/* 0x0028	     */		sethi	%hi(FreePkts),%o0
/* 0x002c	     */		add	%o0,%lo(FreePkts),%o2
                       .L77000097:
/* 0x0030	     */		st	%g0,[%o2] ! volatile
                       .L900000606:
/* 0x0034	     */		ld	[%o1],%o0 ! volatile
/* 0x0038	     */		add	%o3,1,%o3
/* 0x003c	     */		add	%o2,4,%o2
/* 0x0040	     */		cmp	%o3,%o0
/* 0x0044	     */		bl,a	.L900000606
/* 0x0048	     */		st	%g0,[%o2] ! volatile
                       .L77000100:
/* 0x004c	     */		ret
/* 0x0050	     */		restore	%g0,%g0,%g0
/* 0x0054	   0 */		.type	MPID_SHMEM_FreeSetup,2
/* 0x0054	     */		.size	MPID_SHMEM_FreeSetup,(.-MPID_SHMEM_FreeSetup)

	.section	".text",#alloc,#execinstr
/* 000000	   0 */		.align	4
!
! SUBROUTINE MPID_SHMEM_FlushPkts
!
! OFFSET    SOURCE LINE	LABEL	INSTRUCTION

                       	.global MPID_SHMEM_FlushPkts
                       MPID_SHMEM_FlushPkts:
/* 000000	     */		save	%sp,-96,%sp
/* 0x0004	     */		sethi	%hi(.L_mcount_counters+24),%o0
/* 0x0008	     */		call	_mcount,1	! Result = %g0
/* 0x000c	     */		add	%o0,%lo(.L_mcount_counters+24),%o0

!  435		      !}
!  437		      !void MPID_SHMEM_FlushPkts()
!  438		      !{
!  439		      !    int i;
!  440		      !    MPID_PKT_T *pkt;
!  441		      !    MPID_PKT_T *tail;
!  443		      !    if (to_free == 0) return;

/* 0x0010	 443 */		sethi	%hi(to_free),%o0
/* 0x0014	     */		ld	[%o0+%lo(to_free)],%o0 ! volatile
/* 0x0018	     */		cmp	%o0,0
/* 0x001c	     */		be	.L77000113
/* 0x0020	     */		nop
                       .L77000106:
/* 0x0024	     */		sethi	%hi(MPID_numids),%o0
/* 0x0028	     */		add	%o0,%lo(MPID_numids),%i0

!  444		      !    for (i=0; i<MPID_numids; i++) {

/* 0x002c	 444 */		ld	[%i0],%o0 ! volatile
/* 0x0030	     */		cmp	%o0,0
/* 0x0034	     */		ble	.L77000112
/* 0x0038	     */		or	%g0,0,%l1
                       .L77000116:
/* 0x003c	     */		sethi	%hi(MPID_lshmem+384),%o0
/* 0x0040	     */		add	%o0,%lo(MPID_lshmem+384),%i4
/* 0x0044	     */		sethi	%hi(FreePkts),%o0
/* 0x0048	     */		add	%o0,%lo(FreePkts),%i3
/* 0x004c	     */		sethi	%hi(FreePktsTail),%o0
/* 0x0050	     */		add	%o0,%lo(FreePktsTail),%i1
/* 0x0054	     */		or	%g0,0,%l2
/* 0x0058	     */		sethi	%hi(MPID_lshmem),%o0
/* 0x005c	     */		add	%o0,%lo(MPID_lshmem),%i2

!  445		      !	if ((pkt = FreePkts[i])) {

/* 0x0060	 445 */		ld	[%l2+%i3],%i5 ! volatile
                       .L900000709:
/* 0x0064	 445 */		cmp	%i5,0
/* 0x0068	     */		be	.L77000110
/* 0x006c	     */		add	%l1,1,%l1
                       .L77000109:

!  446		      !	    tail			  = FreePktsTail[i];

/* 0x0070	 446 */		ld	[%l2+%i1],%l0 ! volatile

!  447		      !	    p2p_lock( MPID_lshmem.availlockPtr[i] );

/* 0x0074	 447 */		call	mutex_lock,1	! Result = %g0
/* 0x0078	     */		ld	[%l2+%i2],%o0 ! volatile

!  448		      !	    tail->head.next		  = 
!  449		      !		(MPID_PKT_T *)MPID_lshmem.availPtr[i]->head;

/* 0x007c	 449 */		ld	[%l2+%i4],%o0 ! volatile
/* 0x0080	     */		ld	[%o0],%o0 ! volatile
/* 0x0084	     */		st	%o0,[%l0+4] ! volatile

!  450		      !	    MPID_lshmem.availPtr[i]->head = pkt;

/* 0x0088	 450 */		ld	[%l2+%i4],%o0 ! volatile
/* 0x008c	     */		st	%i5,[%o0] ! volatile

!  451		      !	    p2p_unlock( MPID_lshmem.availlockPtr[i] );

/* 0x0090	 451 */		call	mutex_unlock,1	! Result = %g0
/* 0x0094	     */		ld	[%l2+%i2],%o0 ! volatile

!  452		      !	    FreePkts[i] = 0;

/* 0x0098	 452 */		st	%g0,[%l2+%i3] ! volatile
                       .L77000110:

!  453		      !	}
!  454		      !    }

/* 0x009c	 454 */		ld	[%i0],%o0 ! volatile
/* 0x00a0	     */		add	%l2,4,%l2
/* 0x00a4	     */		cmp	%l1,%o0
/* 0x00a8	     */		bl,a	.L900000709
/* 0x00ac	     */		ld	[%l2+%i3],%i5 ! volatile
                       .L77000112:

!  455		      !    to_free = 0;

/* 0x00b0	 455 */		sethi	%hi(to_free),%o0
/* 0x00b4	     */		st	%g0,[%o0+%lo(to_free)] ! volatile
                       .L77000113:
/* 0x00b8	     */		ret
/* 0x00bc	     */		restore	%g0,%g0,%g0
/* 0x00c0	   0 */		.type	MPID_SHMEM_FlushPkts,2
/* 0x00c0	     */		.size	MPID_SHMEM_FlushPkts,(.-MPID_SHMEM_FlushPkts)

	.section	".text",#alloc,#execinstr
/* 000000	   0 */		.align	4
!
! SUBROUTINE MPID_SHMEM_FreeRecvPkt
!
! OFFSET    SOURCE LINE	LABEL	INSTRUCTION

                       	.global MPID_SHMEM_FreeRecvPkt
                       MPID_SHMEM_FreeRecvPkt:
/* 000000	     */		save	%sp,-96,%sp
/* 0x0004	     */		sethi	%hi(.L_mcount_counters+28),%o0
/* 0x0008	     */		call	_mcount,1	! Result = %g0
/* 0x000c	     */		add	%o0,%lo(.L_mcount_counters+28),%o0

!  456		      !}
!  458		      !void MPID_SHMEM_FreeRecvPkt( pkt )
!  459		      !MPID_PKT_T *pkt;
!  460		      !{
!  461		      !    int        src, i;
!  462		      !    MPID_PKT_T *tail;
!  464		      !    MPID_TRACE_CODE_PKT("Freepkt",pkt->head.owner,(pkt->head.mode));
!  466		      !    src	       = pkt->head.owner;
!  467		      !/*
!  468		      !    if (src == MPID_MyWorldRank) {
!  469		      !	pkt->head.next = MPID_localavail;
!  470		      !	MPID_localavail = pkt;
!  471		      !	return;
!  472		      !    }
!  473		      !    */
!  474		      !    pkt->head.next = FreePkts[src];

/* 0x0010	 474 */		ld	[%i0+8],%o0 ! volatile
/* 0x0014	     */		sll	%o0,2,%o3
/* 0x0018	     */		sethi	%hi(FreePkts),%o0
/* 0x001c	     */		add	%o0,%lo(FreePkts),%o2
/* 0x0020	     */		add	%o3,%o2,%o0
/* 0x0024	     */		ld	[%o0],%o1 ! volatile
/* 0x0028	     */		st	%o1,[%i0+4] ! volatile

!  475		      !/* Set the tail if we're the first */
!  476		      !    if (!FreePkts[src])

/* 0x002c	 476 */		ld	[%o0],%o0 ! volatile
/* 0x0030	     */		cmp	%o0,0
/* 0x0034	     */		bne,a	.L900000809
/* 0x0038	     */		st	%i0,[%o3+%o2] ! volatile
                       .L77000118:

!  477		      !	FreePktsTail[src] = pkt;

/* 0x003c	 477 */		sethi	%hi(FreePktsTail),%o0
/* 0x0040	     */		add	%o0,%lo(FreePktsTail),%o0
/* 0x0044	     */		st	%i0,[%o3+%o0] ! volatile

!  478		      !    FreePkts[src]  = pkt;

/* 0x0048	 478 */		st	%i0,[%o3+%o2] ! volatile
                       .L900000809:

!  479		      !    to_free++;

/* 0x004c	 479 */		sethi	%hi(to_free),%o0
/* 0x0050	     */		ld	[%o0+%lo(to_free)],%o1 ! volatile
/* 0x0054	     */		add	%o1,1,%o1
/* 0x0058	     */		st	%o1,[%o0+%lo(to_free)] ! volatile

!  481		      !    if (to_free >= MPID_pktflush) {

/* 0x005c	 481 */		sethi	%hi(MPID_pktflush),%o1
/* 0x0060	     */		ld	[%o1+%lo(MPID_pktflush)],%o1 ! volatile
/* 0x0064	     */		ld	[%o0+%lo(to_free)],%o0 ! volatile
/* 0x0068	     */		cmp	%o0,%o1
/* 0x006c	     */		bl	.L77000122
/* 0x0070	     */		nop
                       .L77000120:

!  482		      !	MPID_SHMEM_FlushPkts();

/* 0x0074	 482 */		call	MPID_SHMEM_FlushPkts,0	! Result = %g0	! (tail call)
/* 0x0078	     */		restore	%g0,%g0,%g0
                       .L77000122:
/* 0x007c	     */		ret
/* 0x0080	     */		restore	%g0,%g0,%g0
/* 0x0084	   0 */		.type	MPID_SHMEM_FreeRecvPkt,2
/* 0x0084	     */		.size	MPID_SHMEM_FreeRecvPkt,(.-MPID_SHMEM_FreeRecvPkt)

	.section	".text",#alloc,#execinstr
/* 000000	   0 */		.align	4
!
! SUBROUTINE MPID_SHMEM_GetSendPkt
!
! OFFSET    SOURCE LINE	LABEL	INSTRUCTION

                       	.global MPID_SHMEM_GetSendPkt
                       MPID_SHMEM_GetSendPkt:
/* 000000	     */		save	%sp,-96,%sp
/* 0x0004	     */		sethi	%hi(.L_mcount_counters+32),%o0
/* 0x0008	     */		call	_mcount,1	! Result = %g0
/* 0x000c	     */		add	%o0,%lo(.L_mcount_counters+32),%o0

!  483		      !    }
!  484		      !}
!  486		      !/* 
!  487		      !   If this is set, then packets are allocated, then set, then passed to the
!  488		      !   sendcontrol routine.  If not, packets are created on the call stack and
!  489		      !   then copied to a shared-memory packet.
!  491		      !   We should probably make "localavail" global, then we can use a macro
!  492		      !   to allocate packets as long as there is a local supply of them.
!  494		      !   For example, just
!  495		      !       extern MPID_PKT_T *MPID_localavail = 0;
!  496		      !   and the
!  497		      !   #define ...GetSendPkt(inpkt) \
!  498		      !   {if (MPID_localavail) {inpkt= MPID_localavail; \
!  499		      !   MPID_localavail=inpkt->head.next;}else inpkt = routine();\
!  500		      !   inpkt->head.next = 0;}
!  501		      ! */
!  502		      !MPID_PKT_T *MPID_SHMEM_GetSendPkt(nonblock)
!  503		      !int nonblock;
!  504		      !{
!  505		      !    MPID_PKT_T *inpkt;
!  506		      !    static MPID_PKT_T *localavail = 0;
!  507		      !    int   freecnt=0;
!  509		      !    if (localavail) {

/* 0x0010	 509 */		sethi	%hi(.L644),%o1
/* 0x0014	 504 */		or	%g0,%i0,%i5
/* 0x0018	 509 */		ld	[%o1+%lo(.L644)],%o0 ! volatile
/* 0x001c	     */		cmp	%o0,0
/* 0x0020	     */		be	.L77000139
/* 0x0024	     */		or	%g0,0,%i4
                       .L77000125:

!  510		      !	inpkt      = localavail;

/* 0x0028	 510 */		ld	[%o1+%lo(.L644)],%i0 ! volatile
/* 0x002c	     */		ba	.L900000909
/* 0x0030	     */		ld	[%i0+4],%o0 ! volatile
                       .L77000139:
/* 0x0034	     */		sethi	%hi(MPID_myid),%o0
/* 0x0038	     */		add	%o0,%lo(MPID_myid),%i1
/* 0x003c	     */		sethi	%hi(MPID_lshmem+384),%o0
/* 0x0040	     */		add	%o0,%lo(MPID_lshmem+384),%i3
/* 0x0044	     */		sethi	%hi(MPID_lshmem),%o0
/* 0x0048	     */		add	%o0,%lo(MPID_lshmem),%i2

!  511		      !    }
!  512		      !    else {
!  513		      !	/* If there are no available packets, this code does a yield */
!  514		      !	while (1) {
!  515		      !	    p2p_lock( MPID_lshmem.availlockPtr[MPID_myid] );

/* 0x004c	 515 */		ld	[%i1],%o0 ! volatile
                       .L900000908:
/* 0x0050	 515 */		sll	%o0,2,%o0
/* 0x0054	     */		call	mutex_lock,1	! Result = %g0
/* 0x0058	     */		ld	[%o0+%i2],%o0 ! volatile

!  516		      !	    inpkt			     = 
!  517		      !		(MPID_PKT_T *)MPID_lshmem.availPtr[MPID_myid]->head;

/* 0x005c	 517 */		ld	[%i1],%o0 ! volatile
/* 0x0060	     */		sll	%o0,2,%o0
/* 0x0064	     */		ld	[%o0+%i3],%o0 ! volatile
/* 0x0068	     */		ld	[%o0],%i0 ! volatile

!  518		      !	    MPID_lshmem.availPtr[MPID_myid]->head = 0;

/* 0x006c	 518 */		ld	[%i1],%o0 ! volatile
/* 0x0070	     */		sll	%o0,2,%o0
/* 0x0074	     */		ld	[%o0+%i3],%o0 ! volatile
/* 0x0078	     */		st	%g0,[%o0] ! volatile

!  519		      !	    p2p_unlock( MPID_lshmem.availlockPtr[MPID_myid] );

/* 0x007c	 519 */		ld	[%i1],%o0 ! volatile
/* 0x0080	     */		sll	%o0,2,%o0
/* 0x0084	     */		call	mutex_unlock,1	! Result = %g0
/* 0x0088	     */		ld	[%o0+%i2],%o0 ! volatile

!  520		      !	    /* If we found one, exit the loop */
!  521		      !	    if (inpkt) break;

/* 0x008c	 521 */		cmp	%i0,0
/* 0x0090	     */		bne	.L77000135
/* 0x0094	     */		cmp	%i5,0
                       .L77000129:

!  523		      !	    /* No packet.  Wait a while (if possible).  If we do this
!  524		      !	       several times without reading a packet, try to drain the
!  525		      !	       incoming queues 
!  526		      !	     */
!  527		      !#ifdef MPID_DEBUG_ALL
!  528		      !	    if (!freecnt) {
!  529		      !		    MPID_TRACE_CODE("No freePkt",MPID_myid);
!  530		      !		}
!  531		      !#endif
!  532		      !	    /* If not blocking, just return a null packet.  Not used 
!  533		      !	       currently (?) */
!  534		      !	    if (nonblock) return(inpkt);

/* 0x0098	 534 */		bne	.L77000136
/* 0x009c	     */		nop
                       .L77000131:

!  535		      !	    freecnt++;
!  536		      !	    p2p_yield();

/* 0x00a0	 536 */		call	p2p_yield,0	! Result = %g0
/* 0x00a4	     */		add	%i4,1,%i4

!  537		      !	    if ((freecnt % 8) == 0) {

/* 0x00a8	 537 */		sra	%i4,31,%o0
/* 0x00ac	     */		and	%o0,7,%o0
/* 0x00b0	     */		add	%i4,%o0,%o0
/* 0x00b4	     */		andn	%o0,7,%o0
/* 0x00b8	     */		subcc	%i4,%o0,%g0
/* 0x00bc	     */		bne,a	.L900000908
/* 0x00c0	     */		ld	[%i1],%o0 ! volatile
                       .L77000132:

!  538		      !		MPID_DeviceCheck( MPID_NOTBLOCKING );

/* 0x00c4	 538 */		call	MPID_DeviceCheck,1	! Result = %g0
/* 0x00c8	     */		or	%g0,0,%o0

!  539		      !		/* Return the packets that we have */
!  540		      !		MPID_SHMEM_FlushPkts();

/* 0x00cc	 540 */		call	MPID_SHMEM_FlushPkts,0	! Result = %g0
/* 0x00d0	     */		nop
/* 0x00d4	     */		ba	.L900000908
/* 0x00d8	     */		ld	[%i1],%o0 ! volatile
                       .L77000135:

!  541		      !	    }
!  543		      !        }
!  544		      !    }
!  545		      !    localavail	 = inpkt->head.next;

/* 0x00dc	 545 */		ld	[%i0+4],%o0 ! volatile
                       .L900000909:
/* 0x00e0	 545 */		sethi	%hi(.L644),%o1
/* 0x00e4	     */		st	%o0,[%o1+%lo(.L644)] ! volatile

!  546		      !    inpkt->head.next = 0;

/* 0x00e8	 546 */		st	%g0,[%i0+4] ! volatile
                       .L77000136:
/* 0x00ec	     */		ret
/* 0x00f0	     */		restore	%g0,%g0,%g0
/* 0x00f4	   0 */		.type	MPID_SHMEM_GetSendPkt,2
/* 0x00f4	     */		.size	MPID_SHMEM_GetSendPkt,(.-MPID_SHMEM_GetSendPkt)

	.section	".text",#alloc,#execinstr
/* 000000	   0 */		.align	4
!
! SUBROUTINE MPID_SHMEM_SendControl
!
! OFFSET    SOURCE LINE	LABEL	INSTRUCTION

                       	.global MPID_SHMEM_SendControl
                       MPID_SHMEM_SendControl:
/* 000000	     */		save	%sp,-96,%sp
/* 0x0004	     */		sethi	%hi(.L_mcount_counters+36),%o0
/* 0x0008	     */		call	_mcount,1	! Result = %g0
/* 0x000c	     */		add	%o0,%lo(.L_mcount_counters+36),%o0

!  548		      !    MPID_TRACE_CODE_PKT("Allocsendpkt",-1,inpkt->head.mode);
!  550		      !    return inpkt;
!  551		      !}
!  553		      !int MPID_SHMEM_SendControl( pkt, size, dest )
!  554		      !MPID_PKT_T *pkt;
!  555		      !int        size, dest;
!  556		      !{
!  557		      !    MPID_PKT_T *tail;
!  559		      !/* Place the actual length into the packet */
!  560		      !    MPID_TRACE_CODE_PKT("Sendpkt",dest,pkt->head.mode);
!  562		      !    pkt->head.src     = MPID_myid;

/* 0x0010	 562 */		sethi	%hi(MPID_myid),%o0

!  563		      !    pkt->head.next    = 0;           /* Should already be true */
!  565		      !    p2p_lock( MPID_lshmem.incominglockPtr[dest] );

/* 0x0014	 565 */		sll	%i2,2,%i2
/* 0x0018	 562 */		ld	[%o0+%lo(MPID_myid)],%o0 ! volatile
/* 0x001c	     */		st	%o0,[%i0+12] ! volatile
/* 0x0020	 565 */		sethi	%hi(MPID_lshmem+128),%o0
/* 0x0024	     */		add	%o0,%lo(MPID_lshmem+128),%o0
/* 0x0028	     */		add	%i2,%o0,%i1
/* 0x002c	 563 */		st	%g0,[%i0+4] ! volatile
/* 0x0030	 565 */		call	mutex_lock,1	! Result = %g0
/* 0x0034	     */		ld	[%i1],%o0 ! volatile

!  566		      !    tail = (MPID_PKT_T *)MPID_lshmem.incomingPtr[dest]->tail;

/* 0x0038	 566 */		sethi	%hi(MPID_lshmem+256),%o0
/* 0x003c	     */		add	%o0,%lo(MPID_lshmem+256),%o0
/* 0x0040	     */		add	%i2,%o0,%o1
/* 0x0044	     */		ld	[%o1],%o0 ! volatile
/* 0x0048	     */		ld	[%o0+4],%o0 ! volatile

!  567		      !    if (tail) 

/* 0x004c	 567 */		cmp	%o0,0
/* 0x0050	     */		be,a	.L900001008
/* 0x0054	     */		ld	[%o1],%o0 ! volatile
                       .L77000141:

!  568		      !	tail->head.next = pkt;

/* 0x0058	 568 */		st	%i0,[%o0+4] ! volatile
/* 0x005c	     */		ba	.L900001007
/* 0x0060	     */		ld	[%o1],%o0 ! volatile
                       .L900001008:

!  569		      !    else
!  570		      !	MPID_lshmem.incomingPtr[dest]->head = pkt;

/* 0x0064	 570 */		st	%i0,[%o0] ! volatile

!  572		      !    MPID_lshmem.incomingPtr[dest]->tail = pkt;

/* 0x0068	 572 */		ld	[%o1],%o0 ! volatile
                       .L900001007:
/* 0x006c	 572 */		st	%i0,[%o0+4] ! volatile

!  573		      !    p2p_unlock( MPID_lshmem.incominglockPtr[dest] );

/* 0x0070	 573 */		call	mutex_unlock,1	! Result = %g0
/* 0x0074	     */		ld	[%i1],%o0 ! volatile
/* 0x0078	     */		ret
/* 0x007c	     */		restore	%g0,0,%o0
/* 0x0080	   0 */		.type	MPID_SHMEM_SendControl,2
/* 0x0080	     */		.size	MPID_SHMEM_SendControl,(.-MPID_SHMEM_SendControl)

	.section	".text",#alloc,#execinstr
/* 000000	   0 */		.align	4
!
! SUBROUTINE MPID_SetupGetAddress
!
! OFFSET    SOURCE LINE	LABEL	INSTRUCTION

                       	.global MPID_SetupGetAddress
                       MPID_SetupGetAddress:
/* 000000	     */		save	%sp,-96,%sp
/* 0x0004	     */		sethi	%hi(.L_mcount_counters+40),%o0
/* 0x0008	     */		call	_mcount,1	! Result = %g0
/* 0x000c	     */		add	%o0,%lo(.L_mcount_counters+40),%o0
/* 0x0010	     */		or	%g0,%i1,%i2

!  575		      !    return MPI_SUCCESS;
!  576		      !}
!  578		      !/* 
!  579		      !   Return the address the destination (dest) should use for getting the 
!  580		      !   data at in_addr.  len is INOUT; it starts as the length of the data
!  581		      !   but is returned as the length available, incase all of the data can 
!  582		      !   not be transfered 
!  583		      ! */
!  584		      !void * MPID_SetupGetAddress( in_addr, len, dest )
!  585		      !void *in_addr;
!  586		      !int  *len, dest;
!  587		      !{
!  588		      !    void *new;
!  589		      !    int  tlen = *len;

/* 0x0014	 589 */		ld	[%i1],%i1 ! volatile

!  591		      !    MPID_TRACE_CODE("Allocating shared space",len);
!  592		      !/* To test, just comment out the first line and set new to null */
!  593		      !    new = p2p_shmalloc( tlen );

/* 0x0018	 593 */		call	p2p_shmalloc,1	! Result = %o0
/* 0x001c	     */		or	%g0,%i1,%o0
/* 0x0020	     */		orcc	%g0,%o0,%i0

!  594		      !/* new = 0; */
!  595		      !    if (!new) {

/* 0x0024	 595 */		bne	.L77000158
/* 0x0028	     */		nop
                       .L77000147:

!  596		      !	tlen = tlen / 2; 

/* 0x002c	 596 */		srl	%i1,31,%o0
/* 0x0030	     */		add	%i1,%o0,%o0
/* 0x0034	     */		sra	%o0,1,%o0
/* 0x0038	     */		orcc	%g0,%o0,%i1

!  597		      !	while(tlen > 0 && !(new = p2p_shmalloc(tlen))) 

/* 0x003c	 597 */		ble	.L900001109
/* 0x0040	     */		cmp	%i1,0
                       .L77000148:
/* 0x0044	     */		call	p2p_shmalloc,1	! Result = %o0
/* 0x0048	     */		nop
/* 0x004c	     */		orcc	%g0,%o0,%i0
/* 0x0050	     */		bne	.L77000154
/* 0x0054	     */		srl	%i1,31,%o0
                       .L900001108:

!  598		      !	    tlen = tlen / 2;

/* 0x0058	 598 */		add	%i1,%o0,%o0
/* 0x005c	     */		sra	%o0,1,%o0
/* 0x0060	     */		orcc	%g0,%o0,%i1
/* 0x0064	     */		ble	.L77000154
/* 0x0068	     */		nop
                       .L77000151:
/* 0x006c	     */		call	p2p_shmalloc,1	! Result = %o0
/* 0x0070	     */		nop
/* 0x0074	     */		orcc	%g0,%o0,%i0
/* 0x0078	     */		be	.L900001108
/* 0x007c	     */		srl	%i1,31,%o0
                       .L77000154:

!  599		      !	if (tlen == 0) {

/* 0x0080	 599 */		cmp	%i1,0
                       .L900001109:
/* 0x0084	 599 */		be	.L900001107
/* 0x0088	     */		sethi	%hi(_iob+32),%o0
                       .L77000156:

!  600		      !	    fprintf( stderr, "Could not get any shared memory for long message!" );
!  601		      !	    exit(1);
!  602		      !	}
!  603		      !	/* fprintf( stderr, "Message too long; sending partial data\n" ); */
!  604		      !	*len = tlen;

/* 0x008c	 604 */		st	%i1,[%i2] ! volatile
                       .L77000158:
/* 0x0090	     */		ret
/* 0x0094	     */		restore	%g0,%g0,%g0
                       .L900001107:
/* 0x0098	 600 */		add	%o0,%lo(_iob+32),%o0
/* 0x009c	     */		sethi	%hi(.L681),%o1
/* 0x00a0	     */		call	fprintf,2	! Result = %g0
/* 0x00a4	     */		add	%o1,%lo(.L681),%o1
/* 0x00a8	 601 */		call	exit,1	! Result = %g0
/* 0x00ac	     */		or	%g0,1,%o0
/* 0x00b0	     */		ba	.L77000158
/* 0x00b4	     */		st	%i1,[%i2] ! volatile
/* 0x00b8	   0 */		.type	MPID_SetupGetAddress,2
/* 0x00b8	     */		.size	MPID_SetupGetAddress,(.-MPID_SetupGetAddress)

	.section	".text",#alloc,#execinstr
/* 000000	   0 */		.align	4
!
! SUBROUTINE MPID_FreeGetAddress
!
! OFFSET    SOURCE LINE	LABEL	INSTRUCTION

                       	.global MPID_FreeGetAddress
                       MPID_FreeGetAddress:
/* 000000	     */		save	%sp,-96,%sp
/* 0x0004	     */		sethi	%hi(.L_mcount_counters+44),%o0
/* 0x0008	     */		call	_mcount,1	! Result = %g0
/* 0x000c	     */		add	%o0,%lo(.L_mcount_counters+44),%o0

!  605		      !    }
!  606		      !#ifdef FOO
!  607		      !/* If this mapped the address space, we wouldn't need to copy anywhere */
!  608		      !/*
!  609		      !if (MPID_DEBUG_FILE) {
!  610		      !    fprintf( MPID_DEBUG_FILE, 
!  611		      !	    "[%d]R About to copy to %d from %d (n=%d) (%s:%d)...\n", 
!  612		      !	    MPID_MyWorldRank, new, in_addr, tlen, 
!  613		      !	    __FILE__, __LINE__ );
!  614		      !    fflush( MPID_DEBUG_FILE );
!  615		      !    }
!  616		      ! */
!  618		      !    MEMCPY( new, in_addr, tlen );
!  619		      !#endif
!  621		      !    MPID_TRACE_CODE("Allocated space at",(long)new );
!  622		      !    return new;
!  623		      !}
!  625		      !void MPID_FreeGetAddress( addr )
!  626		      !void *addr;
!  627		      !{
!  628		      !    MPID_TRACE_CODE("Freeing space at",(long)addr );
!  629		      !    p2p_shfree( addr );

/* 0x0010	 629 */		call	p2p_shfree,1	! Result = %g0	! (tail call)
/* 0x0014	     */		restore	%g0,%g0,%g0
                       .L77000163:
/* 0x0018	     */		ret
/* 0x001c	     */		restore	%g0,%g0,%g0
/* 0x0020	   0 */		.type	MPID_FreeGetAddress,2
/* 0x0020	     */		.size	MPID_FreeGetAddress,(.-MPID_FreeGetAddress)

	.section	".bss",#alloc,#write
	.align	4
.L_mcount_counters:
	.skip	48

! Begin Disassembling Stabs
	.xstabs	".stab.index","Xs ; V=3.1 ; R=SC4.0 18 Oct 1995 C 4.0",60,0,0,0	! (/tmp/acomp.10843.2.s:1)
	.xstabs	".stab.index","/Net/garcon/garcon3/MPI/mpich/mpid/ch_shmem; /opt/SUNWspro/SC4.0/bin/acc -Xs -YP,:/usr/ucblib:/opt/SUNWspro/SC4.0/bin/../lib:/opt/SUNWspro/SC4.0/bin:/usr/ccs/lib:/usr/lib -pg -O -DMPID_NO_FORTRAN -DMPE_USE_EXTENSIONS=\'1\' -DHAVE_PROTOTYPES=\'1\' -DHAVE_NO_C_CONST=\'1\' -DSTDC_HEADERS=\'1\' -DHAVE_STDLIB_H=\'1\' -DHAVE_UNISTD_H=\'1\' -DHAVE_STDARG_H=\'1\' -DMALLOC_RET_VOID=\'1\' -DHAVE_SYSTEM=\'1\' -DHAVE_NICE=\'1\' -DHAVE_MEMORY_H=\'1\' -DHAVE_STRING_H=\'1\' -DHAVE_LONG_DOUBLE=\'1\' -DHAVE_LONG_LONG_INT=\'1\' -DMPI_ADI2 -I../../mpid/ch2 -I../.. -I../../include -I./ -I../ch2 -I../util -DMPI_solaris -DMPID_DEVICE_CODE -DHAVE_UNAME=\'1\' -DHAVE_GETHOSTBYNAME=\'1\' -DMPID_DEBUG_NONE -DMPID_STAT_NONE -DHAVE_MMAP=\'1\' -DHAVE_MUTEX_INIT=\'1\' -DHAVE_SHMAT=\'1\' -DHAVE_SEMOP=\'1\' -DHAVE_SIGACTION=\'1\' -DHAVE_SIGNAL=\'1\' -DHAVE_SIGSET=\'1\' -DRETSIGTYPE=\'void\' -DHAVE_GETTIMEOFDAY=\'1\' -c -S -I/usr/ucbinclude  -c shmempriv.c -Qoption acomp -xp",52,0,0,0	! (/tmp/acomp.10843.2.s:2)
! End Disassembling Stabs

! Begin Disassembling Ident
	.ident	"@(#)stdio.h\t1.7\t95/06/08 SMI"	! (/tmp/acomp.10843.2.s:4)
	.ident	"@(#)va_list.h\t1.6\t96/01/26 SMI"	! (/tmp/acomp.10843.2.s:5)
	.ident	"@(#)unistd.h\t1.4\t92/12/15 SMI"	! (/tmp/acomp.10843.2.s:6)
	.ident	"@(#)fcntl.h\t1.1\t90/04/27 SMI"	! (/tmp/acomp.10843.2.s:7)
	.ident	"@(#)types.h\t1.10\t93/07/21 SMI"	! (/tmp/acomp.10843.2.s:8)
	.ident	"@(#)select.h\t1.10\t92/07/14 SMI"	! (/tmp/acomp.10843.2.s:9)
	.ident	"@(#)time.h\t2.47\t95/08/24 SMI"	! (/tmp/acomp.10843.2.s:10)
	.ident	"@(#)feature_tests.h\t1.7\t94/12/06 SMI"	! (/tmp/acomp.10843.2.s:11)
	.ident	"@(#)types.h\t1.10\t93/07/21 SMI"	! (/tmp/acomp.10843.2.s:12)
	.ident	"@(#)time.h\t1.23\t95/08/28 SMI"	! (/tmp/acomp.10843.2.s:13)
	.ident	"@(#)sysmacros.h\t1.1\t90/04/27 SMI"	! (/tmp/acomp.10843.2.s:14)
	.ident	"@(#)mman.h\t1.19\t95/11/10 SMI"	! (/tmp/acomp.10843.2.s:15)
	.ident	"@(#)types.h\t1.10\t93/07/21 SMI"	! (/tmp/acomp.10843.2.s:16)
	.ident	"@(#)systeminfo.h\t1.14\t93/06/11 SMI"	! (/tmp/acomp.10843.2.s:17)
	.ident	"@(#)processor.h\t1.4\t94/11/11 SMI"	! (/tmp/acomp.10843.2.s:18)
	.ident	"@(#)types.h\t1.10\t93/07/21 SMI"	! (/tmp/acomp.10843.2.s:19)
	.ident	"@(#)procset.h\t1.15\t93/05/05 SMI"	! (/tmp/acomp.10843.2.s:20)
	.ident	"@(#)synch.h\t1.31\t95/08/24 SMI"	! (/tmp/acomp.10843.2.s:21)
	.ident	"@(#)machlock.h\t1.14\t94/10/20 SMI"	! (/tmp/acomp.10843.2.s:22)
	.ident	"@(#)types.h\t1.10\t93/07/21 SMI"	! (/tmp/acomp.10843.2.s:23)
	.ident	"@(#)synch.h\t1.21\t93/04/13 SMI"	! (/tmp/acomp.10843.2.s:24)
	.ident	"@(#)types.h\t1.10\t93/07/21 SMI"	! (/tmp/acomp.10843.2.s:25)
	.ident	"acomp: SC4.0 18 Oct 1995 C 4.0"	! (/tmp/acomp.10843.2.s:84)
! End Disassembling Ident
