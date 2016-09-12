#ifndef __EMI_CONFIG_H__
#define __EMI_CONFIG_H__

#include "emi_types.h"
#include "msg_table.h"
#include "emi_shbuf.h"

#define EMI_MSG_TABLE_SIZE    (1<<EMI_HASH_MASK)
#define EMI_MAX_MSG_SIZE (0x1FF)
#define EMI_MAX_DATA    (EMI_MAX_MSG/5)
#define USR_EMI_PORT    1361

#define EMI_MSG_BUF_SIZE    (1<<(EMI_ORDER_NUM - 1))

struct emi_config{
    eu32 emi_port;
    eu32 emi_data_size_per_msg;
    eu32 emi_key;
};

extern struct emi_config *emi_config;

extern struct emi_config *get_config(void);
extern void set_default_config(struct emi_config *config);
extern struct emi_config *guess_config(void);
extern struct emi_config *get_root_default_config(void);
extern struct emi_config *get_usr_default_config(void);

#endif
