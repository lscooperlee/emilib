
#ifndef __EMI_IF_H__
#define __EMI_IF_H__

#include "emi_types.h"

struct emi_addr;
struct emi_msg;
typedef int (*emi_func)(struct emi_msg const *);

/**
 * emi_addr_alloc - alloc a emi_addr structure.
 *
 * @return:NULL if allocation failed.
 */
extern struct emi_addr *emi_addr_alloc(void);


/**
 * emi_addr_free - free the emi_addr structure alloced.
 * @addr:    the alloced emi_addr structure.
 *
 */
extern void emi_addr_free(struct emi_addr *addr);


/**
 * emi_msg_alloc - alloc a emi_msg structure.
 * @create:    set YESCREATE will create the field of emi_addr structure internally.normally this is required.
 * @size:    the size of data that needed allocing, the function will not alloc data if is zero.
 *
 * @return:    NULL if allocation failed
 */
extern struct emi_msg *emi_msg_alloc(eu32 size);

/**
 * emi_msg_free_data - free the emi_msg data field if it is allocated.
 * @msg:    the emi_msg structure pointer.
 *
 */
extern void emi_msg_free_data(struct emi_msg *msg);

/**
 * emi_msg_free - free the emi_msg structure alloced.
 * @msg:    the alloced emi_msg structure.
 *
 */
extern void emi_msg_free(struct emi_msg *msg);


/**
 * emi_msg_send - send a msg to the address that filled in the msg->addr.ipv4.
 * @msg:    sended msg.
 *
 * @return:    a minus value indicates the process failed.
 */
extern int emi_msg_send(struct emi_msg *msg);


/**
 * emi_msg_unregister - unregister a msg and the attached function from the emi_core.
 * @defined_msg:    unregistered msg.
 * @func:            the attached function.
 *
 */
extern void emi_msg_unregister(eu32 defined_msg,emi_func func);


/**
 * emi_fill_msg - fill a emi_msg structure.
 * @msg:            the emi_msg structure that wanted filling.
 * @dest_ip:        target ip address.could be NULL if not use.
 * @data:            a pointer to packed data that wanted sending.could be NULL if not use.
 * @cmd:            the cmd needed that needed sending.
 * @defined_msg:    the msg that want to be sended
 * @flag:            the sending flag
 *
 * @return:    a minus value indicates the process failed.
 * Note, that list is expected to be not empty.
 */
extern int emi_fill_msg(struct emi_msg *msg,const char *dest_ip,const void *data,eu32 cmd,eu32 defined_msg,eu32 flag);

/**
 * emi_fill_addr -fill a emi_addr structure
 * @addr:            the addr pointer which needed to be filled.
 * @ip:                the target ip address
 */
extern int emi_fill_addr(struct emi_addr *addr,const char *ip,int port);

extern int emi_msg_send_highlevel(const char *ipaddr, int msgnum, void *send_data, 
        int send_size, void *ret_data, int ret_size, eu32 cmd, eu32 flag);

extern int emi_msg_send_highlevel_block(const char *ipaddr, int msgnum,void *send_data,
        int send_size,void *ret_data, int ret_size, eu32 cmd);

extern int emi_msg_send_highlevel_nonblock(const char *ipaddr, int msgnum,void *send_data,
        int send_size, eu32 cmd);


#endif
