/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:crt.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 

	n8
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#include "n8.h"
#include "crt.h"
#include "line.h"
#include "list.h"
#include "disp.h"
#include "block.h"
#include "cursor.h"
#include "lineedit.h"
#include <ctype.h>

/* !! */
int GetMinRow()
{
	return 1;
}

int GetMaxRow()
{
	return term_sizey() - 1;
}

int GetRowWidth()
{
	return term_sizey() - 1;
}

int GetMinCol()
{
	return 0;
}

int GetMaxCol()
{
	return term_sizex() - 1;
}

int GetColWidth()
{
	return term_sizex() - 1;
}

void widthputs(const char *s, size_t len)
{
	char buf[MAXLINESTR + 1];

	strjfcpy(buf, s, MAXLINESTR, len);
	term_puts(buf);
}

void crt_crmark()
{
	term_color(sysinfo.c_crmark);
	term_putch('!');
}

void crt_draw_proc(const char *s, crt_draw_t *gp)
{
	char buf[MAXEDITLINE + 1];
	char buf_dsp[MAXEDITLINE + 1], *p;
	bool cf, bf;
	int ln, sx, n, m;
	int x_st, x_ed;

	if(sysinfo.numberf) {
		term_locate(gp->dline, 0);
		term_color(sysinfo.c_linenum);
		term_printf("%5ld:", gp->line);
	}

	p = buf_dsp;
	cf = FALSE;
	bf = FALSE;
	if(s == NULL) {
		strcpy(buf_dsp, "~");
	} else {
		strcpy(buf, gp->line == GetLineOffset() ? csrle.buf : s);

		if(*s == '\0' || s[strlen(s) - 1] != '\n') {
			strcat(buf, "[EOF]");
		} else {
			cf = sysinfo.crmarkf;
		}
		if(*buf != '\0' && buf[strlen(buf) - 1] == '\n') {
			buf[strlen(buf) - 1] = '\0';
		}

		ln = le_regbuf(buf, p);

		sx = GetScroll();
		n = kanji_poscandsp(sx, p);
		if(sx > 0) {
			m = kanji_posbuf(n, p);
			if(m == ln) {
				sx = ln;
			} else {
				if(sx != n) {
					memset(p + m, ' ', kanji_countbuf(p[m]));
				}
				sx = kanji_posbuf(sx, p);
			}
			p += sx;
		}
		n = kanji_posbuf(kanji_poscandsp(GetColWidth() - NumWidth, p), p);
		if(p[n] != '\0') {
			cf = FALSE;
		}
		p[n] = '\0';

		if(cf && n == GetColWidth()) {
			cf = FALSE;
		}
	}
	term_locate(gp->dline, NumWidth);

	if(!block_range(gp->line, &gp->bm, &x_st, &x_ed)) {
		term_color_normal();
		term_puts(p);
	} else {
		n = 0;

		x_st = kanji_posdsp(x_st, buf) - sx;
		x_ed = kanji_posdsp(x_ed, buf) - sx;
		if(x_st < 0) {
			x_st = 0;
		}

		if(gp->bm.blkm == BLKM_x && gp->line == GetLineOffset() && gp->bm.x_st == GetBufferOffset() && x_st < x_ed) {
			++x_st;
		}

		if(x_st > 0) {
			if(x_st > strlen(p)) {
				x_st = strlen(p);
			}
			ln = utf8_disp_copy(buf, p, x_st);
			p += strlen(buf);
			x_ed -= ln;
			n = ln;

			term_color_normal();
			term_puts(buf);
		}

		term_color(sysinfo.c_block);
		if(x_ed <= strlen(p)) {
			ln = utf8_disp_copy(buf, p, x_ed);
			p += strlen(buf);
			n += ln;
			term_puts(buf);

			term_color_normal();
		}
		term_puts(p);

		if(gp->bm.blkm == BLKM_x && cf && *p == '\0') {
			 cf = FALSE;
			 term_color(sysinfo.c_block);
			 term_putch('!');
		}

		if(gp->bm.blkm == BLKM_y) {
			 cf = FALSE;
			 term_color(sysinfo.c_block);
			 widthputs("", GetColWidth() - NumWidth - (strlen(p) + n));
		}
	}
	if(cf) {
		crt_crmark();
	}
	term_color_normal();
	term_clrtoe();

	++gp->line;
	++gp->dline;
}

void CrtDrawAll()
{
	crt_draw_t cd;
	int sline;

	block_set(&cd.bm);
	if(csrle.l_sx == csrle.sx) {
		sline = csrse.l_sy-csr_getsy();
		if(abs(sline) < GetRowWidth() - 3) { // !!
			term_locate(GetMinRow(), 0);
			term_scroll(sline);
		}
	}

	cd.dline = GetMinRow();
	cd.line = csr_getsy() + cd.dline;
	lists_proc((void (*)(const char *, void *))crt_draw_proc, &cd, cd.line, csr_getsy() + GetRowWidth());
	csrle.l_sx = csrle.sx;
	csrse.l_sy = csr_getsy();
	csrse.l_cy = csrse.cy;
}

