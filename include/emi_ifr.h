
#ifndef __EMI_IFR_H__
#define __EMI_IFR_H__

#include "emi_types.h"

struct emi_msg;
typedef int (*emi_func)(struct emi_msg const *);

/**
* emi_msg_register - register a msg and the attached function to the emi_core.
* @defined_msg:    registered msg.
* @func:            the attached function.
*
* @return:    a minus value indicates the process failed.
*/
extern int emi_msg_register(eu32 defined_msg, emi_func func);


/**
* emi_msg_unregister - unregister a msg and the attached function from the emi_core.
* @defined_msg:    unregistered msg.
* @func:            the attached function.
*
*/
extern void emi_msg_unregister(eu32 defined_msg,emi_func func);

extern void *emi_retdata_alloc(struct emi_msg const *msg, eu32 size);

extern int emi_load_retdata(struct emi_msg const *msg, const void *data, eu32 size);

extern int emi_init(void);

extern void emi_loop(void);


#endif
