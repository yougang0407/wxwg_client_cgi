#ifndef __GB_H
#define __GB_H


/*
	gb ranges from a1a1 to f7fe inclusive
	we use a kuten-like mapping the above range to 101-8794
*/
#define		GBMAX	8795
extern long tabgb[GBMAX];	/* runes indexed by gb ordinal */

void code_hndl_gbtab_init(void);
int code_hndl_gbmap_init(int* m);
int code_hndl_gb_2unicot(void* data, const char *p);


/*
	these is used in per utf8 to gb convert session
*/
#define NRUNE	65536
#define UTFmax		6		/* maximum bytes per rune */
#define N		10000		/* just blocking */
typedef unsigned short Rune;	/* 16 bits */
struct __utf_2gb_mngr {
	char obuf[UTFmax*N];	/* maximum bloat from N runes */
	Rune runes[N];
};

char* code_hndl_utf_2gb(struct __utf_2gb_mngr* u2g, 
							const char* text, int len);

#endif