void DeleteAndDraw()
{
	term_locate(GetRow(), 0);
	term_scroll(-1);
}

void InsertAndDraw()
{
	term_locate(GetRow(), 0);
	term_scroll(1);
}

void RefreshMessage()
{
	if(OnMessage_Flag) {
		OnMessage_Flag = FALSE;
		*sysinfo.systemline = '\0';

		if(sysinfo.sl_drp != NULL) {
			dsp_regfin(sysinfo.sl_drp);
			sysinfo.sl_drp = NULL;
		}
	}
}


/*-------------------------------------------------------------------
	Put Guide Line

	2000/03/11 by Mia	upd
		If GetLastNumber( ) == 0, causes SIGFPE and dumps core.
		This occurs when filer is kicked with no file opened.
------------------------------------------------------------------*/
#define LN_guide 34

dspfmt_t *dspreg_guide(void *vp, int a, int sizex, int sizey)
{
	dspfmt_t *dfp, *dfpb;
	char *p, tmp[MAXEDITLINE * 2 + 1], tmp2[MAXEDITLINE + 1];
	int length;
	unsigned long cn;

	char kc_char[] = {'E', 'J', 'S', 'U', 'u'};
	char rm_char[] = {'L', '+', 'C'};

	if(CurrentFileNo < 0 || CurrentFileNo >= MAX_edbuf) {
		return NULL;
	}
	if(sizex <= LN_guide) {
		dfp = dsp_fmtinit("", NULL);
		dfp->col = AC_reverse;
		return dfp;
	}
	if(edbuf[CurrentFileNo].pm) {
		dfp = dsp_fmtinit("S", NULL);
	} else {
		dfp = dsp_fmtinit("P", NULL);
		dfp->col = AC_reverse;
	}
	dfpb = dfp;
	if(sysinfo.overwritef) {
		dfp = dsp_fmtinit("R", dfp);
	} else {
		dfp = dsp_fmtinit("i", dfp);
		dfp->col = AC_reverse;
	}
	if(GetBufferOffset() >= strlen(csrle.buf)) {
		cn = 0;
	} else {
		get_utf8_code(&csrle.buf[GetBufferOffset()], &cn);
	}
	if(*sysinfo.doublekey != '\0') {
		strcpy(tmp2, sysinfo.doublekey);
	} else {
		sprintf(tmp2, "%08lx", cn);
	}
	p = edbuf[CurrentFileNo].path;
	length = strlen(p);
	if(sizex - LN_guide < length) {
		p += length - (sizex - LN_guide);
	}
	sprintf(tmp, " %05ld/%3ld%%:%03d[%8.8s]%6ld %2d%c%c%c%s"
		, GetLineOffset()
		, GetLineOffset() == 1 || GetLastNumber( ) == 0 ? 0 : GetLineOffset() * 100 / GetLastNumber()
		, le_getcsx(&csrle)
		, tmp2
		, csrse.bytes+GetLastNumber() - 1
		, CurrentFileNo + 1
		, kc_char[edbuf[CurrentFileNo].kc]
		, rm_char[edbuf[CurrentFileNo].rm]
		, edbuf[CurrentFileNo].cf ? '*' : ':'
		, p);

	dfp = dsp_fmtinit(tmp, dfp);
	dfp->col = AC_reverse;

	return dfpb;
}

void putDoubleKey(int key)
{
	sprintf(sysinfo.doublekey, "   ^%c   ", key + '@');
}

void delDoubleKey()
{
	*sysinfo.doublekey = '\0';
}

void system_guide_init()
{
	dspreg_t *drp;

	drp = dsp_reginit();
	drp->sizey = 1;
	drp->func = dspreg_guide;

	dsp_regadd(drp);
}

void system_guide()
{
	dsp_allview();
}

dspfmt_t *dspreg_sysmsg(void *vp, int a, int sizex, int sizey)
{
	dspfmt_t *dfp;

	dfp = dsp_fmtinit(sysinfo.systemline, NULL);
	dfp->col = sysinfo.c_sysmsg;

	return dfp;
}


void system_msg(const char *buffer)	// 暫定的dsp化
{
	OnMessage_Flag = TRUE;

	strcpy(sysinfo.systemline, buffer);

	if(*buffer == '\0') {
		term_locate(GetMaxRow(), 0);
		term_clrtoe();					// !!
		return;
	}

	if(sysinfo.sl_drp != NULL) {
		dsp_regfin(sysinfo.sl_drp);
	}

	sysinfo.sl_drp = dsp_reginit();
	sysinfo.sl_drp->y = dspall.sizey - 1;
	sysinfo.sl_drp->sizey = 1;
	sysinfo.sl_drp->func = dspreg_sysmsg;

	dsp_regadd(sysinfo.sl_drp);

	dsp_regview(sysinfo.sl_drp);

	term_locate(GetMaxRow(), get_display_length(buffer));
	term_color_normal();
}

