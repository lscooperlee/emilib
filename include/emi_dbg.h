
#ifndef    __DEBUG_H__
#define    __DEBUG_H__

#include <syslog.h>
#include <stdio.h>

#define EMI_ERROR       LOG_ERR
#define EMI_WARNING     LOG_WARNING
#define EMI_INFO        LOG_INFO
#define EMI_DEBUG       LOG_DEBUG
#define DEDAULT_LOGLEVEL LOG_DEBUG

#define emi_printf      printf

#ifdef DEBUG

#if defined DBG_STDOUT

#define emilog_init()

#include <string.h>
#include <stdarg.h>

extern void _emilog(int priority, char *buf, const char *format, ...);
#define emilog(priority, format, arg...)                                                    \
    do {                                                                                    \
        char ___buf[1024]; \
        sprintf(___buf, "%s: %s: %d: ", __FILE__, __func__, __LINE__); \
        _emilog(priority, ___buf, format, ##arg); \
    }while(0)

#else //syslog by default

#define emilog_init()  openlog(NULL, 0, LOG_LOCAL0)

#define emilog(priority, format, arg...)                                                    \
    do {                                                                                    \
        syslog(priority, "%s: %s: %d: "#format, __FILE__, __func__, __LINE__, ## arg);      \
    } while (0)

#endif

#else //no debug

#define emilog_init()

#define emilog(priority, format, arg...)                                            

#endif

#endif

