#include <stdlib.h>
#include <math.h>
#include "mpi.h"
#include "mpid.h"
#include "sppfastcoll.h"
/*extern FILE *fp;*/

MPID_Fastbar *MPID_SPP_Alloc_barrier();

int MPID_SPP_First_barrier(comm)
MPI_Comm comm;
{
   MPID_Fastbar *bar;
   int nproc,node,enode,ncycles;
   int twok,twok1,k,to,from,c,ns,nr;
   MPI_Datatype db_stype,db_rtype;
   MPI_Request req[2];
   MPI_Status mpi_status[2];
   static int mps_db_init=0,mps_db_msgtype=0,*blocklength;
   static MPI_Aint *addr;
   char *ptr;
   int ierr;
   int rank;

   MPI_Comm_size(comm,&nproc);

   if( comm->ADIBarrier==NULL) bar = MPID_SPP_Alloc_barrier(comm);

   MPI_Comm_rank(comm,&node);
   ncycles = bar->nc;

   twok1 = (int)pow(2.0,(double )(ncycles-1));

   if (++mps_db_msgtype > 32767) mps_db_msgtype=1;

   if(nproc>1) {
       addr = (MPI_Aint *)malloc(nproc* sizeof(MPI_Aint));
       blocklength = (int *)malloc(nproc* sizeof(int));
    
       for (k = 0;k<ncycles;k++) {
          twok = twok1+twok1;
          /* first do the send */
          ns=0;
          /*fprintf(fp,"cycle %ld: ",k);fflush(fp); */
          for (c = 0;c<nproc;c++) {
             enode = (node-c);
             if(enode < 0) enode +=nproc;
             if(enode%twok  ==  0) {
                if (enode + twok1 <nproc)  {
                   /*fprintf(fp,"%ld ",c);fflush(fp);*/
                   MPI_Address(&(bar->barf[c].flag),&addr[ns]);
                   blocklength[ns++]=sizeof(MPI_Aint);
                }
             }
          }
          if( (ierr = MPI_Type_hindexed(ns,blocklength,addr,
                                MPI_BYTE,&db_stype))) goto error_exit;
          if( (ierr = MPI_Type_commit(&db_stype))) goto error_exit;
          to = (node+twok1)%nproc;
          /*fprintf(fp,"S to= %ld\n",to);fflush(fp);*/
          if( (ierr = MPI_Isend(MPI_BOTTOM,1,db_stype,to,mps_db_msgtype,
                            comm,&req[0]))) goto error_exit;
    
          /* now do the receive */
          /*fprintf(fp,"cycle %ld: ",k);fflush(fp);*/
          nr=0;
          for (c = 0;c<nproc;c++){
             enode = (node-c);
             if(enode<0) enode += nproc;
             if (((enode%twok)!=0)&&((enode%twok1) == 0)) {
                /*fprintf(fp,"%ld ",c);fflush(fp);*/
                MPI_Address(&(bar->barf[c].flag),&addr[nr]);
                blocklength[nr++]=sizeof(MPI_Aint);
             }
          }
          if( (ierr= MPI_Type_hindexed(nr,blocklength,addr,
                   MPI_BYTE,&db_rtype))) goto error_exit;
          if( (ierr = MPI_Type_commit(&db_rtype))) goto error_exit;
          from = node - twok1;
          if(from<0) from += nproc;
          /*fprintf(fp,"R from= %ld\n",from);fflush(fp);*/
          if( (ierr = MPI_Irecv(MPI_BOTTOM,1,db_rtype,from,mps_db_msgtype,
                            comm,&req[1]))) goto error_exit;
    
          if( (ierr = MPI_Waitall(2,req,mpi_status))) goto error_exit;
          if( (ierr = MPI_Type_free(&db_stype))) goto error_exit;
          if( (ierr = MPI_Type_free(&db_rtype))) goto error_exit;
    
          twok1 /=2;
       }
       free(addr);
       free(blocklength);
   }

   for (c = 0;c<nproc;c++){
       ptr = (char *)bar->barf[c].flag;
       bar->barf[c].ival = (int *)(ptr+4);
       bar->barf[c].addr = (void **)(ptr+8);
       bar->barf[c].dval = (double *)(ptr+16);
       bar->barf[c].rval = (float *)(ptr+20);
   }

   return 0;
error_exit:
   free(addr);
   free(blocklength);
   return ierr;
}

