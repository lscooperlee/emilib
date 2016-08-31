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

#ifndef __EMI_H__
#define __EMI_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include "emi_types.h"

#ifdef BLUETOOTH
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#endif


struct emi_addr{
    struct sockaddr_in    ipv4;
#ifdef BLUETOOTH
    struct sockaddr_l2    l2cap;
#endif
    pid_t    pid;
    eu32 id;
};


struct emi_msg{
    struct emi_addr dest_addr;
    struct emi_addr src_addr;
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
 * This flag is used in BLOCK mode to indicate that extra data is returned from emi message handler function
 */
#define EMI_MSG_RET_WITHDATA        0x00020000

    eu32 count;                //the member is used for count the processes when several processes share one massage.it is internally,do not use it. useful in ~BLOCK msg.
    eu32 size;
    eu32 cmd;
    eu32 msg;
    char data[];
};

typedef int (*emi_func)(struct emi_msg *);


#endif
