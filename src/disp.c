/*--------------------------------------------------------------------
  display module.

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 

	n8
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#include	"n8.h"
#include	"crt.h"

dspfmt_t *dsp_fmtinit(const char *s, dspfmt_t *dfp)
{
	dspfmt_t *dfpb;

	dfpb = mem_alloc(sizeof(dspfmt_t));
	if(dfp != NULL) {
		dfp->next = dfpb;
	}
	dfpb->col = AC_normal;
	dfpb->str = mem_alloc(strlen(s) + 1);
	strcpy(dfpb->str, s);

	dfpb->next = NULL;
	return dfpb;
}

void dsp_fmtsfin(dspfmt_t *dfp)
{
	dspfmt_t *next;
	while(dfp != NULL) {
		next = dfp->next;
		free(dfp->str);
		free(dfp);
		dfp = next;
	}
}

void dsp_regresize(dspreg_t *drp, int sizex, int sizey)
{
	int i, a;

	if(sizex > dspall.sizex) {
		sizex = dspall.sizex;
	}
	if(sizey > dspall.sizey) {
		sizey = dspall.sizey;
	}
	if(sizex != -1) {
		drp->sizex = sizex;
	}
	if(sizey != -1 || drp->sizey != sizey) {
		drp->sizey = sizey;
		a = drp->y + sizey - dspall.sizey;
		if(a > 0) {
			 drp->y -= a;
		}
	}
}

char *dsp_add(char *str, int length)
{
	while(length > 0 && *str != '\0') {
		int ch = (unsigned char)*str & 0xf0;
		if(ch >= 0xc0) {
			length--;
			str++;
			if(*str != '\0' && ch >= 0xe0) {
				str++;
				if(*str != '\0' && ch == 0xf0) {
					str++;
				}
			}
		}
		if(*str == '\0') {
			break;
		}
		length--;
		str++;
	}
	return str;
}

void dsp_regview(dspreg_t *drp)
{
	dspfmt_t *dfp, *dfp_b;
	char *p;
	int n, m, i;
	int disp_length;
	char spc_buf[LN_dspbuf + 1];

	for(i = 0 ; i < drp->sizey ; ++i) {
		dfp = drp->func(drp->vp, i, drp->sizex, drp->sizey);
		if(dfp == NULL) {
			continue;
		}

		dfp_b = dfp;
		p = dfp->str;
		n = drp->sizex;
		m = 0;

		memset(spc_buf, ' ', n);
		spc_buf[n] = '\0';

		for(;;) {
			if(*p == '\0') {
				 if(dfp->next == NULL) {
				 	p = spc_buf;
				} else {
				 	 dfp = dfp->next;
				 	 p = dfp->str;
				}
			}
			disp_length =  min(get_display_length(p), n - m);
			term_color(dfp->col);
			term_locate(drp->y + i, drp->x + m);
			widthputs(p, disp_length);
			m += disp_length;
			p = dsp_add(p, disp_length);
			if(m >= drp->sizex) {
				break;
			}
		}
		dsp_fmtsfin(dfp_b);
	}
}

dspreg_t *dsp_reginit()
{
	dspreg_t *drp;

	drp = (dspreg_t *)mem_alloc(sizeof(dspreg_t));
	drp->x = 0;
	drp->y = 0;
	drp->sizex = dspall.sizex;
	drp->sizey = dspall.sizey;
	drp->vp = NULL;
	return drp;
}

int dsp_regexist(dspreg_t *drp)
{
	int i;

	for(i = 0 ; i < dspall.drp_num ; ++i) {
		if(drp == dspall.drp[i]) {
			return i;
		}
	}
	return -1;
}

void dsp_regadd(dspreg_t *drp)
{
	if(dspall.drp_num >= MAX_dspreg || dsp_regexist(drp) != -1) {
		return;
	}
	dspall.drp[dspall.drp_num] = drp;
	++dspall.drp_num;
}

void dsp_regrm(dspreg_t *drp)
{
	int i;

	i = dsp_regexist(drp);
	if(i == -1) {
		return;
	}
	if(i + 1 < dspall.drp_num) {
		memcpy(&dspall.drp[i], &dspall.drp[i + 1], sizeof(dspreg_t *) * dspall.drp_num - i - 1);
	}
	--dspall.drp_num;
}

void dsp_regfin(dspreg_t *drp)
{
	dsp_regrm(drp);
	free(drp);
}

void dsp_allinit()
{
	dspall.drp_num = 0;
	dspall.sizex = term_sizex() - 1;
	dspall.sizey = GetMaxRow() + 1;	//!!
}

void dsp_allview()
{
	int i;

	for(i = 0 ; i < dspall.drp_num ; ++i) {
		dsp_regview(dspall.drp[i]);
	}
}

