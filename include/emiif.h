/*
EMI:    embedded message interface
Copyright (C) 2009  Cooper <davidontech@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see http://www.gnu.org/licenses/.
*/


#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "emi.h"

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
 * emi_msg_free - free the emi_msg structure alloced.
 * @msg:    the alloced emi_msg structure.
 *
 */
extern void emi_msg_free(struct emi_msg *msg);


/**
 * emi_msg_send - send a msg to the address that filled in the msg->dest_addr.ipv4.
 * @msg:    sended msg.
 *
 * @return:    a minus value indicates the process failed.
 */
extern int emi_msg_send(struct emi_msg *msg);


/**
 * emi_msg_register - register a msg and the attached function to the emi_core.
 * @defined_msg:    registered msg.
 * @func:            the attached function.
 *
 * @return:    a minus value indicates the process failed.
 */
extern int emi_msg_register(eu32 defined_msg,emi_func func);


/**
 * emi_msg_register - register a msg and the attached function to the emi_core.
 * @defined_msg:    registered msg.
 * @func:            the attached function.
 *
 * @return:    a minus value indicates the process failed.
 */
int emi_msg_register_exclusive(eu32 defined_msg,emi_func func);
#define emi_msg_register_blockreturn(msg,func)    emi_msg_register_exclusive(msg,func)

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
extern int emi_fill_msg(struct emi_msg *msg,char *dest_ip,void *data,eu32 cmd,eu32 defined_msg,eu32 flag);

/**
 * emi_fill_addr -fill a emi_addr structure
 * @addr:            the addr pointer which needed to be filled.
 * @ip:                the target ip address
 */
extern int emi_fill_addr(struct emi_addr *addr,char *ip,int port);

/**
 *
 *
 */

int emi_msg_send_highlevel_block(char *ipaddr, int msgnum,void *send_data,int send_size,void *ret_data, int ret_size, eu32 cmd);



/**
 *
 *
 *
 */

int emi_msg_send_highlevel_nonblock(char *ipaddr, int msgnum,void *send_data,int send_size, eu32 cmd);

/**
 *
 *
 *
 */
int emi_msg_prepare_return_data(struct emi_msg *msg, void *data, eu32 size);

/**
 *emi_init -    you may use it at the very beginning of your program
 *                and speed up very a little when responding the recieved msg.
 *
 */
extern int emi_init(void);

extern void emi_loop(void);
#ifdef __cplusplus
}
#endif
#endif
