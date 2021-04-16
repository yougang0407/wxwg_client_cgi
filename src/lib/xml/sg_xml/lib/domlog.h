/* 2002-07-09 add mask L_SCR to show log infomation on screen */

#ifndef __DOMLOG_H__
#define __DOMLOG_H__

#include <stdarg.h>

enum {
	L_DBG	= 0x01,
	L_INFO	= 0x02,
	L_NOTE	= 0x04,
	L_ERR	= 0x08,
	L_SCR	= 0x10,
};

void dom_log(int level, const char* fmt, va_list ap);

enum {
	LOG_FILESET = 1,
	LOG_FILEGET = 2,
	LOG_FILECLS = 3,
};

long dom_lfioctl(int cmd, char* file);
#define dom_lfset(file) dom_lfioctl(LOG_FILESET, file)
#define dom_lfget(file) dom_lfioctl(LOG_FILEGET, file)

int dom_llevel(int l);

#endif
