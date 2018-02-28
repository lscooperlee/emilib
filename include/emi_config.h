#ifndef __EMI_CONFIG_H__
#define __EMI_CONFIG_H__

#include "emi_types.h"

#ifndef EMI_HASH_MASK
#define EMI_HASH_MASK    8
#endif

#define EMI_MSG_TABLE_SIZE    (1<<EMI_HASH_MASK)
#define USR_EMI_PORT    1361

#define EMI_MSG_BUF_SIZE    (1<<EMI_ORDER_NUM)

struct emi_config{
    eu32 emi_port;
};

extern struct emi_config *emi_config;

extern struct emi_config *get_config(void);
extern void set_default_config(struct emi_config *config);

#endif
