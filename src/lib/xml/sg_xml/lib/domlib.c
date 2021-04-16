/* add legal param check in build_string() */

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <conio.h>
#endif

#include <stdarg.h>
#include <string.h>

#ifdef WIN32
#include "sg_domtree.h"
#include "dtd_chk.h"
#else
#include "sg_domtree.h"
#include "dtd_chk.h"
#endif


void eprintf(char* fmt, ...)
{
	va_list ap;

	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
	va_end(ap);
	exit(-1);
}


char* estrdup(const char* str)
{
	if(str == NULL)
		return NULL;
	
	return strdup(str);
} 


/* cut string's left char */
char* eltrim(const char* str)
{
	char* p = (char*)str;
	char t[2] = {'\0','\0'};	
	while(*p!='\0') {
		t[0] = *p;
	    if(strstr("\r\n\t ",(char*)t) == NULL) break;
		p++;
	}
	return p;
}


/* cut string's front and tail space char */
int alltrim(char* str, long *ps, long *pe)
{
	char *p;

	*ps = *pe = 0;
	for(p = str; p <= str + strlen(str)-1; p++) {
		if(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
			continue;
		*ps = (long)p;
		break;
	}
	if(*ps == 0)
		return 0;
	
	for(p = str + strlen(str)-1; p >= (char *)ps; p--) {
		if(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
			continue;
		*pe = (long)p;
		break;
	}
	if(*pe == 0)
		return 0;

	return 1;
}


//Updated synchronously 
void build_string(int flag, char** pp, char* str)
{
	static int max = 0;
	static int cur_len = 0;
	int next_len = 0;
	int str_len = 0;
#define MIN_HOP 500

	if(pp == NULL || str == NULL)
		return;

	if(flag == 1) {
		max = 0;
		if(*pp != NULL)
			free(*pp);
		*pp = NULL;
	}

	if(*pp == NULL) {
		*pp = (char*) malloc(MIN_HOP);
		if(*pp == NULL) {
			printf("malloc error for build_string\n");
			exit(0);
		}
		max = MIN_HOP;
		cur_len = 0;
		memset(*pp,0,max);
	}

	//next_len = strlen(str) + strlen(*pp) + 1;
	str_len = strlen( str );
	next_len = str_len + cur_len + 1;
	if(next_len > max) {
		int next_hop = (int) (next_len / max) + 1;
		char* t = (char*) malloc(max * next_hop);
		if(t == NULL) {
			printf("malloc error for rebuild_string\n");
			exit(0);
		}
		max = max * next_hop;
		memset(t,0,max);
		strcpy(t,*pp);
		free(*pp);
		*pp = t;
	}
	//strcat(*pp, str);
	strcpy( *pp + cur_len, str );
	cur_len += str_len;
}


/* wether an validate xml name 
to decide wether an ID name use this
*/
int isvalidatexmlname(const char* name)
{
	char* p = (char*) name;
	const char VALIDATE_CHAR_SET1[] = "abcdefghijklmnopqrstuvwxyz"	;
	const char VALIDATE_CHAR_SET2[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"	;
	const char VALIDATE_CHAR_SET3[] = "0123456789";
	const char VALIDATE_CHAR_SET4[] = "_";
	
	/* WHAT's an validate xml name ?
	[] must begin with 26 char
	[] only include 26 char, 10 number, and bottom line "_"
	[] can NOT seperate with space char
	*/

	if(name == NULL || name[0] == '\0')
		return 0;

	if(!(strchr(VALIDATE_CHAR_SET1,(int) *p) ||
		strchr(VALIDATE_CHAR_SET2,(int) *p)))
		return 0;

	p ++;
	while(*p) {
		if(!(strchr(VALIDATE_CHAR_SET1, (int) *p) ||
			strchr(VALIDATE_CHAR_SET2, (int) *p) ||
			strchr(VALIDATE_CHAR_SET3, (int) *p) ||
			strchr(VALIDATE_CHAR_SET4, (int) *p))) 
			return 0;
		p ++;
	}

	return 1;	
}


/* to decide wether an NMTOKEN use this 
 */
int isxmlname(const char* name)
{
	char* p = (char*) name;
	const char VALIDATE_CHAR_SET1[] = "abcdefghijklmnopqrstuvwxyz"	;
	const char VALIDATE_CHAR_SET2[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"	;
	const char VALIDATE_CHAR_SET3[] = "0123456789";
	const char VALIDATE_CHAR_SET4[] =  "_-.";
	
	/* WHAT's an xml name ?
	[] must begin with 26 char or bottom line "_"
	[] only include 26 char, 10 number, "_", "-" and "."
	[] can NOT seperate with space char
	*/

	if(name == NULL || name[0] == '\0')
		return 0;

	if(!(strchr(VALIDATE_CHAR_SET1,(int) *p) ||
		strchr(VALIDATE_CHAR_SET2,(int) *p) ||
		strchr("_",(int) *p)))
		return 0;

	p ++;
	while(*p) {
		if(!(strchr(VALIDATE_CHAR_SET1, (int) *p) ||
			strchr(VALIDATE_CHAR_SET2, (int) *p) ||
			strchr(VALIDATE_CHAR_SET3, (int) *p) ||
			strchr(VALIDATE_CHAR_SET4, (int) *p))) 
			return 0;
		p ++;
	}

	return 1;	
}
