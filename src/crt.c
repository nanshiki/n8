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
#include "filer.h"
#include "lineedit.h"
#include "keyf.h"
#include "sh.h"
#include <ctype.h>

#define	SPLIT_HORIZON_MIN	5
#define	SPLIT_VERTICAL_MIN	35

enum {
	systemGuide1,
	systemGuide2,
	systemVertical,

	systemMax
};

enum {
	splitDataHorizon,
	splitDataVertical,
	splitDataMax
};

static dspreg_t *system_drp[systemMax];
static int current_under;
static int split_start[splitDataMax];
static int split_move[splitDataMax];
static int split_size[splitDataMax][splitHalfMax];

int GetMinRow()
{
	return 1;
}

int GetMaxRow()
{
	if(split_mode == splitHorizon && CurrentFileNo == split_file_no[splitLower]) {
		// 下半分
		return split_size[splitDataHorizon][splitLower];
	}
	if(split_mode == splitHorizon && CurrentFileNo == split_file_no[splitUpper]) {
		// 上半分
		return split_size[splitDataHorizon][splitUpper];
	}
	return term_sizey() - 1;
}

int GetRowWidth()
{
	if(split_mode == splitHorizon) {
		// 水平分割
		if(CurrentFileNo == split_file_no[splitLower]) {
			// 下半分
			return split_size[splitDataHorizon][splitLower];
		}
		return split_size[splitDataHorizon][splitUpper];
	}
	return term_sizey() - 1;
}

int GetMinCol()
{
	if(split_mode == splitVertical && CurrentFileNo == split_file_no[splitRight]) {
		// 右半分
		return split_start[splitDataVertical];
	}
	return 0;
}

int GetMaxCol()
{
	if(split_mode == splitVertical && CurrentFileNo == split_file_no[splitRight]) {
		// 右半分
		return split_size[splitDataVertical][splitRight];
	}
	if(split_mode == splitVertical && CurrentFileNo == split_file_no[splitLeft]) {
		// 左半分
		return split_size[splitDataVertical][splitLeft];
	}
	return term_sizex() - 1;
}

int GetColWidth()
{
	if(split_mode == splitVertical) {
		// 垂直分割
		if(CurrentFileNo == split_file_no[splitRight]) {
			// 右半分
			return split_size[splitDataVertical][splitRight];
		}
		// 左半分
		return split_size[splitDataVertical][splitLeft];
	}
	return term_sizex() - 1;
}

void widthputs(const char *s, size_t len)
{
	char buf[MAXLINESTR + 1];

	strjfcpy(buf, s, MAXLINESTR, len, TRUE);
	if(term_puts(buf, NULL) && sysinfo.nfdf) {
		term_redraw_line();
	}
}

void crt_crmark(int under)
{
	term_color(sysinfo.c_crmark);
	if(under) {
		term_color_underline();
	}
	term_putch('!');
}

