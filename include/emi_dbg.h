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

#ifndef    __DEBUG_H__
#define    __DEBUG_H__


#ifdef DEBUG

#include "emi.h"
#include "msg_table.h"
#include "emi_config.h"


/*
 * dbg is just a printf in console
 */
#define dbg(format, arg...)                                            \
    do {                                                            \
        printf("EMIDEBUG:    %s: %s: " format,__FILE__, __func__, ## arg);         \
    } while (0)

/*
 * emiprt can be changed into a log file, so emiprt should be used for emi_core only.
 *
 */
#define emiprt(a) perror(a)

extern void debug_flag(eu32 flag);
extern void debug_addr(struct emi_addr *addr,char *p);
extern void debug_msg(struct emi_msg *msg,int more);
extern void debug_single_map(struct msg_map *map);
extern void debug_msg_map(struct msg_map **table,struct msg_map *map);
extern void debug_msg_table(struct msg_map **table);
extern void debug_msg_full_table(struct msg_map **table);
extern void debug_msg_chain(struct msg_map **table,struct msg_map *map);
extern void debug_config(struct emi_config *config);
extern void debug_msg_body(struct emi_msg *msg);

#else

#define dbg(a,arg...)
#define debug_flag(flag)
#define debug_addr(addr,p)
#define debug_msg(msg,more)
#define debug_single_map(map)
#define debug_msg_map(table,map)
#define debug_msg_table(table)
#define debug_msg_chain(table,map)
#define debug_msg_full_table(table)
#define emiprt(a)
#define debug_config(config)
#define debug_msg_body(msg);

#endif

#endif
