/* 
 * rcsid = "$Header: /nfs/globus1/src/master/nexus_source/dbgstate.h,v 1.5 1996/10/07 04:39:53 tuecke Exp $"
 *
 */
typedef struct _nexus_debug_state_t
{
    unsigned long catagory;
    unsigned long module;
    unsigned long operation;
    unsigned long level;
} nexus_debug_state_t;


#define NXDBG_NONE	0x00000000
#define NXDBG_ALL	0xffffffff
#define NXDBG_USER	0x00000001
#define NXDBG_CORE	0x00000002
#define NXDBG_PR	0x00000004
#define NXDBG_ST	0x00000008
#define NXDBG_DB	0x00000010
#define NXDBG_TH	0x00000020

/* CORE modules */
#define NXDBG_C_INIT		0x00000001
#define NXDBG_C_NODELOCK	0x00000002
#define NXDBG_C_PROCESS		0x00000004
#define NXDBG_C_CONTEXT		0x00000008
#define NXDBG_C_HANDLER		0x00000010

/* PR modules */
#define NXDBG_PR_IFACE		0x00000001
#define NXDBG_PR_LOCAL		0x00000002
#define NXDBG_PR_TCP		0x00000004
#define NXDBG_PR_MPL		0x00000008
#define NXDBG_PR_INX		0x00000010
#define NXDBG_PR_RX		0x00000020
#define NXDBG_PR_MPINX		0x00000040

/* ST modules */
#define NXDBG_ST_IFACE		0x00000001
#define NXDBG_ST_FORK		0x00000002
#define NXDBG_ST_RSH		0x00000004
#define NXDBG_ST_IBMDS		0x00000008
#define NXDBG_ST_INX		0x00000010
#define NXDBG_ST_MPINX		0x00000020
#define NXDBG_ST_MPL		0x00000040
#define NXDBG_ST_SOLDL		0x00000080
#define NXDBG_ST_SS		0x00000100
#define NXDBG_ST_NS		0x00000200

/* DB modules */
#define NXDBG_DB_IFACE		0x00000001
#define NXDBG_DB_FILE		0x00000002

/* TH modules */
