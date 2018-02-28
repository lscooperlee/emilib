
#ifndef    __DEBUG_H__
#define    __DEBUG_H__

#include <syslog.h>
#include <stdio.h>

#define EMI_ERROR       LOG_ERR
#define EMI_WARNING     LOG_WARNING
#define EMI_INFO        LOG_INFO
#define EMI_DEBUG       LOG_DEBUG

#define emi_printf      printf

struct emi_msg;

extern void debug_emi_msg(struct emi_msg *msg);

#ifdef DEBUG

#if defined DBG_STDOUT

#define DEDAULT_LOGLEVEL LOG_DEBUG

#define emilog_init()

#define emilog(priority, format, arg...)                                                       \
    do {                                                                                       \
        if(priority <= DEDAULT_LOGLEVEL) {                                                     \
            fprintf(stderr, "%s: %s: %d: "#format, __FILE__, __func__, __LINE__, ## arg);      \
        } \
    } while (0)

#else //syslog by default

#define emilog_init()  openlog(NULL, 0, LOG_LOCAL0)

#define emilog(priority, format, arg...)                                            \
    do {                                                            \
        syslog(priority, "%s: %s: %d: "#format, __FILE__, __func__, __LINE__, ## arg);         \
    } while (0)

#endif

#else //no debug

#define emilog_init()

#define emilog(priority, format, arg...)                                            

#endif

#endif