void crt_draw_proc(const char *s, crt_draw_t *gp)
{
	char buf[MAXEDITLINE + 1];
	char buf_dsp[MAXEDITLINE + 1], *p;
	char buf_ac[MAXEDITLINE + 1], *ac;
	bool cf, bf;
	int ln, sx, n, m, len;
	int x_st, x_ed;
	int under = FALSE;
	int y = GetRow();

	if(split_mode == splitHorizon && CurrentFileNo == split_file_no[splitLower]) {
		y += split_start[splitDataHorizon];
	}
	if(sysinfo.underlinef && gp->dline == y && current_under) {
		under = TRUE;
	}

	if(sysinfo.numberf) {
		term_locate(gp->dline, GetMinCol());
		term_color(sysinfo.c_linenum);
		term_printf("%5ld:", gp->line);
	}

	p = buf_dsp;
	ac = buf_ac;
	cf = FALSE;
	bf = FALSE;
	if(s == NULL) {
		strcpy(buf_dsp, "~");
	} else {
		strcpy(buf, gp->line == GetLineOffset() ? csrle.buf : s);
		if(*s == '\0' || s[strlen(s) - 1] != '\n') {
			if(sysinfo.eoff) {
				strcat(buf, "[EOF]");
			}
		} else {
			cf = sysinfo.crmarkf;
		}
		if(*buf != '\0' && buf[strlen(buf) - 1] == '\n') {
			buf[strlen(buf) - 1] = '\0';
		}

		ln = le_regbuf(buf, p, ac);

		sx = GetScroll();
		n = kanji_poscandsp(sx, p);
		if(sx > 0) {
			m = kanji_posbuf(n, p);
			if(m == ln) {
				sx = ln;
			} else {
				if(sx != n) {
					memset(p + m, ' ', kanji_countbuf(&p[m]));
				}
				sx = kanji_posbuf(sx, p);
			}
			ac += sx;
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
	term_locate(gp->dline, NumWidth + GetMinCol());
	if(!block_range(gp->line, &gp->bm, &x_st, &x_ed)) {
		term_color_normal();
		if(under) {
			term_color_underline();
		}
		if(term_puts(p, ac) && sysinfo.nfdf) {
			term_redraw_line();
		}
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
			len = strlen(buf);
			p += len;
			x_ed -= ln;
			n = ln;

			term_color_normal();
			if(under) {
				term_color_underline();
			}
			if(term_puts(buf, ac) && sysinfo.nfdf) {
				term_redraw_line();
			}
			ac += len;
		}

		term_color(sysinfo.c_block);
		if(x_ed <= strlen(p)) {
			ln = utf8_disp_copy(buf, p, x_ed);
			len = strlen(buf);
			p += len;
			ac += len;
			n += ln;
			if(term_puts(buf, NULL) && sysinfo.nfdf) {
				term_redraw_line();
			}
			term_color_normal();
			if(under) {
				term_color_underline();
			}
		}
		if(term_puts(p, ac) && sysinfo.nfdf) {
			term_redraw_line();
		}

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
		crt_crmark(under);
	}
	term_color_normal();
	term_clrtoe(under ? AC_under : AC_normal);
	term_color_normal();

	++gp->line;
	++gp->dline;
}

void crt_draw_file()
{
	crt_draw_t cd;
	int sline;

	cd.dline = 1;
	if(split_mode == splitHorizon && CurrentFileNo == split_file_no[splitLower]) {
		cd.dline = split_start[splitDataHorizon];
	}

	block_set(&cd.bm);
	if(csrle.l_sx == csrle.sx) {
		sline = csrse.l_sy - csr_getsy();
		if(abs(sline) < GetRowWidth() - 3) {
			term_locate(cd.dline, 0);
			term_scroll(sline);
		}
	}

	if(split_mode == splitHorizon && CurrentFileNo == split_file_no[splitLower]) {
		cd.dline++;
	}
	cd.line = csr_getsy() + 1;
	lists_proc((void (*)(const char *, void *))crt_draw_proc, &cd, cd.line, csr_getsy() + GetRowWidth());
	csrle.l_sx = csrle.sx;
	csrse.l_sy = csr_getsy();
	csrse.l_cy = csrse.cy;
}

void CrtDrawAll()
{
	if(CurrentFileNo < 0 || CurrentFileNo >= MAX_edbuf) {
		term_cls();
		return;
	}
	if(split_mode == splitNone) {
		current_under = TRUE;
		crt_draw_file();
	} else {
		int keep;

		keep = CurrentFileNo;

		current_under = (keep == split_file_no[splitFirst]);
		CurrentFileNo = split_file_no[splitFirst];
		crt_draw_file();

		current_under = (keep == split_file_no[splitSecond]);
		CurrentFileNo = split_file_no[splitSecond];
		crt_draw_file();

		CurrentFileNo = keep;
	}
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
		system_msg("");

		if(sysinfo.sl_drp != NULL) {
			dsp_regfin(sysinfo.sl_drp);
			sysinfo.sl_drp = NULL;
		}
	}
}


