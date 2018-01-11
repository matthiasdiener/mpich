
#include <globdev.h>
#include "mpid.h"
#include "mpiimpl.h"
#include "protos.h"
#include "attr.h"


/*********************/
/* public prototypes */
/*********************/

#include "topology_intra_fns.h"


/**********************/
/* private prototypes */
/**********************/

static int num_protos_in_channel(struct channel_t *);
static int channels_proto_match(struct channel_t *, int, struct channel_t *,
                                int, int);
static void print_topology(struct MPIR_COMMUNICATOR *);
static int create_topology_keys(struct MPIR_COMMUNICATOR *);


/******************/
/* global symbols */
/******************/

/* key to retrieve the Topology information from each communicator */
int MPICHX_TOPOLOGY_COLORS = MPI_KEYVAL_INVALID;
int MPICHX_TOPOLOGY_DEPTHS = MPI_KEYVAL_INVALID;
MPI_Delete_function mpi_topology_depths_destructor;
MPI_Delete_function mpi_topology_colors_destructor;

/**************************/
/* local utility function */
/**************************/

/******** print_topology **********************************************/

static void
print_topology(struct MPIR_COMMUNICATOR *comm)
{
   int max_depth = 0;
   int i, j, rank, size;

   (void) MPIR_Comm_size(comm, &size);
   (void) MPIR_Comm_rank(comm, &rank);

   globus_libc_fprintf(stderr, "*** Start print topology from proc #%d/%d\n",
                       rank, size);
   globus_libc_fprintf(stderr, "Sizes of my clusters:\n");
   for (i = 0; i < comm->Topology_Depths[rank]; i++)
      globus_libc_fprintf(stderr, "Level %d: %d procs\n", i,
                                               comm->Topology_ClusterSizes[i]);
   globus_libc_fprintf(stderr, "proc\t");
   for (i = 0; i < size; i++)
      globus_libc_fprintf(stderr, "% 3d", i);
   globus_libc_fprintf(stderr, "\ndepths\t");
   for (i = 0; i < size; i++)
   {
      if ( max_depth < comm->Topology_Depths[i] )
         max_depth = comm->Topology_Depths[i];
      globus_libc_fprintf(stderr, "% 3d", comm->Topology_Depths[i]);
   }
   globus_libc_fprintf(stderr, "\nCOLORS:");
   for (i = 0; i < max_depth; i++)
   {
      globus_libc_fprintf(stderr, "\nlvl %d\t", i);
      for (j = 0; j < size; j++)
         if ( i < comm->Topology_Depths[j] )
            globus_libc_fprintf(stderr, "% 3d",
                                        comm->Topology_ColorTable[j][i]);
         else
            globus_libc_fprintf(stderr, "   ");
   }
   globus_libc_fprintf(stderr, "\nCLUSTER_IDS:");
   for (i = 0; i < max_depth; i++)
   {
      globus_libc_fprintf(stderr, "\nlvl %d\t", i);
      for (j = 0; j < size; j++)
         if ( i < comm->Topology_Depths[j] )
            globus_libc_fprintf(stderr, "% 3d",
                                        comm->Topology_ClusterIds[j][i]);
         else
            globus_libc_fprintf(stderr, "   ");
   }
   globus_libc_fprintf(stderr, "\n");
   globus_libc_fprintf(stderr, "*** End print topology\n");
   return;
}

/******** cluster_table ***********************************************/

