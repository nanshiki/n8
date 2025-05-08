/*--------------------------------------------------------------------
  menu module.

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 

	n8
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#include	"n8.h"
#include	"menu.h"
#include	"crt.h"
#include	"keyf.h"
#include	"sh.h"
#include	<ctype.h>
#include	<stdarg.h>

dspfmt_t *dspreg_menu(void *vp, int y, int sizex, int sizey);

void menu_iteminit(menu_t *mnp)
{
	mnp->nums = 0;

	mnp->sy = 0;
	mnp->cy = 0;

	mnp->title = "";
	mnp->drp = dsp_reginit();
	mnp->drp->func = dspreg_menu;
	mnp->drp->vp = (void *)mnp;

	mnp->drp->y += 1;
	mnp->drp->sizey -= 1;

	mnp->df = FALSE;
}

void menu_itemfin(menu_t *mnp)
{
	if(mnp->nums > 0) {
		int i;

		term_color(AC_normal);
		for(i = mnp->drp->y ; i < mnp->drp->y + mnp->drp->sizey ; ++i) {
			term_locate(i, mnp->drp->x);
			widthputs("", mnp->drp->sizex);
		}
		free(mnp->mitem);
		mnp->nums = 0;
	}
}

void menu_itemmake(menu_t *mnp, void func(int, mitem_t *, void *), size_t nums, void *vp)
{
	int i;
	int ln;

	menu_itemfin(mnp);
	mnp->mitem = mem_alloc(sizeof(mitem_t) * nums);
	ln = 0;
	for(i = 0 ; i < nums ; ++i) {
		mnp->mitem[i].cc = sysinfo.c_menuc;
		mnp->mitem[i].nc = sysinfo.c_menun;
		mnp->mitem[i].mf = FALSE;
		func(i, mnp->mitem + i, vp);
		ln = max(ln, get_display_length(mnp->mitem[i].str));
		++mnp->nums;
	}
	ln += 4;
	if(check_frame_ambiguous2() && sysinfo.framechar != frameCharTeraTerm) {
		ln += 2;
	}
	dsp_regresize(mnp->drp, ln, min(mnp->nums + 2, dspall.sizey - 2));
	dsp_regadd(mnp->drp);
}

typedef struct
{
	size_t width;
	size_t num;
	char *s;
} itemstr_t;

void makelists_proc(int a, mitem_t *mip, void *vp)
{
	itemstr_t *isp;
	isp = vp;
	strcpy(mip->str, isp->s + isp->width * a);
}

void menu_itemmakelists(menu_t *mnp, size_t width, size_t num, char *s)
{
	itemstr_t is;

	is.num = num;
	is.width = width;
	is.s = s;

	menu_itemmake(mnp, makelists_proc, num, &is);
}

int menu_vselect(char *title, int x, int y, size_t num, ...)
{
	menu_t menu;
	va_list args;
	int i;
	int ln;
	char *p;

	menu_iteminit(&menu);
	if(title != NULL) {
		menu.title = title;
	}
	if(x != -1) {
		menu.drp->x = x;
	}
	if(check_frame_ambiguous2() && (menu.drp->x & 1)) {
		menu.drp->x--;
	}
	if(y != -1) {
		menu.drp->y = y;
	}
	if(menu.drp->y > dspall.sizey - num - 3) {
		menu.drp->y = dspall.sizey - num - 3;
	}
	va_start(args, num);
	menu_itemfin(&menu);
	menu.mitem = mem_alloc(sizeof(mitem_t) * num);
	ln = 0;
	for(i = 0 ; i <num ; ++i) {
		menu.mitem[i].cc = sysinfo.c_menuc;
		menu.mitem[i].nc = sysinfo.c_menun;
		menu.mitem[i].mf = FALSE;

		p = va_arg(args, char *);
		if(p == NULL) {
			strcpy(menu.mitem[i].str, "null");
		} else {
			strcpy(menu.mitem[i].str, p);
		}
		ln = max(ln, get_display_length(menu.mitem[i].str));
		++menu.nums;
	}
	ln += 4;
	if(check_frame_ambiguous2() && sysinfo.framechar != frameCharTeraTerm) {
		ln++;
		if(ln & 1) {
			ln++;
		}
	}
	dsp_regresize(menu.drp, ln, min(menu.nums + 2, dspall.sizey - 2));
	dsp_regadd(menu.drp);
	va_end(args);

	return menu_select(&menu);
}

void menu_dview(menu_t *mnp)
{
	int i;

	for(i = 0 ; i < mnp->nums ; ++i) {
		fprintf(stderr, "%2d:%s\n", i, mnp->mitem[i].str);
	}
}

void menu_csrmove(menu_t *mnp, int ly)
{
	int ly_b;

	ly_b = mnp->cy + mnp->sy;
	ly = max(ly, 0);
	ly = min(ly, mnp->nums - 1);
	if(ly_b == ly) {
		return;
	}
	if(ly_b - ly > mnp->cy) {
		ly_b -= mnp->cy;
		mnp->cy = 0;
		mnp->sy -= ly_b - ly;
		return;
	}
	if(ly - ly_b > mnp->drp->sizey - 2 - mnp->cy -1) {
		ly -= mnp->drp->sizey - 2 - mnp->cy - 1;
		mnp->cy = mnp->drp->sizey - 2 - 1;
		mnp->sy += ly - ly_b;
		return;
	}
	mnp->cy += ly - ly_b;
}

int menu_csrnext(menu_t *mnp, char c)
{
	int n, m;

	m = n = mnp->cy + mnp->sy;
	for(;;) {
		++n;
		if(n >= mnp->nums) {
			n = 0;
		}
		if(toupper(*mnp->mitem[n].str) == toupper(c)) {
			menu_csrmove(mnp, n);
			return TRUE;
		}
		if(n == m) {
			return FALSE;
		}
	}
	return FALSE;
}

char left_top_char[] = { (char)0xe2, 0x95 ,(char)0x94, 0x00 };
char horizon_line_char[] = { (char)0xe2, 0x95 ,(char)0x90, 0x00 };
char right_top_char[] = { (char)0xe2, 0x95 ,(char)0x97, 0x00 };
char vertical_line_char[] = { (char)0xe2, 0x95 ,(char)0x91, 0x00 };
char left_bottom_char[] = { (char)0xe2, 0x95 ,(char)0x9a, 0x00 };
char right_bottom_char[] = { (char)0xe2, 0x95 ,(char)0x9d, 0x00 };

void make_frame_top(char *buf, char *msg, int size)
{
	int a, w, pos;
	int flag = FALSE;

	a = min(get_display_length(msg), size - 3 - 3);
	if(sysinfo.framechar >= frameCharFrame) {
		strcpy(buf, left_top_char);
		strcpy(&buf[3], horizon_line_char);
		pos = 6;
	} else if(sysinfo.framechar == frameCharASCII) {
		strcpy(buf, "+-");
		pos = 2;
	} else {
		strcpy(buf, "  ");
		pos = 2;
	}
	if(*msg != '\0') {
		buf[pos++] = ' ';
		w = strjfcpy(buf + pos, msg, LN_dspbuf - pos, a, TRUE);
		w += 5;
		if(check_frame_ambiguous2() && sysinfo.framechar != frameCharTeraTerm) {
			w += 3;
			if(w & 1) {
				flag = TRUE;
				w++;
			}
		}
	} else {
		w = 3;
		if(check_frame_ambiguous2() && sysinfo.framechar != frameCharTeraTerm) {
			w += 4;
		}
	}
	a = strlen(buf);
	if(*msg != '\0') {
		buf[a++] = ' ';
		if(flag) {
			buf[a++] = ' ';
		}
	}
	while(w < size) {
		if(sysinfo.framechar >= frameCharFrame) {
			strcpy(&buf[a], horizon_line_char);
			a += 3;
			if(sysinfo.ambiguous == AM_FIX1 ||  sysinfo.framechar == frameCharTeraTerm) {
				w++;
			} else {
				w += 2;
			}
		} else {
			buf[a++] = (sysinfo.framechar == frameCharASCII) ? '-' : ' ';
			w++;
		}
	}
	if(sysinfo.framechar >= frameCharFrame) {
		strcpy(&buf[a], right_top_char);
		a += 3;
	} else {
		buf[a++] = (sysinfo.framechar == frameCharASCII) ? '+' : ' ';
	}
	buf[a] = '\0';
}

void make_frame_bottom(char *buf, int size)
{
	if(sysinfo.framechar >= frameCharFrame) {
		int a = 3;
		int c = (sysinfo.ambiguous == AM_FIX1 || sysinfo.framechar == frameCharTeraTerm) ? 2 : 4;
		strcpy(buf, left_bottom_char);
		while(size > c) {
			strcpy(&buf[a], horizon_line_char);
			a += 3;
			size--;
			if(sysinfo.ambiguous != AM_FIX1 && sysinfo.framechar != frameCharTeraTerm) {
				size--;
			}
		}
		strcpy(&buf[a], right_bottom_char);
	} else {
		*buf = (sysinfo.framechar == frameCharASCII) ? '+' : ' ';
		memset(buf + 1, (sysinfo.framechar == frameCharASCII) ? '-' : ' ', size - 2);
		buf[size - 1] = (sysinfo.framechar == frameCharASCII) ? '+' : ' ';
		buf[size] = '\0';
	}
}

dspfmt_t *dspreg_menu(void *vp, int y, int sizex, int sizey)
{
	int w;
	dspfmt_t *dfp, *dfpb;
	char buf[LN_dspbuf + 1];
	menu_t *mnp;

	mnp = (menu_t *)vp;
	w = sizex - 1;
	if(y == 0) {
		make_frame_top(buf, mnp->title, sizex);
	}
	if(y == sizey - 1) {
		make_frame_bottom(buf, sizex);
	}

	if(y == 0 || y == sizey - 1) {
		dfp = dsp_fmtinit(buf, NULL);
		dfp->col = sysinfo.c_frame;
		return dfp;
	}
	--y;
	if(sysinfo.framechar >= frameCharFrame) {
		strcpy(buf, vertical_line_char);
		dfp = dsp_fmtinit(buf, NULL);
	} else {
		dfp = dsp_fmtinit((sysinfo.framechar == frameCharASCII) ? "|" : " ", NULL);
	}
	dfpb = dfp;
	dfp->col = sysinfo.c_frame;

	dfp = dsp_fmtinit(mnp->mitem[mnp->sy + y].mf ? "*" : " ", dfp);
	if(mnp->cy == y && !mnp->df) {
		dfp->col = mnp->mitem[mnp->sy + y].cc;
	} else {
		dfp->col = mnp->mitem[mnp->sy + y].nc;
	}
	strjfcpy(buf, mnp->mitem[mnp->sy + y].str, LN_dspbuf, sizex - ((check_frame_ambiguous2() && sysinfo.framechar != frameCharTeraTerm) ? 5 : 3), TRUE);
	dfp = dsp_fmtinit(buf, dfp);
	if(mnp->cy == y && !mnp->df) {
		dfp->col = mnp->mitem[mnp->sy + y].cc;
	} else {
		dfp->col = mnp->mitem[mnp->sy + y].nc;
	}
	if(sysinfo.framechar >= frameCharFrame) {
		strcpy(buf, vertical_line_char);
		dfp = dsp_fmtinit(buf, dfp);
	} else {
		dfp = dsp_fmtinit((sysinfo.framechar == frameCharASCII) ? "|" : " ", dfp);
	}
	dfp->col = sysinfo.c_frame;
	return dfpb;
}

int menu_select(menu_t *mnp)
{
	int c;

	for(;;) {
		dsp_allview();
		term_csrh();
		c = get_keyf(1);
		switch(c) {
		case -1:
			continue;
		default:
			if(c & KF_normalcode) {
				if(menu_csrnext(mnp, c & 0x00ff)) {
					c = mnp->cy + mnp->sy;
					break;
				}
			}
			continue;
		case KF_SysCursorup:
			menu_csrmove(mnp, mnp->cy + mnp->sy - 1);
			continue;
		case KF_SysCursordown:
			menu_csrmove(mnp, mnp->cy + mnp->sy + 1);
			continue;
		case KF_SysScrolldown:
			menu_csrmove(mnp, mnp->cy + mnp->sy - mnp->drp->sizey - 2);
			continue;
		case KF_SysScrollup:
			menu_csrmove(mnp, mnp->cy + mnp->sy + mnp->drp->sizey - 2);
			continue;
		case KF_SysCursortopside:
			menu_csrmove(mnp, 0);
			continue;
		case KF_SysCursorendside:
			menu_csrmove(mnp, mnp->nums - 1);
			continue;
		case KF_SysReturn:
			c = mnp->cy + mnp->sy;
			break;
		case KF_SysEscape:
			c = -1;
			break;
		}
		break;
	}
	menu_itemfin(mnp);
	dsp_regfin(mnp->drp);
	return c;
}

