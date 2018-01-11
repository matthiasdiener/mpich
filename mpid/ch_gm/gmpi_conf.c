/*************************************************************************
 * Myricom MPICH-GM ch_gm backend                                        *
 * Copyright (c) 2001 by Myricom, Inc.                                   *
 * All rights reserved.                                                  *
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef WIN32
#define strcasecmp stricmp
#endif

#define GM_STRONG_TYPES 0

#include "gmpi.h"
#include "gmpi_smpi.h"
#include "mpiddev.h"
#include "mpimem.h"
#include "bnr.h"


static void 
gmpi_getenv (const char *varenv, char **result, char *msg, 
	     unsigned int required)
{
  *result = (char *) getenv (varenv);
  if ((required == 1) && (*result == NULL))
    {
      fprintf (stderr, "<MPICH-GM> Error: Need to obtain %s in %s !\n",
	       msg, varenv);
      gmpi_abort (0);
    }
}


static void
gmpi_allocate_port (int *board_number, unsigned int *port_number)
{
  unsigned int board_id, port_id, count;
  gm_status_t status;
  
  /* do we know which board to use ? */
  if (*board_number < 0)
    { 
      for (count = 0; count < 3; count++)
	{
	  for (port_id = 2; port_id < GMPI_MAX_GM_PORTS; port_id++)
	    {
	      if (port_id != 3)
		{
		  for (board_id = 0; board_id < GMPI_MAX_GM_BOARDS; board_id++)
		    {
		      status = gm_open (&gmpi_gm_port, board_id, port_id, 
					"MPICH-GM", GM_API_VERSION);
		      if (status == GM_SUCCESS)
			{
			  *board_number = board_id;
			  *port_number = port_id;
			  return;
			}
		    }
		}
	    }
	}
    }
  else
    {
      /* open a port on the allocated board */
      board_id = *board_number;
      for (count = 0; count < 3; count++)
	{
	  for (port_id = 2; port_id < GMPI_MAX_GM_PORTS; port_id++)
	    {
	      if (port_id != 3)
		{
		  status = gm_open (&gmpi_gm_port, board_id, port_id, 
				    "MPICH-GM", GM_API_VERSION);
		  if (status == GM_SUCCESS)
		    {
		      *port_number = port_id;
		      return;
		    }
		}
	    }
	}
    }
}


static void
gmpi_allocate_world (unsigned int size)
{
  gmpi.node_ids = (unsigned int *) calloc (size, sizeof (unsigned int));
  gmpi.mpi_pids = (unsigned int *) calloc (size, sizeof (unsigned int));
  gmpi.port_ids = (unsigned int *) calloc (size, sizeof (unsigned int));
  gmpi.board_ids = (unsigned int *) calloc (size, sizeof (unsigned int));
  gmpi.pending_sends = (unsigned int *) calloc (size, sizeof (unsigned int));
  gmpi.dropped_sends = (unsigned int *) calloc (size, sizeof (unsigned int));
  gmpi.host_names = (char **) calloc (size, sizeof (char *));
  gmpi.exec_names = (char **) calloc (size, sizeof (char *));
  
  gmpi_malloc_assert (gmpi.node_ids, "gmpi_getconf", "malloc: node_ids");
  gmpi_malloc_assert (gmpi.mpi_pids, "gmpi_getconf", "malloc: mpi_pids");
  gmpi_malloc_assert (gmpi.port_ids, "gmpi_getconf", "malloc: port_ids");
  gmpi_malloc_assert (gmpi.board_ids, "gmpi_getconf", "malloc: board_ids");
  gmpi_malloc_assert (gmpi.pending_sends, "gmpi_getconf", 
		      "malloc: pending_sends");
  gmpi_malloc_assert (gmpi.dropped_sends, "gmpi_getconf", 
		      "malloc: dropped_sends");
  gmpi_malloc_assert (gmpi.host_names, "gmpi_getconf", "malloc: host_names");
  gmpi_malloc_assert (gmpi.exec_names, "gmpi_getconf", "malloc: exec_names");
}


