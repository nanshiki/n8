/*--------------------------------------------------------------------
    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#ifndef _SH_H_
#define	_SH_H_

#include	"sh_defs.h"
#include	"function.h"

typedef enum{
	KDM_func,
	KDM_macro
} kdm_t;

#define	MAX_args	4

typedef	struct
{
	int key1;
	int key2;
	int funcNo;

	char *args[MAX_args];

	kdm_t kdm;
} keydef_t;

extern int keydef_num(int r);
extern void keydef_init();
extern keydef_t *keydef_set(int r, kdm_t kdm, int n, int k1, int k2);
extern keydef_t *keydef_get(int r, int k1, int k2);

extern char *keyfcode[MAX_region][MAX_func];
extern void (*funclist[MAX_region][MAX_func])();

#define KF_normalcode 0x10000

extern char **keydef_args;

#endif