int
cluster_table(struct MPIR_COMMUNICATOR *comm)
{
   /*********************************************************/
   /* initializing Topology_Depths and Topology_ClusterIds  */
   /* for topology-aware collective operations and topology */
   /* reporting to MPI app                                  */
   /*********************************************************/

   int max_depth, rank, size;
   int mpi_errno = MPI_SUCCESS;
   int p0, p1;
   int lvl, i;
   int *Topology_Depths;
   int **Topology_ClusterIds;
   int **color;
   int **Cluster_Sets;

   /* don't do anything for intercommunicators */
   if (comm->comm_type == MPIR_INTER) return mpi_errno;

   (void) MPIR_Comm_rank(comm, &rank);
   (void) MPIR_Comm_size(comm, &size);

   /*********************************/
   /* Phase 1 of 3 - Finding Depths */
   /*********************************/

   Topology_Depths = (int *) globus_libc_malloc (size*sizeof(int));
   if ( !Topology_Depths )
      MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                 "cluster_table() - failed malloc");
   comm->Topology_Depths = Topology_Depths;

   max_depth = 0;
   for (i = 0; i < size; i ++)
   {
      struct channel_t *chanl;

      chanl = get_channel(comm->lrank_to_grank[i]);
      if ( !chanl )
         MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                    "cluster_table() - NULL channel returned");
      Topology_Depths[i] =
             num_protos_in_channel(chanl);

      if (Topology_Depths[i] > max_depth)
         max_depth = Topology_Depths[i];
   } /* endfor */

   comm->Topology_InfoSets = (single_set_t *) globus_libc_malloc (
                                 Topology_Depths[rank] * sizeof(single_set_t));
   if ( comm->Topology_InfoSets == NULL )
      MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                 "cluster_table() - failed malloc");

   /***************************/
   /* Phase 2 of 3 - Coloring */
   /***************************/

   color = (int **) globus_libc_malloc (size*sizeof(int *));
   if ( !color )
      MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                 "cluster_table() - failed malloc");
   comm->Topology_ColorTable = color;
   for (i = 0; i < size; i ++)
   {
      color[i] = (int *) globus_libc_malloc (Topology_Depths[i]*sizeof(int));
      if ( !color[i] )
         MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                    "cluster_table() - failed malloc");

      for (lvl = 0; lvl < Topology_Depths[i]; lvl ++)
         color[i][lvl] = -1;
   } /* endfor */

   for (lvl = 0; lvl < max_depth; lvl ++)
   {
      int next_color = 0;

      for (p0 = 0; p0 < size; p0 ++)
      {
         if (lvl < Topology_Depths[p0] && color[p0][lvl] == -1)
         {
            /* this proc has not been colored at this level yet, */
            /* i.e., it hasn't matched any of the procs to the   */
            /* left at this level yet ... ok, start new color    */
            /* at this level.                                    */

            color[p0][lvl] = next_color ++;
            for (p1 = p0 + 1; p1 < size; p1 ++)
            {
               if (lvl < Topology_Depths[p1] && color[p1][lvl] == -1)
               {
                  struct channel_t *chanl0, *chanl1;;

                  chanl0 = get_channel(comm->lrank_to_grank[p0]);
                  if ( !chanl0 )
                     MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2,
                                "MPICH-G2 Internal",
                                "cluster_table() - NULL channel returned");
                  chanl1 = get_channel(comm->lrank_to_grank[p1]);
                  if ( !chanl1 )
                     MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2,
                                "MPICH-G2 Internal",
                                "cluster_table() - NULL channel returned");

                  if (channels_proto_match(chanl0, Topology_Depths[p0]-1-lvl,
                                           chanl1, Topology_Depths[p1]-1-lvl,
                                           lvl))
                     color[p1][lvl] = color[p0][lvl];
               } /* endif */
            } /* endfor */
         } /* endif */
      } /* endfor */
   } /* endfor */

   /************************************************/ 
   /* Phase 3 of 3 - Setting CID's based on colors */
   /************************************************/

   Cluster_Sets = (int**) globus_libc_malloc (Topology_Depths[rank]
                                              * sizeof(int*));
   if ( !Cluster_Sets )
      MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                 "cluster_table() - failed malloc");
   comm->Topology_ClusterSets = Cluster_Sets;
   for (i = 0; i < Topology_Depths[rank]; i++)
      Cluster_Sets[i] = (int*) NULL;

   Topology_ClusterIds = (int **) globus_libc_malloc (size*sizeof(int *));
   if ( !Topology_ClusterIds )
      MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                 "cluster_table() - failed malloc");
   comm->Topology_ClusterIds = Topology_ClusterIds;
   for (i = 0; i < size; i ++)
   {
      Topology_ClusterIds[i] = 
                   (int *) globus_libc_malloc (Topology_Depths[i]*sizeof(int));
      if ( !Topology_ClusterIds[i] )
         MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                    "cluster_table() - failed malloc");

      /* intializing to an invalid value -1 */
      for (lvl = 0; lvl < Topology_Depths[i]; lvl ++)
         Topology_ClusterIds[i][lvl] = -1;
   } /* endfor */

   for (lvl = max_depth-1; lvl >= 0; lvl --)
   {
      for (p0 = 0; p0 < size; p0 ++)
      {
         if (lvl < Topology_Depths[p0] 
             && Topology_ClusterIds[p0][lvl] == -1)
         {
            /* p0 has not been assigned a cid at this level yet,
             * which means all the procs at this level that
             * have the same color as p0 at this level have also 
             * not been assigned cid's yet.
             *
             * find the rest of the procs at this level that 
             * have the same color as p0 and assign them cids.
             * same color are enumerated.  */

            int next_cid;

            Topology_ClusterIds[p0][lvl] = 0;
            next_cid = 1;

            for (p1 = p0+1; p1 < size; p1 ++)
            {
               if (lvl < Topology_Depths[p1]
                   && color[p0][lvl] == color[p1][lvl])
               {
                  /* p0 and p1 match colors at this level, 
                   * which means p1 will now get its cid set at
                   * this level.  but to what value?  if p1 also
                   * matches color with any proc to its left
                   * at level lvl+1, then p1 copies that proc's
                   * cid at this level, otherwise p1 gets the
                   * next cid value at this level.  */

                  if (lvl+1 < Topology_Depths[p1])
                  {
                     int p2;

                     for (p2 = 0; p2 < p1; p2 ++)
                     {
                        if (lvl+1 < Topology_Depths[p2]
                            && color[p1][lvl] == color[p2][lvl]
                            && color[p1][lvl+1] == color[p2][lvl+1])
                        {
                           Topology_ClusterIds[p1][lvl] =
                                                  Topology_ClusterIds[p2][lvl];
                           break;
                        }
                     } /* endfor */
                     if (p2 == p1)
                        /* did not find one */
                        Topology_ClusterIds[p1][lvl] = next_cid ++;
                  }
                  else
                     /* p1 does not have a level lvl+1 */
                     Topology_ClusterIds[p1][lvl] = next_cid ++;
               } /* endif */
            } /* endfor */

            if ( lvl < Topology_Depths[rank]  &&  Cluster_Sets[lvl] == NULL
                 &&  Topology_ClusterIds[rank][lvl] != -1 )
            {
               /* my proc has just gotten a cid at this level */
               /* I need to allocate memory for the set of master processes */
               /* of my cluster at this level */
               Cluster_Sets[lvl] = (int*) globus_libc_malloc (next_cid
                                                              * sizeof(int));
               if ( !Cluster_Sets[lvl] )
                  MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2,
                             "MPICH-G2 Internal",
                             "cluster_table() - failed malloc");
            }
         } /* endif */
      } /* endfor */
   } /* endfor */

   /* know how many processes are in my cluster at each level */
   comm->Topology_ClusterSizes = (int *) globus_libc_malloc (sizeof(int)
                                                      * Topology_Depths[rank]);
   if ( !comm->Topology_ClusterSizes  )
      MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                 "cluster_table() - failed malloc");
   for (lvl = 0; lvl < Topology_Depths[rank]; lvl++)
   {
      int cnt;
      int my_color = color[rank][lvl];

      for (cnt = 0, i = 0; i < size; i++)
         if ( Topology_Depths[i] > lvl  &&  my_color == color[i][lvl] )
            cnt++;
      comm->Topology_ClusterSizes[lvl] = cnt;
   }

   mpi_errno = create_topology_keys(comm);
