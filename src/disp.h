/*--------------------------------------------------------------------
  dispray module.

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 
--------------------------------------------------------------------*/
#ifndef DISP_H
#define DISP_H

typedef struct dspfmt
{
	color_t col;
	char *str;
	struct dspfmt *next;
} dspfmt_t;

typedef struct
{
	int x, y;
	int sizex, sizey;
	void *vp;
	dspfmt_t *(*func)(void *vp, int a, int sizex, int sizey);
} dspreg_t;

#define	MAX_dspreg	16

typedef struct
{
	int sizex, sizey;
	int drp_num;
	dspreg_t *drp[MAX_dspreg];
	bool ff;
} dspall_t;

VAL dspall_t dspall;

dspfmt_t *dsp_fmtinit(const char *s, dspfmt_t *dfp);
void dsp_fmtsfin(dspfmt_t *dfp);
void dsp_regresize(dspreg_t *drp, int sizex, int sizey);
char *dsp_add(char *str, int length);
void dsp_regview(dspreg_t *drp);
dspreg_t *dsp_reginit();
int dsp_regexist(dspreg_t *drp);
void dsp_regadd(dspreg_t *drp);
void dsp_regrm(dspreg_t *drp);
void dsp_regfin(dspreg_t *drp);
void dsp_allinit();
void dsp_allview();

#endif
