
#ifndef __EMI_H__
#define __EMI_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include "emi_types.h"

#ifndef SEND_ONLY
#include "emi_lock.h"
#endif


struct emi_addr{
    struct sockaddr_in    ipv4;
    pid_t    pid;
    eu32 id;
};


struct emi_msg{
    struct emi_addr addr;
    eu32 flag;


/*
 * the default msg is  ~BLOCK, unless this FLAG was set.
 */
#define EMI_MSG_MODE_BLOCK          0x00000100

/*
 * This flag is used for registering a emi message to emi_core
 */
#define EMI_MSG_CMD_REGISTER        0x00004000

/*
 * This flag is used in BLOCK mode to indicate the emi message handler function is succeeded
 */
#define EMI_MSG_RET_SUCCEEDED       0x00010000

/*
 * This flag is used indicate the emi message has extra data with it internally
 */
#define EMI_MSG_FLAG_RETDATA        0x00020000

/*
* Used for marking if msg->data is allocated, if so the data area should be freed when msg area is freed
*/
#define EMI_MSG_FLAG_ALLOCDATA        0x00040000

    eu32 msg;
    eu32 cmd;
    eu32 size;
    eu32 retsize;

/** The members above this line are payload, meaning they will be sent in communication **/
/** The members below will not be sent, they are for local use and may be initialized locally **/

    eu64 data_offset;
    eu64 retdata_offset;

#ifndef SEND_ONLY
    eu32 count;
    espinlock_t lock;
#endif

#ifdef __cplusplus
    emi_msg() = delete;
#endif
};

#define EMI_MSG_PAYLOAD_SIZE    ((unsigned long)&((struct emi_msg *)0)->data_offset)


typedef int (*emi_func)(struct emi_msg const *);


#define GET_OFFSET(base, addr) ((char *)(addr)-(char *)(base))
#define GET_ADDR(base, offset) ((char *)(base) + (offset))
#define GET_DATA(msg) GET_ADDR((msg), ((struct emi_msg *)(msg))->data_offset)
#define GET_RETDATA(msg) GET_ADDR((msg), ((struct emi_msg *)(msg))->retdata_offset)

#endif