/*-------------------------------------------------------------------
	Put Guide Line
------------------------------------------------------------------*/
#define LN_guide 34

struct {
	char *text;
	int attr;
} filer_menu[] = {
	{ " ", 0 },
	{ "F", AC_bold },
	{ "ile  ", 0 },
	{ "D", AC_bold },
	{ "ir  ", 0 },
	{ "M", AC_bold },
	{ "ask  ", 0 },
	{ "P", AC_bold },
	{ "ath  ", 0 },
	{ "S", AC_bold },
	{ "ort  ", 0 },
	{ "W", AC_bold },
	{ "indow  ", 0 },
	{ "A", AC_bold },
	{ "ll  ", 0 },
	{ NULL, 0 }
};

static char kc_char[] = {'E', 'J', 'S', 'U', 'u'};
static char rm_char[] = {'L', '+', 'C'};

dspfmt_t *display_guide(int sizex)
{
	dspfmt_t *dfp, *dfpb;
	char *p, tmp[MAXEDITLINE * 2 + 1];
	char mark;
	int length;
	int percent;
	color_t attr = edbuf[CurrentFileNo].readonly ? sysinfo.c_readonly : sysinfo.c_statusbar;

	if(CurrentFileNo < 0 || CurrentFileNo >= MAX_edbuf) {
		return NULL;
	}
	if(sizex <= LN_guide) {
		dfp = dsp_fmtinit("", NULL);
		dfp->col = sysinfo.c_statusbar;
		return dfp;
	}
	if(edbuf[CurrentFileNo].pm) {
		dfp = dsp_fmtinit("S", NULL);
		dfp->col = edbuf[CurrentFileNo].readonly ? AC_color(sysinfo.c_readonly) : 0;
	} else {
		dfp = dsp_fmtinit("P", NULL);
		dfp->col = attr;
	}
	dfpb = dfp;
	if(sysinfo.overwritef) {
		dfp = dsp_fmtinit("o", dfp);
		dfp->col = edbuf[CurrentFileNo].readonly ? AC_color(sysinfo.c_readonly) : 0;
	} else {
		dfp = dsp_fmtinit("i", dfp);
		dfp->col = attr;
	}
	if(split_mode != splitVertical) {
		dfp = dsp_fmtinit(" ", dfp);
		dfp->col = attr;
		dfp = dsp_fmtinit("[]", dfp);
		if(edbuf[CurrentFileNo].block.blkm != BLKM_none) {
			dfp->col = edbuf[CurrentFileNo].readonly ? AC_color(sysinfo.c_readonly) : 0;
		} else {
			dfp->col = attr;
		}
	}
	p = edbuf[CurrentFileNo].path;
	length = strlen(p);
	if(sizex - LN_guide < length) {
		p += length - (sizex - LN_guide);
	}
	sprintf(tmp, " %5ld:%-3d ", GetLineOffset(), le_getcsx(&csrle) + 1);
	dfp = dsp_fmtinit(tmp, dfp);
	dfp->col = attr;

	percent = GetLineOffset() == 1 || GetLastNumber( ) == 0 ? 0 : GetLineOffset() * 100 / GetLastNumber();
	if(sysinfo.systeminfof) {
		if(split_mode != splitVertical) {
			unsigned long code;
			if(GetBufferOffset() >= strlen(csrle.buf)) {
				code = 0;
			} else {
				get_utf8_code(&csrle.buf[GetBufferOffset()], &code);
			}
			sprintf(tmp, "[%8lX] %6ld ", code, csrse.bytes + GetLastNumber() - 1);
			dfp = dsp_fmtinit(tmp, dfp);
			dfp->col = attr;
		}
		sprintf(tmp, "%3d%%%s", percent, (split_mode == splitVertical) ? "   " : "");
		dfp = dsp_fmtinit(tmp, dfp);
		dfp->col = attr;
	} else {
		int n, w[2];

		if(percent >= 100) {
			percent = 99;
		}
		w[0] = percent / ((split_mode == splitVertical) ? 20 : 5);
		w[1] = ((split_mode == splitVertical) ? 4 : 19) - w[0];
		dfp = dsp_fmtinit("|", dfp);
		dfp->col = attr;
		for(n = 0 ; n < 2 ; n++) {
			if(w[n] > 0) {
				tmp[w[n]--] = '\0';
				while(w[n] >= 0) {
					tmp[w[n]] = ' ';
					w[n]--;
				}
				dfp = dsp_fmtinit(tmp, dfp);
				dfp->col = attr;
			}
			if(n == 0) {
				dfp = dsp_fmtinit(" ", dfp);
				dfp->col = AC_normal;
			}
		}
		dfp = dsp_fmtinit("|", dfp);
		dfp->col = attr;
	}
	mark = edbuf[CurrentFileNo].readonly ? 'R' : edbuf[CurrentFileNo].cf ? '*' : ' ';
	if(split_mode == splitVertical) {
		char *pn;
		if((pn = strrchr(edbuf[CurrentFileNo].path, '/')) != NULL) {
			p = pn + 1;
		}
	}
	sprintf(tmp, "%2d%c%c%c %s"
		, CurrentFileNo + 1
		, kc_char[edbuf[CurrentFileNo].kc]
		, rm_char[edbuf[CurrentFileNo].rm]
		, mark
		, p);
	dfp = dsp_fmtinit(tmp, dfp);
	dfp->col = attr;

	return dfpb;
}

