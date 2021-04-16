#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "domlog.h"

void dom_log(int level, const char* fmt, va_list ap)
{
	static FILE *fp = NULL;
	char info[1024];
	static char* file = NULL;
	int llevel = dom_llevel(0);

	if(!(llevel & level))
		return;

	/* get head info */
	memset(info,0,sizeof(info));
	if(level & L_DBG)
		sprintf(info,"[debug] : ");
	else if(level & L_INFO)
		sprintf(info,"[info] : ");
	else if(level & L_NOTE)
		sprintf(info,"[notice] : ");
	else if(level & L_ERR)
		sprintf(info,"[error] : ");
	else 
		sprintf(info,"[info] : ");

	vsprintf(&info[strlen(info)], fmt, ap);
	
	if(level & L_SCR)
		fprintf(stderr,info);
	
	/* open log file */
	if((file = (char*)dom_lfget(NULL)) != NULL) {
		if(fp == NULL) {
			fp = fopen(file,"a");
		}
		if(fp != NULL) {
			fprintf(fp,info);
			fclose(fp);
			fp = NULL;
		}
	}

	return;
}

long dom_lfioctl(int cmd, char* v)
{
	static char* logfile = NULL;

	switch(cmd) {
	case LOG_FILEGET:
		return (long) logfile;
	case LOG_FILESET:
		logfile = v;
		if(logfile != NULL)
			unlink(logfile);
		break;
	case LOG_FILECLS:
		if(unlink(v) != 0)
			return 0;
		break;
	default:
		return 0;
	}
	
	return 1;
}

int dom_llevel(int l)
{
	static int level = 0;
	int pre;

	if(l == 0)
		return level;
	
	pre = level;
	level = l;

	return pre;
}