/*
printf("create_topology_keys returned %d\n", mpi_errno);
*/

   return mpi_errno;
}

/******** destroy_cluster_table ***************************************/

void
destroy_cluster_table(struct MPIR_COMMUNICATOR *comm)
{
   int i, my_depth, rank, size;

   (void) MPIR_Comm_rank(comm, &rank);
   (void) MPIR_Comm_size(comm, &size);

   /* don't do anything for intercommunicators */
   if (comm->comm_type == MPIR_INTER) return;

/*
   if (rank == 0) print_topology(comm);
*/

   for (i = 0; i < size; i ++)
      g_free(comm->Topology_ColorTable[i]);
   g_free(comm->Topology_ColorTable);

   my_depth = comm->Topology_Depths[rank];
   for (i = 0; i < my_depth; i ++)
      g_free(comm->Topology_ClusterSets[i]);
   g_free(comm->Topology_ClusterSets);

   for (i = 0; i < size; i++)
      g_free(comm->Topology_ClusterIds[i]);
   g_free(comm->Topology_ClusterIds);

   g_free(comm->Topology_Depths);
   g_free(comm->Topology_InfoSets);
   g_free(comm->Topology_ClusterSizes);

   return;
}

/******** num_protos_in_channel ***************************************/

/* later need to fix this for WAN/LAN split of TCP: done (OK) */
static int
num_protos_in_channel(struct channel_t *cp)
{
    struct miproto_t *mp;
    int rc = 0;

    if (!cp)
    {
	globus_libc_fprintf(stderr,
	    "\tERROR: num_protos_in_channel(): grank %d: passed NULL cp\n",
	    MPID_MyWorldRank);
	MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 
				1, "MPICH-G2", "");
    } /* endif */

    for (mp = cp->proto_list; mp; mp = mp->next)
    {
	switch (mp->type)
	{
            /* TCP: 1 for localhost + 1 for LAN + 1 for WAN */
	    case tcp: rc += 3; break;
	    case mpi: rc ++; break;
	    default:
		globus_libc_fprintf(stderr,
		    "\tERROR: num_protos_in_channel(): grank %d: encountered "
		    "unrecognized proto type %d", 
		    MPID_MyWorldRank, mp->type);
		MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 
					1, "MPICH-G2", "");
		break;
	} /* end switch() */
    } /* endfor */

    return rc;

} /* end num_protos_in_channel() */