#define GMPI_SOCKET_BUFFER_SIZE 128*1024

static void
gmpi_getconf (void)
{
  char *gmpi_eager, *gmpi_shmem, *gmpi_recvmode;
  unsigned int i, j, port_id;
  int board_id;
  
  setbuf (stdout, NULL);
  setbuf (stderr, NULL);

  if (getenv ("MAN_MPD_FD") != NULL)
    {
      char attr_buffer[BNR_MAXATTRLEN];
      char val_buffer[BNR_MAXVALLEN];
      /* MPD to spawn processes */
      char my_hostname[256];
      char *hostnames;
      BNR_Group bnr_group;
      
      /* MPD to spawn processes */
      gmpi.mpd = 1;
      i = BNR_Init ();
      i = BNR_Get_group (&bnr_group);
      i = BNR_Get_rank (bnr_group, &MPID_MyWorldRank);
      i = BNR_Get_size (bnr_group, &MPID_MyWorldSize);
      
      /* data allocation */
      gmpi_allocate_world (MPID_MyWorldSize);
      hostnames = (char *) malloc (MPID_MyWorldSize * 256 * sizeof (char *));
      gmpi_malloc_assert (hostnames, "gmpi_getconf", "malloc: hostnames");

      /* open a port */
      board_id = -1;
      port_id = 0;
      gmpi_allocate_port (&board_id, &port_id);
      if (port_id == 0)
	{
	  fprintf (stderr, "[%d] Error: Unable to open a GM port !\n", 
		   MPID_MyWorldRank);
	  gmpi_abort (0);
	}
      
      /* get the GM node id */
      if (gm_get_node_id (gmpi_gm_port, &(gmpi.my_node_id)) != GM_SUCCESS)
	{
	  fprintf (stderr, "[%d] Error: Unable to get GM local node id !\n", 
		   MPID_MyWorldRank);
	  gmpi_abort (0);
	}
  
      /* build the data to send to master */
      gm_bzero (val_buffer, BNR_MAXVALLEN * sizeof (char));
      gm_get_host_name (gmpi_gm_port, my_hostname);
      sprintf (val_buffer, "< %d:%d:%d:%s >\n", port_id, board_id, 
	       gmpi.my_node_id, my_hostname);
      
      /* put our information */
      gm_bzero (attr_buffer, BNR_MAXATTRLEN * sizeof (char));
      sprintf (attr_buffer, "MPICH-GM data [%d]\n", MPID_MyWorldRank);
      i = BNR_Put (bnr_group, attr_buffer, val_buffer, -1);
      
      /* get other processes data */
      i = BNR_Fence (bnr_group);
      for (j = 0; j < MPID_MyWorldSize; j++)
	{
	  gm_bzero (attr_buffer, BNR_MAXATTRLEN * sizeof (char));
	  sprintf (attr_buffer, "MPICH-GM data [%d]\n", j);
	  i = BNR_Get (bnr_group, attr_buffer, val_buffer);
	  
	  /* decrypt data */
	  if (sscanf (val_buffer, "< %d:%d:%d:%s >", &(gmpi.port_ids[j]), 
		      &(gmpi.board_ids[j]), &(gmpi.node_ids[j]), 
		      &(hostnames[j*256])) != 4)
	    {
	      fprintf (stderr, "[%d] Error: unable to decode data "
		       "from %d !\n", MPID_MyWorldRank, j);
	      gmpi_abort (0);
	    }
	}
      
      /* compute the local mapping */
      smpi.num_local_nodes = 0;
      for (j = 0; j < MPID_MyWorldSize; j++)
	{
	  if (strcmp (my_hostname, &(hostnames[j*256])) == 0)
	    {
	      if (j == MPID_MyWorldRank)
		{
		  smpi.my_local_id = smpi.num_local_nodes;
		}
	      smpi.local_nodes[j] = smpi.num_local_nodes;
	      smpi.num_local_nodes++;
	    }
	  else
	    {
	      smpi.local_nodes[j] = -1;
	    }
	}
      free (hostnames);
    }
  else
    {
      char *gmpi_magic, *gmpi_master, *gmpi_port1, *gmpi_port2;
      char *gmpi_id, *gmpi_np, *gmpi_board;
      char buffer[GMPI_SOCKET_BUFFER_SIZE];
      char temp[32];
      unsigned int count, magic_number, master_port1, master_port2;
      int gmpi_sockfd;
      struct hostent *master;
      
      /* mpirun with sockets */
      gmpi.mpd = 0;
      gmpi_getenv ("GMPI_MAGIC", &gmpi_magic, "the job magic number", 1);
      gmpi_getenv ("GMPI_MASTER", &gmpi_master, "the master's hostname", 1);
      gmpi_getenv ("GMPI_PORT1", &gmpi_port1, "the master's port 1 number", 1);
      gmpi_getenv ("GMPI_PORT2", &gmpi_port2, "the master's port 2 number", 1);
      gmpi_getenv ("GMPI_ID", &gmpi_id, "the MPI ID of the process", 1);
      gmpi_getenv ("GMPI_NP", &gmpi_np, "the number of MPI processes", 1);
      gmpi_getenv ("GMPI_BOARD", &gmpi_board, "the specified board", 1);
      
      if (sscanf (gmpi_magic, "%d", &magic_number) != 1)
	{
	  fprintf (stderr, "<MPICH-GM> Error: Bad magic number "
		   "(GMPI_MAGIC is %s) !\n", gmpi_magic);
	  gmpi_abort (0);
	}
      gmpi.magic = magic_number;
      
      if ((sscanf (gmpi_np, "%d", &MPID_MyWorldSize) != 1)
	  || (MPID_MyWorldSize < 0))
	{
	  fprintf (stderr, "<MPICH-GM> Error: Bad number of processes "
		   "(GMPI_NP is %s) !\n", gmpi_np);
	  gmpi_abort (0);
	}
      
      if ((sscanf (gmpi_id, "%d", &MPID_MyWorldRank) != 1)
	  || (MPID_MyWorldRank < 0) || (MPID_MyWorldRank >= MPID_MyWorldSize))
	{
	  fprintf (stderr, "<MPICH-GM> Error: Bad MPI ID "
		   "(GMPI_ID is %s, total number of MPI processes is %d) !\n", 
		   gmpi_np, MPID_MyWorldSize);
	  gmpi_abort (0);
	}
  
      if (sscanf (gmpi_port1, "%d", &master_port1) != 1)
	{
	  fprintf (stderr, "<MPICH-GM> Error: Bad master port 1 number "
		   "(GMPI_PORT1 is %s) !\n", gmpi_port1);
	  gmpi_abort (0);
	}

      if (sscanf (gmpi_port2, "%d", &master_port2) != 1)
	{
	  fprintf (stderr, "<MPICH-GM> Error: Bad master port 2 number "
		   "(GMPI_PORT2 is %s) !\n", gmpi_port2);
	  gmpi_abort (0);
	}

      if (sscanf (gmpi_board, "%d", &board_id) != 1)
	{
	  fprintf (stderr, "<MPICH-GM> Error: Bad board ID "
		   "(GMPI_BOARD is %s) !\n", gmpi_board);
	  gmpi_abort (0);
	}

      /* data allocation */
      gmpi_allocate_world (MPID_MyWorldSize);

      /* open a port */
      port_id = 0;
      gmpi_allocate_port (&board_id, &port_id);
      if (port_id == 0)
	{
	  fprintf (stderr, "[%d] Error: Unable to open a GM port !\n", 
		   MPID_MyWorldRank);
	  gmpi_abort (0);
	}

      /* get the GM node id */
      if (gm_get_node_id (gmpi_gm_port, &(gmpi.my_node_id)) != GM_SUCCESS)
	{
	  fprintf (stderr, "[%d] Error: Unable to get GM local node id !\n", 
		   MPID_MyWorldRank);
	  gmpi_abort (0);
	}
  
      /* get a socket */
      gmpi_sockfd = socket (AF_INET, SOCK_STREAM, 0);
      if (gmpi_sockfd < 0)
	{
	  fprintf (stderr, "[%d] Error: Unable to open a socket !\n", 
		   MPID_MyWorldRank);
	  gmpi_abort (0);
	}
  
      /* get the master's IP address */
      master = gethostbyname (gmpi_master);
      if (master == NULL)
	{ 
	  fprintf (stderr, "[%d] Error: Unable to translate "
		   "the hostname of the master (%s)!\n", 
		   MPID_MyWorldRank, gmpi_master);
	  gmpi_abort (0);
	}
  
      /* connect to the master */
      gm_bzero ((char *) (&(gmpi.master_addr)), sizeof (gmpi.master_addr));
      gmpi.master_addr.sin_family = AF_INET;
      gm_bcopy ((char *) (master->h_addr), 
		(char *) (&(gmpi.master_addr.sin_addr)),
		master->h_length);
      gmpi.master_addr.sin_port = htons (master_port1);
      i = GMPI_INIT_TIMEOUT;
      while (connect (gmpi_sockfd, (struct sockaddr *) (&(gmpi.master_addr)), 
		      sizeof (gmpi.master_addr)) < 0)
	{
	  usleep (1000);
	  if (i == 0)
	    {
	      fprintf (stderr, "[%d] Error: Unable to connect to "
		       "the master !\n", MPID_MyWorldRank);
	      gmpi_abort (0);
	    }
	  i--;
	}
  
      /* send the magic:ID:port:board:node used to the master */
      count = 0;
      sprintf (buffer, "<<<%d:%d:%d:%d:%d:%d>>>\n", magic_number, 
	       MPID_MyWorldRank, port_id, board_id, gmpi.my_node_id, 
	       (int) getpid ());
      while (count < strlen (buffer))
	{
	  i = write (gmpi_sockfd, &(buffer[count]), strlen (buffer) - count);
	  if (i < 0)
	    {
	      fprintf (stderr, "[%d] Error: write to socket failed !\n", 
		       MPID_MyWorldRank);
	      gmpi_abort (0);
	    }
	  count += i;
	}
      close (gmpi_sockfd);
      
      /* get another socket */
      gmpi_sockfd = socket (AF_INET, SOCK_STREAM, 0);
      if (gmpi_sockfd < 0)
	{
	  fprintf (stderr, "[%d] Error: Unable to open a socket (2)!\n", 
		   MPID_MyWorldRank);
	  gmpi_abort (0);
	}
      
      /* re-connect to the master */
      gmpi.master_addr.sin_port = htons (master_port2);
      i = GMPI_INIT_TIMEOUT;
      while (connect (gmpi_sockfd, (struct sockaddr *) (&(gmpi.master_addr)), 
		      sizeof (gmpi.master_addr)) < 0)
	{
	  usleep (1000);
	  if (i == 0)
	    {
	      fprintf (stderr, "[%d] Error: Unable to connect(2) to "
		       "the master !\n", MPID_MyWorldRank);
	      gmpi_abort (0);
	    }
	  i--;
	}
      
      /* send the a small authentification magic:ID to the master */
      count = 0;
      sprintf (buffer, "<->%d:%d<->\n", magic_number, MPID_MyWorldRank);
      while (count < strlen (buffer))
	{
	  i = write (gmpi_sockfd, &(buffer[count]), strlen (buffer) - count);
	  if (i < 0)
	    {
	      fprintf (stderr, "[%d] Error: write to socket (2) failed !\n", 
		       MPID_MyWorldRank);
	      gmpi_abort (0);
	    }
	  count += i;
	}

      /* Get the whole GM mapping from the master */
      count = 0;
      gm_bzero (buffer, GMPI_SOCKET_BUFFER_SIZE * sizeof(char));
      while (strstr (buffer, "]]]") == NULL)
	{
	  i = read (gmpi_sockfd, &(buffer[count]), 
		    GMPI_SOCKET_BUFFER_SIZE - count);
	  if (i < 0)
	    {
	      fprintf (stderr, "[%d] Error: read from socket failed !\n", 
		       MPID_MyWorldRank);
	      gmpi_abort (0);
	    }
	  count += i;
	}
      close (gmpi_sockfd);
  
      /* check the initial marker */
      j = 0;
      if (strncmp (&(buffer[j]), "[[[", 3) != 0)
	{
	  fprintf (stderr, "[%d] Error: bad format on data from master !\n",
		   MPID_MyWorldRank);
	  gmpi_abort (0);
	}

      /* Decrypt the global mapping */
      j += 3;
      for (i = 0; i < MPID_MyWorldSize; i++)
	{
	  if (sscanf (&(buffer[j]), "<%d:%d:%d>", &(gmpi.port_ids[i]), 
		      &(gmpi.board_ids[i]), &(gmpi.node_ids[i])) != 3)
	    {
	      fprintf (stderr, "[%d] Error: unable to decode data "
		       "from master !\n", MPID_MyWorldRank);
	      gmpi_abort (0);
	    }
      
	  sprintf (temp, "<%d:%d:%d>", gmpi.port_ids[i], gmpi.board_ids[i], 
		   gmpi.node_ids[i]);
	  j += strlen (temp);

	  smpi.local_nodes[i] = -1;
	}

      /* check the marker between global map and local map */  
      if (strncmp (&(buffer[j]), "|||", 3) != 0)
	{
	  fprintf (stderr, "[%d] Error: bad format on data from master !\n",
		   MPID_MyWorldRank);
	  gmpi_abort (0);
	}

      /* decrypt the local mapping */
      j += 3;
      smpi.num_local_nodes = 0;
      while (strncmp (&(buffer[j]), "]]]", 3) != 0)
	{ 
	  if (sscanf (&(buffer[j]), "<%d>", &i) != 1)
	    {
	      fprintf (stderr, "[%d] Error: unable to decode master data !\n",
		       MPID_MyWorldRank);
	      gmpi_abort (0);
	    }
      
	  if (i == MPID_MyWorldRank)
	    {
	      smpi.my_local_id = smpi.num_local_nodes;
	    }
	  smpi.local_nodes[i] = smpi.num_local_nodes;
	  smpi.num_local_nodes++;
      
	  sprintf (temp, "<%d>", i);
	  j += strlen (temp);
	}

      /* check size of the data from the master */
      j += 3;
      if (j != count)
	{
	  fprintf (stderr, "[%d] Error: amount of data from master !\n",
		   MPID_MyWorldRank);
	  gmpi_abort (0);
	}
    }

  /* check consistency */
  if ((gmpi.port_ids[MPID_MyWorldRank] != port_id) 
      || (gmpi.board_ids[MPID_MyWorldRank] != board_id)
      || (gmpi.node_ids[MPID_MyWorldRank] != gmpi.my_node_id))
    {
      fprintf (stderr, "[%d] Error: inconsistency in collected data !\n", 
	       MPID_MyWorldRank);
      gmpi_abort (0);
    }

  gmpi_getenv ("GMPI_EAGER", &gmpi_eager, NULL, 0);
  gmpi_getenv ("GMPI_SHMEM", &gmpi_shmem, NULL, 0);
  gmpi_getenv ("GMPI_RECV", &gmpi_recvmode, NULL, 0);

  /* Set the EAGER/Rendez-vous protocols threshold */
  if (gmpi_eager == NULL)
    {
      gmpi.eager_size = GMPI_EAGER_SIZE_DEFAULT;
    }
  else
    {
      gmpi.eager_size = strtoul(gmpi_eager, NULL, 10);
      if (gmpi.eager_size < 128)
	{
	  gmpi.eager_size = 128;
	}
      
      if (gmpi.eager_size > GMPI_MAX_PUT_LENGTH)
	{
	  gmpi.eager_size = GMPI_MAX_PUT_LENGTH;
	}
    }

  /* set the GM receive mode */
  if (gmpi_recvmode == NULL)
    {
      gmpi.gm_receive_mode = gm_receive;
    }
  else
    {
      if (strcasecmp (gmpi_recvmode, "polling") == 0)
	{
	  gmpi.gm_receive_mode = gm_receive;
	}
      else
	{
	  if (strcasecmp (gmpi_recvmode, "blocking") == 0)
	    {
	      gmpi.gm_receive_mode = gm_blocking_receive_no_spin;
	    }
	  else
	    {
	      if (strcasecmp (gmpi_recvmode, "hybrid") == 0)
		{
		  gmpi.gm_receive_mode = gm_blocking_receive;
		}
	      else
		{
		  gmpi.gm_receive_mode = gm_receive;
		}
	    }
	}
    }

  /* Set the shared memory support */
  if (gmpi.gm_receive_mode == gm_receive)
    {
      if (gmpi_shmem == NULL)
	{
	  gmpi.shmem = 1;
	}
      else
	{
	  if (strcmp (gmpi_shmem, "1") == 0) 
	    {
	      gmpi.shmem = 1;
	    }
	  else
	    {
	      gmpi.shmem = 0;
	    }
	}
    }
  else
    {
      gmpi.shmem = 0;
    }
}


