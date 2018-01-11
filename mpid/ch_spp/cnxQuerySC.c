#include <stdio.h>
#include <stdlib.h>
#include <sys/cnx_pattr.h>
#include <sys/cnx_sysinfo.h>

static cnx_scid_t getSubcomplexId()
{
  struct cnx_pattributes pa;

  if (cnx_getpattr(getpid(), CNX_PATTR_SCID, &pa) == -1)
  {
    perror("cnx_getpattr");
    exit(-1);
  }
  return pa.pattr_scid;
}

static unsigned int getNumNodesCpusInSubcomplex(cnx_scid_t myScId,
              unsigned int *numNodes, unsigned int *totalCPUs)
{
  cnx_is_target_data_t target;
  cnx_is_sc_basic_info_data_t scBasicInfo;

  cnx_sysinfo_target_subcomplex(&target, myScId);
  if (cnx_sysinfo(CNX_IS_SC_BASIC_INFO, &target, &scBasicInfo,
                  1, CNX_IS_SC_BASIC_INFO_COUNT, NULL) == -1)
  {
    perror("cnx_sysinfo");
    exit(-1);
  }
  *numNodes = scBasicInfo.node_count;
  *totalCPUs = scBasicInfo.cpu_count;
  return 0;
}

static void getNumCPUsPerNode(cnx_scid_t myScId, unsigned int numNodes,
		       unsigned int *numCPUs)
{
  cnx_is_target_data_t target;
  cnx_is_scnode_basic_info_data_t *scNodeBasicInfo;
  int i;

  cnx_sysinfo_target_scnode(&target, myScId, CNX_IS_ALL_NODES);
  scNodeBasicInfo = (cnx_is_scnode_basic_info_data_t *)
	    malloc(numNodes * sizeof(cnx_is_scnode_basic_info_data_t));
  if (cnx_sysinfo(CNX_IS_SCNODE_BASIC_INFO, &target, scNodeBasicInfo,
		  numNodes, CNX_IS_SCNODE_BASIC_INFO_COUNT, NULL) == -1)
  {
    perror("cnx_sysinfo");
   exit(-1);
  }
  for (i=0; i < numNodes; i++)
  {
    numCPUs[i] = scNodeBasicInfo[i].num_cpus;
  }
  free(scNodeBasicInfo);
}

cnx_node_t MPID_SPP_getNodeId()
{
  cnx_is_target_data_t target;
  cnx_is_thread_location_info_data_t threadLocInfo;

  cnx_sysinfo_target_thread(&target, getpid(), cnx_thread_self());
  if (cnx_sysinfo(CNX_IS_THREAD_LOCATION_INFO, &target, &threadLocInfo,
		  1, CNX_IS_THREAD_LOCATION_INFO_COUNT, NULL) == -1)
  {
    perror("cnx_sysinfo");
    exit(-1);
  }
  return threadLocInfo.node;
}

int MPID_SPP_getSCTopology(cnx_node_t *myNode, unsigned int *numNodes, unsigned int *totalCPUs, unsigned int *numCPUs)
{
  cnx_scid_t myScId;

  myScId = getSubcomplexId();
/*   printf("subcomplexid: %d\n", myScId); */
  *myNode = MPID_SPP_getNodeId();
  getNumNodesCpusInSubcomplex(myScId, numNodes, totalCPUs);
  getNumCPUsPerNode(myScId, *numNodes, numCPUs);
}

static void displayTopologyProblem(int numNodes, int *numCPUs,
			    int reqNumNodes, int *reqNumCPUs,
			    int allowOversub)
{
  int i;

  printf("subcomplex topology: -");
  for (i = 0; i < numNodes; i++)
    printf(" %d -", numCPUs[i]);
  printf("\nrequested topology:  -");
  for (i = 0; i < reqNumNodes; i++)
    printf(" %d -", reqNumCPUs[i]);
  printf("\n");
  if (numNodes < reqNumNodes)
    fprintf(stderr, "%d nodes requested, the current subcomplex has %d\n",
                    reqNumNodes, numNodes);
  if (!allowOversub)
    for (i = 0; i < reqNumNodes; i++)
      if (numCPUs[i] < reqNumCPUs[i])
        fprintf(stderr, "%d cpus requested on node %d, this node has %d\n",
               reqNumCPUs[i], i, numCPUs[i]);
}

void MPID_SPP_processTopologyInfo(char *envVarBuf, int *np, int numNodes,
                         int *numCPUs, int allowOversub)
{
  int i;
  int reqNumNodes, reqNumCPUs[8], reqNumProcs;
  int badTopology;

  reqNumNodes = sscanf(envVarBuf, "%d,%d,%d,%d,%d,%d,%d,%d",
		reqNumCPUs,   reqNumCPUs+1, reqNumCPUs+2, reqNumCPUs+3, 
	        reqNumCPUs+4, reqNumCPUs+5, reqNumCPUs+6, reqNumCPUs+7);
  for (reqNumProcs = 0, i = 0; i < reqNumNodes; i++)
    reqNumProcs += reqNumCPUs[i];
  if (*np && *np != reqNumProcs)
  {
    fprintf(stderr, "specified %d processes with -np command line option\n\
and %d processes via the MPI_TOPOLOGY environmental variable\n\
please reconcile and rerun\n", np, reqNumProcs);
    exit(-1);
  }
  *np = reqNumProcs;
  badTopology = 0;
  if (numNodes < reqNumNodes)
    badTopology = 1;
  if (!allowOversub)
    for (i = 0; i < reqNumNodes; i++)
      if (numCPUs[i] < reqNumCPUs[i])
        badTopology = 1;
  if (badTopology)
  {
    displayTopologyProblem(numNodes, numCPUs, reqNumNodes,
			    reqNumCPUs, allowOversub);
    fprintf(stderr, "- quitting\n");
    exit(-1);
  }
  for (i = 0; i < reqNumNodes; i++)
    numCPUs[i] = reqNumCPUs[i];
  while (i < numNodes)
    numCPUs[i++] = 0;
}