/******** channels_proto_match ****************************************/

static int
channels_proto_match(struct channel_t *cp0, int proto_idx0,
                     struct channel_t *cp1, int proto_idx1, int level)
{
    struct miproto_t *mp0, *mp1;
    int i;

    /* if level == 0, then the protocol is WAN-TCP: they always match */
    if ( level == 0 )
        return GLOBUS_TRUE;

    /* for LAN-TCP, the actual proto index is the given index minus 1
     * because the localhost-TCP level is a "pseudo level" */
    if ( level == 1 )
    {
        proto_idx0--;
        proto_idx1--;
    }

    if (proto_idx0 < 0 || proto_idx1 < 0)
    {
	globus_libc_fprintf(stderr,
	    "\tERROR: channels_proto_match(): grank %d: passed invalid args "
	    "proto_idx0 %d proto_idx1 %d\n", 
	    MPID_MyWorldRank, proto_idx0, proto_idx1);
	MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 1, "MPICH-G2", "");
    } /* endif */

    /* finding the correct protos in each channel */
    for (i = 0, mp0 = cp0->proto_list; mp0 && i < proto_idx0;
                                       mp0 = mp0->next, i ++)
    ;
    for (i = 0, mp1 = cp1->proto_list; mp1 && i < proto_idx1;
                                       mp1 = mp1->next, i ++)
    ;

    if (mp0 && mp1 && mp0->type == mp1->type)
    {
        int rc = GLOBUS_FALSE;

	/* now that i have correct proto for each, seeing if they match */
        switch (mp0->type)
        {
            case tcp:   /* are they on the same LAN? */
                /* level != 0 and proto == TCP */
                switch ( level )
                {
                    case 1:   /* are the procs in the same LAN? */
                    {
                        if ( !strcmp(((struct tcp_miproto_t *) 
                                                 (mp0->info))->globus_lan_id,
                                     ((struct tcp_miproto_t *)
                                                 (mp1->info))->globus_lan_id) )
                            rc = GLOBUS_TRUE;
                        break;
                    }
                    case 2:   /* are the procs on the same localhost? */
                    {
                        if ( ((struct tcp_miproto_t *)
                                                   (mp0->info))->localhost_id
                             == ((struct tcp_miproto_t *)
                                                   (mp1->info))->localhost_id )
                            rc = GLOBUS_TRUE;
                        break;
                    }
                    default: rc = GLOBUS_FALSE;
                }   /* end switch */
            break;
            case mpi:
                if (!strcmp(((struct mpi_miproto_t *) 
                                (mp0->info))->unique_session_string,
                            ((struct mpi_miproto_t *) 
                                (mp1->info))->unique_session_string))
                    rc = GLOBUS_TRUE;
            break;
            default:
                {
                    char err[1024];

                    globus_libc_sprintf(err,
                        "channels_proto_match(): unrecognizable "
                        "proto type %d", 
                        mp0->type);
                    MPID_Abort( (struct MPIR_COMMUNICATOR *)0, 
                            1, "MPICH-G2", err);
                }
            break;
        } /* end switch */
        return rc;
    } /* end if */
    else
	/* one or both proto_idx's was too deep for its channel */
	return GLOBUS_FALSE;

} /* end channels_proto_match() */


/******** create_topology_keys ****************************************/

