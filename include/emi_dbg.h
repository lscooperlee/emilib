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

#include <syslog.h>

#define EMI_ERROR       LOG_ERR
#define EMI_WARNING     LOG_WARNING
#define EMI_INFO        LOG_INFO
#define EMI_DEBUG       LOG_DEBUG

#ifdef DEBUG

extern void debug_emi_msg(struct emi_msg *msg);

#define emilog_init()  openlog(NULL, 0, LOG_LOCAL0)

#define emilog(priority, format, arg...)                                            \
    do {                                                            \
        syslog(priority, "%s: %s: %d: "format, __FILE__, __func__, __LINE__, ## arg);         \
    } while (0)

#else

extern void debug_emi_msg(struct emi_msg *msg);

#define emilog_init()

#define emilog(priority, format, arg...)                                            \

#endif

#endif

