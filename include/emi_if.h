
#ifndef __EMI_IF_H__
#define __EMI_IF_H__

#include "emi_types.h"
#include "emi_msg.h"

extern struct emi_msg *emi_msg_alloc(eu32 size);

extern void emi_msg_free_data(struct emi_msg *msg);

extern void emi_msg_free(struct emi_msg *msg);

extern int emi_msg_send(struct emi_msg *msg);

extern int emi_msg_init(struct emi_msg *msg,const char *dest_ip,const void *data,
        eu32 cmd,eu32 defined_msg,eu32 flag);

#endif