static int
create_topology_keys(struct MPIR_COMMUNICATOR *comm)
{
   int mpi_errno = MPI_SUCCESS;
   int flag, size;
   int *Depths;
   int **Colors;

   (void) MPIR_Comm_size (comm, &size);

   if ( MPICHX_TOPOLOGY_DEPTHS == MPI_KEYVAL_INVALID )
   {
      mpi_errno = MPI_Keyval_create(MPI_NULL_COPY_FN,
                                    mpi_topology_depths_destructor,
                                    &MPICHX_TOPOLOGY_DEPTHS, NULL);
      if ( mpi_errno ) return mpi_errno;
   }

   mpi_errno = MPI_Attr_get(comm->self, MPICHX_TOPOLOGY_DEPTHS, &Depths,
                            &flag);
   if ( mpi_errno ) return mpi_errno;

   if ( !flag )
   {
      int i;

      /* copy the information available at the user level: the user must
       * not have a direct access to the pointer to the real data used by
       * the MPICH library */
      Depths = (int *) globus_libc_malloc (sizeof(int) * size);
      if ( !Depths )
         MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                    "create_topology_keys() - failed malloc");

      for (i = 0; i < size; i++)
         Depths[i] = comm->Topology_Depths[i];
      mpi_errno = MPI_Attr_put(comm->self, MPICHX_TOPOLOGY_DEPTHS, Depths);
      if ( mpi_errno ) return mpi_errno;
   }

   if ( MPICHX_TOPOLOGY_COLORS == MPI_KEYVAL_INVALID )
   {
      mpi_errno = MPI_Keyval_create(MPI_NULL_COPY_FN,
                                    mpi_topology_colors_destructor,
                                    &MPICHX_TOPOLOGY_COLORS, NULL);
      if ( mpi_errno ) return mpi_errno;
   }

   mpi_errno = MPI_Attr_get(comm->self, MPICHX_TOPOLOGY_COLORS, &Colors,
                            &flag);
   if ( mpi_errno ) return mpi_errno;

   if ( !flag )
   {
      int i;

      /* copy the information available at the user level: the user must
       * not have a direct access to the pointer to the real data used by
       * the MPICH library */
      Colors = (int **) globus_libc_malloc (sizeof(int *) * size);
      if ( !Colors )
         MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                    "create_topology_keys() - failed malloc");

      for (i = 0; i < size; i++)
      {
         int j, dep = Depths[i];
         int *column = comm->Topology_ColorTable[i];

         Colors[i] = (int *) globus_libc_malloc (sizeof(int) * dep);
         if ( Colors[i] == NULL )
            MPID_Abort((struct MPIR_COMMUNICATOR *)0, 2, "MPICH-G2 Internal",
                       "create_topology_keys() - failed malloc");
         for (j = 0; j < dep; j++)
            Colors[i][j] = column[j];
      }

      mpi_errno = MPI_Attr_put(comm->self, MPICHX_TOPOLOGY_COLORS, Colors);
      if ( mpi_errno ) return mpi_errno;
   }

   return mpi_errno;
}


/******** mpi_topology_depths_destructor ******************************/

int
mpi_topology_depths_destructor(MPI_Comm comm, int key, void *attr, void *extra)
{
   int mpi_errno = MPI_SUCCESS;
   int *Depths;

   /* if the key is not valid any longer, then the attribute has already
    * been deleted (and memory freed) */
   if ( MPICHX_TOPOLOGY_DEPTHS == MPI_KEYVAL_INVALID )
      return mpi_errno;

   if ( key != MPICHX_TOPOLOGY_DEPTHS )
      /* what does that mean?  What return value?  If there's no bug in MPICH
       * library, this case should never be encountered... */
      return mpi_errno;

   Depths = (int *) attr;
   g_free(Depths);

   return mpi_errno;
}


/******** mpi_topology_colors_destructor ******************************/

int
mpi_topology_colors_destructor(MPI_Comm comm, int key, void *attr, void *extra)
{
   int mpi_errno = MPI_SUCCESS;
   int i, size;
   int **Colors;

   /* if the key is not valid any longer, then the attribute has already
    * been deleted (and memory freed) */
   if ( MPICHX_TOPOLOGY_COLORS == MPI_KEYVAL_INVALID )
      return mpi_errno;

   if ( key != MPICHX_TOPOLOGY_COLORS )
      /* what does that mean?  What return value?  If there's no bug in MPICH
       * library, this case should never be encountered... */
      return mpi_errno;

   Colors = (int **) attr;
   (void) MPI_Comm_size(comm, &size);

   for (i = 0; i < size; i++)
      g_free(Colors[i]);
   g_free(Colors);

   return MPI_SUCCESS;
}

