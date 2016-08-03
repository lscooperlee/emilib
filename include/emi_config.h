#ifndef __EMI_CONFIG_H__
#define __EMI_CONFIG_H__
#include "msg_table.h"
#include "shmem.h"

#define EMI_MAX_MSG	(1<<EMI_HASH_MASK)
#define EMI_DATA_SIZE_PER_MSG	(1024)
#define EMI_MAX_DATA	(EMI_MAX_MSG/5)
#define USR_EMI_PORT	1361

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
