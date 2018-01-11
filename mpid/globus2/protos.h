#ifndef __globdev_protos__
#define __globdev_protos__

#include <globus_common.h>
#include <globus_io.h>
#include <globus_ftp_control.h> /* GRIDFTP */

/*******************/
/* Data Structures */
/*******************/

/***********************/
/* general proto stuff */
/***********************/

enum proto {tcp, mpi, unknown};
struct miproto_t
{
    struct miproto_t *next;
    enum proto       type;
    void             *info; /* proto-specific info */
};

struct channel_t
{
    struct miproto_t *proto_list;
    struct miproto_t *selected_proto; /* points to one in proto_list */
};

#endif /* __glodev_protos__ */
