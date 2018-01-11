#include "stdio.h"
#include "t3drecv.h"

main()
{
  printf("Sizeof(T3D_PKT_HEAD_T) == [%d]\n",sizeof(T3D_PKT_HEAD_T));
  printf("Sizeof(T3D_PKT_SHORT_T) == [%d]\n",sizeof(T3D_PKT_SHORT_T));
  printf("Sizeof(T3D_PKT_LONG_T) == [%d]\n",sizeof(T3D_PKT_LONG_T));
  printf("Sizeof(T3D_PKT_SHORT_SYNC_T) == [%d]\n",sizeof(T3D_PKT_SHORT_SYNC_T));
  printf("Sizeof(T3D_PKT_LONG_SYNC_T) == [%d]\n",sizeof(T3D_PKT_LONG_SYNC_T));
}
