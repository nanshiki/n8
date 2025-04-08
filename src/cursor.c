/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:cursor.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 

	n8
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#include "n8.h"
#include "keyf.h"
#include "cursor.h"
#include "list.h"
#include "crt.h"
#include "block.h"
#include "search.h"
#include "file.h"
#include "line.h"
#include "lineedit.h"
#include "sh.h"
#include <ctype.h>

int GetCol()
{
	return csrle.cx;
}

int GetScroll()
{
	return csrle.sx;
}

int GetBufferOffset()
{
	return csrle.lx;
}

int GetRow()
{
	return csrse.cy;
}

long GetLineOffset()
{
	return csrse.ly;
}

void csr_fix()
{
	csrse.f_cx = csrle.cx;
	csrse.f_sx = csrle.sx;
}

void csr_lenew()
{
	strcpy(csrle.buf, GetList(GetLineOffset())->buffer);
	csrle.dsize = GetColWidth()- NumWidth;	// !!
}

void csr_leupdate()
{
	EditLine *ed;

	ed = GetList(GetLineOffset());
	if(strcmp(csrle.buf, ed->buffer) != 0) {
		Realloc(ed, csrle.buf);
		SetFileChangeFlag();
	}
}

void csr_movehook()
{
	block_cmove();
	search_clrword();
}

void OffsetSetByColumn()
{
	int i;
	int csx, f_csx;

	f_csx = csrse.f_cx + csrse.f_sx;
	csx = 0;
	for(i = 0 ; csx < f_csx; ) {
		if(csrle.buf[i] == '\0') {
			break;
		}
		csx += kanji_countdsp(&csrle.buf[i], csx);
		i = kanji_posnext(i, csrle.buf);
	}
	if(sysinfo.freecursorf) {
		i += f_csx - csx;
	}
	if(csx > f_csx) {
		i = kanji_posprev(i, csrle.buf);
	}
	csrle.sx = csrse.f_sx;
	le_setlx(&csrle, i);
}

void csr_setlx(int lx)
{
	le_setlx(&csrle, lx);
	csr_fix();
	csr_movehook();
}

void LineOffsetSet(long n)
{
	csr_leupdate();

	n = max(n, GetTopNumber());
	n = min(n, GetLastNumber());

	csrse.ed = GetList(n);
	csrse.ly = n;

	csr_lenew();
	OffsetSetByColumn();

	csr_movehook();
}

void CursorSetRow(int n)
{
	n = max(n, GetMinRow());
	n = min(n, GetMaxRow() - 1);
	csrse.cy = n;
}

long csr_getsy()
{
	return csrse.ly - csrse.cy;
}

void csr_setly(long n)
{
	n = max(n, GetTopNumber());
	n = min(n, GetLastNumber());

	if(n == GetLineOffset()) {
		return;
	}
	if(n < GetRow()) {
		CursorSetRow(n);
	}
	LineOffsetSet(n);
}

void csr_setdy(int dy)
{
	dy = min(dy, GetLastNumber());

	LineOffsetSet(GetLineOffset() + dy - GetRow());

	dy = max(dy, GetMinRow());
	dy = min(dy, GetMaxRow() - 1);
	csrse.cy = dy;
}

void CursorMove()
{
#if 0
	if(csrle.lx == strlen(csrle.buf) && csrse.ly < GetLastNumber() && sysinfo.crmarkf) {
		term_locate(GetRow(), GetCol() + NumWidth);
		term_color_normal();
		term_putch(' ');		/* 行末マークを消す。 */
		csrse.gf = TRUE;
	}
#endif
	term_locate(GetRow(), GetCol() + NumWidth);
}

SHELL void op_cursor_up()
{
	csr_setdy(GetRow() - 1);
}

SHELL void op_cursor_down()
{
	if(GetLineOffset() < GetLastNumber()) {
		csr_setdy(GetRow() + 1);
	}
}

SHELL void op_cursor_sleft()
{
	csr_setlx(0);
}

SHELL void op_cursor_sright()
{
	csr_setlx(strlen(csrle.buf));
}


SHELL void op_cursor_tkprev()
{
	if(GetBufferOffset() > 0) {
		csr_setlx(kanji_tkprev(csrle.buf, csrle.lx, TRUE));
	} else {
		if(GetLineOffset() > 1) {
			op_cursor_up();
			op_cursor_sright();
		}
	}
}

SHELL void op_cursor_tknext()
{
	if(GetBufferOffset() < strlen(csrle.buf)) {
		csr_setlx(kanji_tknext(csrle.buf, csrle.lx, TRUE));
	} else {
		if(GetLineOffset() < GetLastNumber()) {
			op_cursor_down();
			op_cursor_sleft();
		}
	}
}

SHELL void op_cursor_left()
{
	if(GetBufferOffset() > 0) {
		csr_setlx(GetBufferOffset() - 1);
	} else {
		if(GetLineOffset() > 1) {
			op_cursor_up();
			op_cursor_sright();
		}
	}
}

SHELL void op_cursor_right()
{
	if(sysinfo.freecursorf || GetBufferOffset() < strlen(csrle.buf)) {
		int p = GetBufferOffset();
		csr_setlx(p + kanji_countbuf(&csrle.buf[p]));
	} else {
		 if(GetLineOffset() < GetLastNumber()) {
		 	op_cursor_down();
		 	op_cursor_sleft();
		 }
	}
}

SHELL void op_cursor_sup()
{
	csr_setdy(GetMinRow());
}

SHELL void op_cursor_sdown()
{
	csr_setdy(min(GetMaxRow() - 1, GetRow() + GetLastNumber() - GetLineOffset()));
}

SHELL void op_cursor_top()
{
	lm_mark(GetLineOffset(), 0);
	csr_setly(GetTopNumber());
}

SHELL void op_cursor_bottom()
{
	lm_mark(GetLineOffset(), 0);
	csr_setly(GetLastNumber());
	csr_setdy(GetMaxRow() - 1);
}

SHELL void op_cursor_pup()
{
	if(edbuf[CurrentFileNo].pm) {
		op_search_prev();
	} else {
		csr_setly(GetLineOffset() - (GetRowWidth() - 1));
	}
}

SHELL void op_cursor_pdown()
{
	if(edbuf[CurrentFileNo].pm) {
		op_search_next();
	} else {
		csr_setly(GetLineOffset() + (GetRowWidth() - 1));
	}
}

SHELL void op_cursor_rup()
{
	int n;

	n = 0;
	if(keyf_numarg() > 0) {
		n = atoi(keyf_getarg(0));
	}
	csr_setly(GetLineOffset() - (n <= 0 ? 1 : n));
}

SHELL void op_cursor_rdown()
{
	int n;

	n = 0;
	if(keyf_numarg() > 0) {
		n = atoi(keyf_getarg(0));
	}
	csr_setly(GetLineOffset() + (n <= 0 ? 1 : n));
}