dspfmt_t *dspreg_guide(void *vp, int a, int sizex, int sizey)
{
	dspfmt_t *dfpb;
	if(eff_check_open()) {
		dspfmt_t *dfp;
		int no;
		dfp = NULL;
		for(no = 0 ; filer_menu[no].text != NULL ; no++) {
			dfp = dsp_fmtinit(filer_menu[no].text, dfp);
			dfp->col = sysinfo.c_statusbar | filer_menu[no].attr;
			if(no == 0) {
				dfpb = dfp;
			}
		}
	} else {
		int keep;

		keep = CurrentFileNo;
		if(split_mode != splitNone) {
			CurrentFileNo = split_file_no[splitFirst];
		}
		dfpb = display_guide(sizex);
		CurrentFileNo = keep;
	}
	return dfpb;
}

dspfmt_t *dspreg_guide2(void *vp, int a, int sizex, int sizey)
{
	dspfmt_t *dfpb;
	int keep = CurrentFileNo;

	if(eff_check_open() || split_mode == splitNone) {
		return NULL;
	} else if(split_mode == splitVertical) {
		CurrentFileNo = split_file_no[splitRight];
		system_drp[systemGuide2]->y = 0;
		system_drp[systemGuide2]->x = split_start[splitDataVertical];
		system_drp[systemGuide2]->sizex = split_size[splitDataVertical][splitRight] + 1;
	} else if(split_mode == splitHorizon) {
		CurrentFileNo = split_file_no[splitLower];
		system_drp[systemGuide2]->y = split_start[splitDataHorizon];
		system_drp[systemGuide2]->x = 0;
		system_drp[systemGuide2]->sizex = term_sizex();
	}
	dfpb = display_guide(sizex);
	CurrentFileNo = keep;
	return dfpb;
}

dspfmt_t *dspreg_vertical(void *vp, int a, int sizex, int sizey)
{
	dspfmt_t *dfp, *dfpb;

	if(eff_check_open() || split_mode != splitVertical) {
		return NULL;
	}
	dfp = dsp_fmtinit("|", NULL);
	dfp->col = AC_normal;
	dfpb = dfp;
	while(sizey > 1) {
		dfp = dsp_fmtinit("|", dfp);
		dfp->col = AC_normal;
		sizey--;
	}
	return dfpb;
}

void putDoubleKey(int key)
{
	sprintf(sysinfo.doublekey, "^%c", key + '@');
	system_msg(sysinfo.doublekey);
}

void delDoubleKey()
{
	*sysinfo.doublekey = '\0';
}

