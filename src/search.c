/*--------------------------------------------------------------------
  string search and replace module.

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 

	n8
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#include "n8.h"
#include "search.h"
#include "line.h"
#include "list.h"
#include "crt.h"
#include "cursor.h"
#include "input.h"
#include "menu.h"
#include "keyf.h"
#include "../lib/regexp.h"
#include <ctype.h>

void cursor_move(int lx, long ly)	// !!
{
	RefreshMessage();

	csr_setly(ly);
	csr_setlx(lx);
}


// multi search/replace用バッファ
char s_search[MAXEDITLINE + 1];	/* search strings */
char s_repl[MAXEDITLINE + 1];	/* replace strings */
char s_get[MAXEDITLINE + 1];	/* 文字列取得バッファ */

void search_init()
{
	*s_search = '\0';
	*s_repl = '\0';

	*s_get = '\0';
}

SHELL void op_search_getword()
{
	int lx;

	if(*s_get == '\0') {
		lx = csrle.lx;
	} else {
		lx = csrle.lx+strlen(s_get);
	}

	lx = kanji_tknext(csrle.buf, lx, FALSE) - csrle.lx;
	if(lx == 0) {
		return;
	}

	strncpy(s_get, csrle.buf + csrle.lx, lx);
	s_get[lx] = '\0';

	system_msg(s_get);

	edbuf[CurrentFileNo].pm = TRUE;
	strcpy(s_search, s_get);
}

void search_clrword()
{
	*s_get = '\0';
}

long search_next(const char *s, int x, long y, regm_t *rmp)
{
	EditLine *ed;
	long n;

	n = GetLastNumber();
	ed = GetList(y);
	if(ed != NULL && ed->buffer != NULL && x >= strlen(ed->buffer)) {
		x = 0;
		++y;
		ed = ed->next;
	}

	for( ; y <= n ; ++y) {
		if(ed == NULL || ed->buffer == NULL) {
			break;
		}
		if(regexp_seeknext(ed->buffer, s, x, rmp, sysinfo.nocasef)) {
			return y;
		}
		ed = ed->next;
		x = 0;
	}
	return -1;
}

long search_prev(const char *s, int x, long y, regm_t *rmp)
{
	EditLine *ed;

	if(x < 0) {
		--y;
	}

	ed = GetList(y);
	if(ed == NULL || ed->buffer == NULL) {
		return -1;
	}
	if(x < 0) {
		x = strlen(ed->buffer);
	}

	for( ; y > 0 ; --y) {
		if(regexp_seekprev(ed->buffer, s, x, rmp, sysinfo.nocasef)) {
			return y;
		}
		ed = ed->prev;
		if(ed == NULL || ed->buffer == NULL) { // buffer
			break;
		}
		x = strlen(ed->buffer); // buffer
	}
	return -1;
}

SHELL void op_search_in()
{
	char buf[MAXEDITLINE + 1];

	*buf = '\0';
	if(HisGets(buf, GETS_FIND_MSG, SEARCHS_SYSTEM) == NULL || *buf == '\0') {
		edbuf[CurrentFileNo].pm = FALSE;
		*s_search = '\0';
	} else {
		lm_mark(GetLineOffset(), 0);
		edbuf[CurrentFileNo].pm = TRUE;
		strcpy(s_search, buf);
	}
}

SHELL void op_search_prev()
{
	regm_t rm;
	long y;

	if(*s_search == '\0') {
		op_search_in();
		if(*s_search == '\0') {
			return;
		}
	}

	csr_leupdate();
	system_msg(SEARCHING_MSG);

	y = search_prev(s_search, GetBufferOffset() - 1, GetLineOffset(), &rm);

	if(y == -1) {
		system_msg(NOT_FOUND_MSG);
	} else {
		cursor_move(rm.rm_so, y);
	}
}

SHELL void op_search_next()
{
	regm_t rm;
	long y;

	if(*s_search == '\0') {
		op_search_in();
		if(*s_search == '\0') {
			return;
		}
	}

	csr_leupdate();
	system_msg(SEARCHING_MSG);
	y = search_next(s_search, GetBufferOffset() + 1, GetLineOffset(), &rm);

	if(y == -1) {
		system_msg(NOT_FOUND_MSG);
	} else {
		cursor_move(rm.rm_so, y);
	}
}

