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

/**
 * for the sake of simplicity, we trim some redundant and never used flags, such as :
 *         EMI_MSG_TYPE_CMD: which is always true because 'cmd' is a member in the struct emi_msg
 *         EMI_MSG_FLAG_NONBLOCK: a msg should have had only two mode, BLOCK and NONBLOCK, one msg can be either a BLOCK mode
 *             or NONBLOCK mode, but not both. so a NONBLOCK msg can be seen as ~BLOCK , we decide to
 *             leave "EMI_MSG_MODE_BLOCK" only.
 *
 *             Note that EMI_MSG_FLAG_BLOCK is changed into EMI_MSG_MODE_BLOCK, all EMI_MSG_XXXX_XXXX are flags, but BLOCK or ~BLOCK
 *             is a kind of MODE.
 *
 *         EMI_MSG_FLAG_BLOCK_RETURN: this is important. at the last release, we add a new mode called BLOCK_RETURN in order to return
 *             extra data to the sender. also, BLOCK_RETURN is an exclusive msg, which means only one msg number can be registered
 *             compared with ~BLOCK mode. if BLOCK_RETURN mode does this, what does BLOCK mode do? NOTHING,
 *
 *             YES, BLOCK mode does nothing, but waits until all msg handlers finish and write a struct emi_msg back to the sender by
 *             the help of emi_core, (BLOCK mode was designed to registered many times). However, this is totally wrong because once
 *             one of the registered msg handler finishes its work and goes back to emi_core, the emi_core write a struct emi_msg
 *             back to sender immediately, which will cause the sender function to return. As a result, other msg handlers may not
 *             finish at all when the sender has already returned.
 *
 *             REMEMBER this, EMI_MSG_MODE_BLOCK is a kind of exclusive and synchronised msg which can return an extra data or the
 *             msg handler return state back to the sender according to the programmer's design. Just like the old BLOCK RETURN mode,
 *             the old BLOCK mode is useless and has been deleted. If a msg is not defined explicitly as BLOCK mode, it must be a
 *             ~BLOCK one.
 *
 *         EMI_MSG_CMD_GET:     I don't even know what it is used for. to delete it loses nothing.
 *         EMI_MSG_FLAG_CREATE:  delete
 *
 *         EMI_MSG_CMD_SUCCEEDED: was changed into EMI_MSG_RET_SUCCEEDED, this is the FLAG recording the return state, if the FLAG
 *         is not set, ~SUCCEDDED (FAILED) it means.
 *
 */


/*
 * the default msg is  ~BLOCK, unless this FLAG was set.
 */
#define EMI_MSG_MODE_BLOCK            0x00000100

#define EMI_MSG_CMD_REGISTER        0x00004000
#define EMI_MSG_RET_SUCCEEDED        0x00010000

    eu32 count;                //the member is used for count the processes when several processes share one massage.it is internally,do not use it. useful in ~BLOCK msg.
    eu32 size;
    eu32 cmd;
    eu32 msg;
    char data[];
};

typedef int (*emi_func)(struct emi_msg *);


#endif