/* This function fill the MPID_Config structure to describe 
   the multi-devices configuration */
MPID_Config *MPID_GetConfigInfo (int *argc, 
				 char ***argv, 
				 void *config, 
				 int  *error_code)
{
  MPID_Config * new_config = NULL;
  MPID_Config * return_config = NULL;
  int i, j;

  /* already configured ? */
  if (config != NULL)
      return (MPID_Config *)config;

  /* Get the GM mapping and the environnement variables */
  gmpi_getconf();

  /* at least one local node : myself ! */ 
  gmpi_debug_assert(smpi.num_local_nodes != 0);


  /* SELF DEVICE */
  /* if it's the first device, start the linked list of devices.
     Otherwise, add a new one at the end. */
  if (new_config == NULL)
    {
      new_config = (MPID_Config *)malloc(sizeof(MPID_Config));
      gmpi_malloc_assert(new_config,
			 "MPID_GetConfigInfo",
			 "malloc: Self device config");
      return_config = new_config;
    }
  else
    {
      new_config->next = (MPID_Config *)malloc(sizeof(MPID_Config));
      gmpi_malloc_assert(new_config->next,
			 "MPID_GetConfigInfo",
			 "malloc: Self device config");
      new_config = new_config->next;
    }
  /* we don't need to check if we need this device: we need this device ! */
  new_config->device_init = MPID_CH_InitSelfMsg;
  new_config->device_init_name = (char *)malloc(255*sizeof(char));
  gmpi_malloc_assert(new_config->device_init_name,
		     "MPID_GetConfigInfo",
		     "malloc: Self device name");
  sprintf(new_config->device_init_name, "Self device");
  new_config->num_served = 1;
  new_config->granks_served = (int *)malloc(sizeof(int));
  gmpi_malloc_assert(new_config->granks_served,
		     "MPID_GetConfigInfo",
		     "malloc: Self device map");
  new_config->granks_served[0] = MPID_MyWorldRank;
  new_config->next = NULL;
  
  
  /* SMP DEVICE */
  /* we don't need this device if there's only one process on this node */
  if (smpi.num_local_nodes > 1) 
    {
#if !GM_OS_VXWORKS
      if (gmpi.shmem == 1)
	{
	  if (gmpi.gm_receive_mode == gm_receive) 
	    {
	      if (new_config == NULL) 
		{
		  new_config = (MPID_Config *)malloc(sizeof(MPID_Config));
		  gmpi_malloc_assert(new_config,
				     "MPID_GetConfigInfo",
				     "malloc: SMP device config");
		  return_config = new_config;
		}
	      else 
		{
		  new_config->next = (MPID_Config *)
		    malloc(sizeof(MPID_Config));
		  gmpi_malloc_assert(new_config->next,
				     "MPID_GetConfigInfo",
				     "malloc: SMP device config");
		  new_config = new_config->next;
		}
	      new_config->device_init = MPID_SMP_InitMsgPass;
	      new_config->device_init_name = (char *)
		malloc(255 * sizeof(char));
	      gmpi_malloc_assert(new_config->device_init_name,
				 "MPID_GetConfigInfo",
				 "malloc: SMP device name");
	      sprintf(new_config->device_init_name, "SMP_plug device");
	      new_config->num_served = smpi.num_local_nodes - 1;
	      new_config->granks_served = (int *)
		malloc(new_config->num_served * sizeof(int));
	      gmpi_malloc_assert(new_config->granks_served,
				 "MPID_GetConfigInfo",
				 "malloc: SMP device map");
	      /* setup routes */
	      j = 0;
	      for(i=0; i<MPID_MyWorldSize; i++)
		{
		  if ((i!= MPID_MyWorldRank) 
		      && (smpi.local_nodes[i] != -1)) 
		    {
		      gmpi_debug_assert(smpi.local_nodes[i] 
					!= smpi.my_local_id);
		      new_config->granks_served[j] = i;
		      j++;
		    }
		}
	      gmpi_debug_assert(j == new_config->num_served);
	      new_config->next = NULL;
	    }
	}
      else
#endif
	{
	  smpi.num_local_nodes = 1;
	  for (i=0; i < MPID_MyWorldSize; i++)
	    {
	      if (i != MPID_MyWorldRank)
		{
		  smpi.local_nodes[i] = -1;
		}
	    }
	}
    }
  

  /* GM DEVICE */
  if (MPID_MyWorldSize > smpi.num_local_nodes)
    {
      if (new_config == NULL)
	{
	  new_config = (MPID_Config *)malloc(sizeof(MPID_Config));
	  gmpi_malloc_assert(new_config,
			     "MPID_GetConfigInfo",
			     "malloc: GM device config");
	  return_config = new_config;
	}
      else
	{
	  new_config->next = (MPID_Config *)malloc(sizeof(MPID_Config));
	  gmpi_malloc_assert(new_config->next,
			     "MPID_GetConfigInfo",
			     "malloc: GM device config");
	  new_config = new_config->next;
	}
      new_config->device_init = MPID_CH_InitMsgPass;
      new_config->device_init_name = (char *)malloc(255 * sizeof(char));
      gmpi_malloc_assert(new_config->device_init_name,
			 "MPID_GetConfigInfo",
			 "malloc: GM device name");
      sprintf(new_config->device_init_name, "Myricom GM device");
      new_config->num_served = MPID_MyWorldSize - smpi.num_local_nodes;
      new_config->granks_served = (int *)malloc(new_config->num_served 
						* sizeof(int));
      gmpi_malloc_assert(new_config->granks_served,
			 "MPID_GetConfigInfo",
			 "malloc: GM device map");
      /* setup the routes */
      j = 0;
      for(i=0; i<MPID_MyWorldSize; i++)
	if (smpi.local_nodes[i] == -1) {
	  new_config->granks_served[j] = i;
	  j++;
	}
      gmpi_debug_assert(j == new_config->num_served);
      new_config->next = NULL;
    }
  
  return return_config;
}

