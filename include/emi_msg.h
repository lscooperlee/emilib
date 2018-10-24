
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

struct emi_retdata_container;

struct emi_addr{
    struct sockaddr_in    ipv4;
    pid_t    pid;
    eu32 id;
};


struct emi_msg{
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
* Used for marking if msg->data is allocated, if so the data area should be freed when msg area is freed
*/
#define EMI_MSG_FLAG_ALLOCDATA        0x00040000

    eu32 msg;
    eu32 cmd;
    eu32 size;
    eu32 retsize;

/** The members above this line are payload, meaning they will be sent in communication **/
/** The members below will not be sent, they are for local use and may be initialized locally **/

    es64 data_offset;
    es64 retdata_offset;

    struct emi_addr addr;

#ifndef SEND_ONLY
    eu32 count;
    espinlock_t lock;
#endif

#ifdef __cplusplus
    void *data() const noexcept;
    emi_retdata_container retdata() noexcept;
#endif
};

#define EMI_MSG_PAYLOAD_SIZE    20


struct emi_retdata {
    es64 next_offset;
    eu32 size;
    eu32 pad;
    char data[];
}; //FIXME: packed


typedef int (*emi_func)(struct emi_msg const *);


#define GET_OFFSET(base, addr) ((char *)(addr)-(char *)(base))
#define GET_ADDR(base, offset) ((char *)(base) + (offset))

inline static void *GET_DATA(struct emi_msg const *msg) {
    return (void *)GET_ADDR(msg, ((struct emi_msg *)msg)->data_offset);
}

inline static struct emi_retdata *get_next_retdata(struct emi_msg const *msg, struct emi_retdata const *data) {

    if(msg->retsize == 0){
        return NULL;
    }

    struct emi_retdata *newdata;
    
    if(data == NULL)
        newdata = (struct emi_retdata *)GET_ADDR(msg, ((struct emi_msg *)msg)->retdata_offset);
    else {
        newdata = (struct emi_retdata *)GET_ADDR(msg, data->next_offset);
    }

    if(newdata == (struct emi_retdata *)msg){
        return NULL;
    }else{
        return newdata;
    }
}

#define for_each_retdata(msg, data)  \
            for(data = get_next_retdata((msg), NULL); (data) !=NULL; data = get_next_retdata((msg), (data)))

#endif