void system_guide_init()
{
	system_drp[systemGuide1] = dsp_reginit();
	system_drp[systemGuide1]->sizey = 1;
	system_drp[systemGuide1]->func = dspreg_guide;
	dsp_regadd(system_drp[systemGuide1]);

	system_drp[systemGuide2] = dsp_reginit();
	system_drp[systemGuide2]->y = term_sizey() / 2;
	system_drp[systemGuide2]->sizey = 1;
	system_drp[systemGuide2]->func = dspreg_guide2;
	dsp_regadd(system_drp[systemGuide2]);

	system_drp[systemVertical] = dsp_reginit();
	system_drp[systemVertical]->sizex = 1;
	system_drp[systemVertical]->sizey = term_sizey();
	system_drp[systemVertical]->func = dspreg_vertical;
	dsp_regadd(system_drp[systemVertical]);
}

void system_guide_reinit()
{
	system_drp[systemGuide1]->sizex = term_sizex() - 1;
	if(split_mode == splitHorizon) {
		split_start[splitDataHorizon] = term_sizey() / 2;
		split_size[splitDataHorizon][splitUpper] = split_start[splitDataHorizon] - 1;
		split_size[splitDataHorizon][splitLower] = split_start[splitDataHorizon] - 1;
		split_start[splitDataHorizon] += split_move[splitDataHorizon];
		split_size[splitDataHorizon][splitUpper] += split_move[splitDataHorizon];
		split_size[splitDataHorizon][splitLower] -= split_move[splitDataHorizon];
	} else {
		split_start[splitDataVertical] = term_sizex() / 2;
		split_size[splitDataVertical][splitLeft] = split_start[splitDataVertical] - 1;
		split_size[splitDataVertical][splitRight] = split_start[splitDataVertical] - 1;
		split_start[splitDataVertical] += split_move[splitDataVertical];
		split_size[splitDataVertical][splitLeft] += split_move[splitDataVertical];
		split_size[splitDataVertical][splitRight] -= split_move[splitDataVertical];
		system_drp[systemVertical]->x = split_start[splitDataVertical] - 1;
	}
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


void system_msg(const char *buffer)
{
	OnMessage_Flag = TRUE;

	strcpy(sysinfo.systemline, buffer);

	if(*buffer == '\0') {
		term_locate(term_sizey() - 1, 0);
		term_clrtoe(AC_normal);
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

	term_locate(term_sizey() - 1, get_display_length(buffer));
	term_color_normal();
}

int get_split_start(int split)
{
	return split_start[(split == splitHorizon) ? splitDataHorizon : splitDataVertical];
}

SHELL void op_file_splitmove()
{
	if(split_mode != splitNone) {
 		int key;
		int size[2];
		int start, move;

		if(split_mode == splitHorizon) {
			start = split_start[splitDataHorizon];
			move = split_move[splitDataHorizon];
			size[splitUpper] = split_size[splitDataHorizon][splitUpper];
			size[splitLower] = split_size[splitDataHorizon][splitLower];
		} else {
			start = split_start[splitDataVertical];
			move = split_move[splitDataVertical];
			size[splitLeft] = split_size[splitDataVertical][splitLeft];
			size[splitRight] = split_size[splitDataVertical][splitRight];
		}
		system_msg(SPLIT_MOVE_MSG);
		while(1) {
			key = get_keyf(1);
			if(split_mode == splitHorizon) {
				if(key == KF_SysCursorup) {
					if(split_size[splitDataHorizon][splitUpper] > SPLIT_HORIZON_MIN) {
						split_move[splitDataHorizon]--;
						split_start[splitDataHorizon]--;
						split_size[splitDataHorizon][splitUpper]--;
						split_size[splitDataHorizon][splitLower]++;
						CrtDrawAll();
						system_guide();
					}
				} else if(key == KF_SysCursordown) {
					if(split_size[splitDataHorizon][splitLower] > SPLIT_HORIZON_MIN) {
						split_move[splitDataHorizon]++;
						split_start[splitDataHorizon]++;
						split_size[splitDataHorizon][splitUpper]++;
						split_size[splitDataHorizon][splitLower]--;
						CrtDrawAll();
						system_guide();
					}
				}
			} else if(split_mode == splitVertical) {
				if(key == KF_SysCursorleft) {
					if(split_size[splitDataVertical][splitLeft] > SPLIT_VERTICAL_MIN) {
						split_move[splitDataVertical]--;
						split_start[splitDataVertical]--;
						split_size[splitDataVertical][splitLeft]--;
						split_size[splitDataVertical][splitRight]++;
						system_drp[systemVertical]->x = split_start[splitDataVertical] - 1;
						CrtDrawAll();
						system_guide();
					}
				} else if(key == KF_SysCursorright) {
					if(split_size[splitDataVertical][splitRight] > SPLIT_VERTICAL_MIN) {
						split_move[splitDataVertical]++;
						split_start[splitDataVertical]++;
						split_size[splitDataVertical][splitLeft]++;
						split_size[splitDataVertical][splitRight]--;
						system_drp[systemVertical]->x = split_start[splitDataVertical] - 1;
						CrtDrawAll();
						system_guide();
					}
				}
			}
			if(key == KF_SysReturn) {
				break;
			}
			if(key == KF_SysEscape) {
				if(split_mode == splitHorizon) {
					split_start[splitDataHorizon] = start;
					split_move[splitDataHorizon] = move;
					split_size[splitDataHorizon][splitUpper] = size[splitUpper];
					split_size[splitDataHorizon][splitLower] = size[splitLower];
					CrtDrawAll();
					system_guide();
				} else {
					split_start[splitDataVertical] = start;
					split_move[splitDataVertical] = move;
					split_size[splitDataVertical][splitLeft] = size[splitLeft];
					split_size[splitDataVertical][splitRight] = size[splitRight];
					system_drp[systemVertical]->x = split_start[splitDataVertical] - 1;
					CrtDrawAll();
					system_guide();
				}
				break;
			}
		}
		csr_setdy(GetRow());
		system_msg("");
	}
}

SHELL void op_file_split()
{
	if(BackFileNo >= 0 && BackFileNo < MAX_edbuf) {
		split_mode++;
		if(split_mode > splitVertical) {
			split_mode = splitNone;
			split_file_no[splitLeft] = -1;
			split_file_no[splitRight] = -1;
		} else {
			int keep = CurrentFileNo;
			split_file_no[splitUpper] = CurrentFileNo;
			split_file_no[splitLower] = BackFileNo;
			if(split_mode == splitHorizon) {
				split_start[splitDataHorizon] = term_sizey() / 2;
				split_size[splitDataHorizon][splitUpper] = split_start[splitDataHorizon] - 1;
				split_size[splitDataHorizon][splitLower] = split_start[splitDataHorizon] - 1;
				split_start[splitDataHorizon] += split_move[splitDataHorizon];
				split_size[splitDataHorizon][splitUpper] += split_move[splitDataHorizon];
				split_size[splitDataHorizon][splitLower] -= split_move[splitDataHorizon];
			} else {
				split_start[splitDataVertical] = term_sizex() / 2;
				split_size[splitDataVertical][splitLeft] = split_start[splitDataVertical] - 1;
				split_size[splitDataVertical][splitRight] = split_start[splitDataVertical] - 1;
				split_start[splitDataVertical] += split_move[splitDataVertical];
				split_size[splitDataVertical][splitLeft] += split_move[splitDataVertical];
				split_size[splitDataVertical][splitRight] -= split_move[splitDataVertical];
				system_drp[systemVertical]->x = split_start[splitDataVertical] - 1;
			}
			CurrentFileNo = BackFileNo;
			csr_setdy(GetRow());
			CurrentFileNo = keep;
		}
		csr_lenew();
		csr_setdy(GetRow());
		system_msg("");
	}
}