void replace()
{
	regm_t rm;
	long y, my;
	int x, mx;
	int chgNo;
	char buf[MAXLINESTR + 1];
	bool rf;

	int cx, sx, lx, cy;
	long ly;
	EditLine *ed;

	csr_leupdate();

	cx = csrle.cx;
	sx = csrle.sx;
	lx = csrle.lx;
	cy = csrse.cy;
	ly = csrse.ly;
	ed = csrse.ed;

	chgNo = 0;
	mx = 0;
	my = 0;
	switch(edbuf[CurrentFileNo].replm) {
	case REPLM_before:
		mx = lx;
		my = ly;
	case REPLM_all:
	default:
		x = 0;
		y = 1;
		break;
	case REPLM_after:
		x = lx;
		y = ly;
		break;
/*	 case REPLM_block:
		x = 1;
		y = GetBlockStart();
		my = GetBlockEnd();*/
	}

	rf = FALSE;
	for(;;) {
		system_msg(SEARCHING_MSG);
		y = search_next(s_search, x, y, &rm);

		if(y == -1) {
			break;
		}

		x = rm.rm_so;
		if(my != 0 && (y > my || (y == my && mx != 0 && x > mx))) {
			break;
		}

		cursor_move(x, y);

		if(!rf) {
			char c;
			int n;
			char buf[LN_dspbuf + 1];

			CrtDrawAll();
			term_locate(GetRow(), GetCol() + NumWidth);

			term_color(sysinfo.c_search);
			memcpy(buf, GetList(y)->buffer + rm.rm_so, rm.rm_eo - rm.rm_so);
			buf[rm.rm_eo - rm.rm_so] = '\0';
			term_puts(buf);

			c = keysel(REPLACE_YNA_MSG, "Yy\r\nNnAa \x1b");
			if(c == '\x1b') {
				break;
			}
			if(c == 'n' || c == ' ') {
				++x;
				continue;
			}
			if(c == 'a') {
				rf = TRUE;
			}
		}

		csr_lenew();

		se_delete(rm.rm_eo - x, TRUE);
		se_insert(s_repl, FALSE);
		x += strlen(s_repl);

		csr_leupdate();

		++chgNo;
	}
	// 変換を開始した時のカーソル位置に戻す。
	csrle.cx = cx;
	csrle.sx = sx;
	csrle.lx = lx;
	csrse.cy = cy;
	csrse.ly = ly;
	csrse.ed = ed;
	csr_fix();
	csr_lenew();

	sprintf(buf, "[ %.30s -> %-.30s ] %d %s", s_search, s_repl, chgNo, REPLACED_MSG);
	system_msg(buf);
}

SHELL void op_search_paging()
{
	edbuf[CurrentFileNo].pm = edbuf[CurrentFileNo].pm ? FALSE : TRUE;
}

SHELL void op_search_repl_redo()
{
	if(*s_search == '\0' || *s_repl == '\0') {
		return;
	}
	edbuf[CurrentFileNo].replm = REPLM_all;
	replace();
}

SHELL void op_search_repl()
{
	int res;
	int nums;

	csr_leupdate();

	nums = 3;
	res = menu_vselect(GetCol(), GetRow(), nums, REPLACE_ALL_MSG, REPLACE_FORWARD_MSG, REPLACE_BACK_MSG, REPLACE_INSIDEBLOCK_MSG);

	switch(res) {
	case -1:
		return;
	case 0:
		edbuf[CurrentFileNo].replm = REPLM_all;
		break;
	case 1:
		edbuf[CurrentFileNo].replm = REPLM_after;
		break;
	case 2:
		edbuf[CurrentFileNo].replm = REPLM_before;
		break;
	case 3:
		edbuf[CurrentFileNo].replm = REPLM_block;
		break;
	}

	CrtDrawAll();
	*s_search = '\0';
	if(HisGets(s_search, GETS_FIND_MSG, SEARCHS_SYSTEM) == NULL) {
		return;
	}
	*s_repl = '\0';
	if(HisGets(s_repl, GETS_REPLACE_MSG, SEARCHS_SYSTEM) == NULL) {
		return;
	}
	replace();
}

