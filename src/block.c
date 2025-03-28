/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:block.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 

	n8
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#include "n8.h"
#include "block.h"
#include "list.h"
#include "line.h"
#include "cursor.h"
#include "keyf.h"
#include "file.h"
#include "sh.h"

#define	blck edbuf[CurrentFileNo].block

void block_set(block_t *bp)
{
	block_t *bbp = &blck;

	bp->blkm = bbp->blkm;
	if(bp->blkm == BLKM_none) {
		return;
	}

	if(bbp->y_st > bbp->y_ed || (bbp->y_st == bbp->y_ed && bbp->x_st > bbp->x_ed)) {
		bp->x_st = bbp->x_ed;
		bp->x_ed = bbp->x_st;
		bp->y_st = bbp->y_ed;
		bp->y_ed = bbp->y_st;
	} else {
		bp->x_st = bbp->x_st;
		bp->x_ed = bbp->x_ed;
		bp->y_st = bbp->y_st;
		bp->y_ed = bbp->y_ed;
	}
}

bool block_range(long n, block_t *bp, int *x_st, int *x_ed)
{
	if(bp->blkm == BLKM_none || bp->y_st > n || bp->y_ed < n) {
		return FALSE;
	}

	if(bp->blkm == BLKM_y) {
		if(bp->y_ed == n) {
			return FALSE;
		}
	} else {
		if(bp->y_st == bp->y_ed) {
			*x_st = bp->x_st;
			*x_ed = bp->x_ed;
			return TRUE;
		}

		if(bp->y_st == n) {
			*x_st = bp->x_st;
			*x_ed = strlen(GetList(n)->buffer) + 1;
			return TRUE;
		}
		if(bp->y_ed == n) {
			*x_st = 0;
			*x_ed = bp->x_ed;
			return TRUE;
		}
	}
	*x_st = 0;
	*x_ed = strlen(GetList(n)->buffer) + 1;
	return TRUE;
}

int GetBlockFlg()
{
	return blck.blkm != BLKM_none;
}

void block_cmove()
{
	if(!GetBlockFlg()) {
		return;
	}
	if(blck.y_ed != GetLineOffset()) {
		blck.blkm = BLKM_y; 
	} else {
		if(blck.x_ed != GetBufferOffset()) {
			blck.blkm = BLKM_x;
		}
	}
	blck.x_ed = GetBufferOffset();
	blck.y_ed = GetLineOffset();
}

#define MAX_bstack 1024

typedef struct
{
	char *s;
	blkm_t blkm;
} bstack_t;

bstack_t bstack[MAX_bstack];
int bstack_nums;
block_t bkm;

void bstack_init()
{
	bstack_nums = 0;
}

void bstack_fin()
{
	int i;
	for(i = bstack_nums ; i > 0 ; ) {
		free(bstack[--i].s);
	}
	bstack_nums = 0;
}

int block_size(block_t *bp)
{
	int ln;
	int x_st, x_ed;
	long n;

	ln = 0;
	n = min(bp->y_st, bp->y_ed);
	while(block_range(n, &bkm, &x_st, &x_ed)) {
		ln += x_ed - x_st, ++n;
	}
	return ln;
}

bool bstack_copy()
{
	EditLine *ed;
	char *p;
	long n;
	int x_st, x_ed;
	char buf[MAXEDITLINE + 1];

	if(bstack_nums >= MAX_bstack) {
		if(!keysel_ynq(DELETE_BLOCK_BUFFER_MSG)) {
			return FALSE;
		}
		bstack_fin();
	}

	bstack[bstack_nums].s = p = (char *)mem_alloc(block_size(&bkm) + 1);
	bstack[bstack_nums].blkm = bkm.blkm;
	++bstack_nums;

	*p = '\0';
	n = bkm.y_st;
	ed = GetList(n);
	while(block_range(n, &bkm, &x_st, &x_ed)) {
		if(ed->next == NULL) {
			strcpy(buf, ed->buffer);
		} else {
			sprintf(buf, "%s\n", ed->buffer);
		}
		buf[x_ed] = '\0';
		strcat(p, buf + x_st);
		ed = ed->next;
		++n;
	}
	return TRUE;
}

void block_cut()
{
	EditLine *ed,*ed_next;
	int x_st, x_ed;
	char buf[MAXEDITLINE + 1];

	if(bkm.blkm == BLKM_y && bkm.y_st < GetLastNumber()) {
		lists_delete(bkm.y_st, bkm.y_ed-1);
	} else {
		ed = GetList(bkm.y_st);
		block_range(bkm.y_st, &bkm, &x_st, &x_ed);
		strcpy(buf, ed->buffer);
		buf[x_st] = '\0';

		ed_next = GetList(bkm.y_ed);
		block_range(bkm.y_ed, &bkm, &x_st, &x_ed);
		strcat(buf, ed_next->buffer + x_ed);

		Realloc(ed, buf);

		if(bkm.y_st + 1 <= bkm.y_ed) {
			lists_delete(bkm.y_st + 1, bkm.y_ed);
		}
	}

	SetFileChangeFlag();
	csr_lenew();

	csr_setdy(GetRow() + bkm.y_st - GetLineOffset());
	csr_setly(bkm.y_st);

	if(bkm.blkm == BLKM_x) {
		csr_setlx(bkm.x_st);
	}
}

void BlockInit()
{
	blck.blkm = BLKM_none;
}

SHELL void op_block_start()
{
	csr_leupdate();

	if(GetBlockFlg()) {
		BlockInit();
	} else {
		blck.blkm = BLKM_x;
		blck.y_st = GetLineOffset();
		blck.y_ed = GetLineOffset();
		blck.x_st = csrle.lx;
		blck.x_ed = csrle.lx;
	}
}

bool BlockCommand()		/* ブロックコマンドの準備用 */
{
	csr_leupdate();

	block_set(&bkm);
	if(GetBlockFlg()) {
		blck.blkm = BLKM_none;
	} else {
		bkm.y_st = GetLineOffset();
		bkm.y_ed = bkm.y_st + 1;

		if(bkm.y_st != GetLastNumber()) {
			bkm.blkm = BLKM_y; 
		} else {
			 bkm.blkm = BLKM_x;
			 --bkm.y_ed;
			 bkm.x_st = 0;
			 bkm.x_ed = strlen(GetList(GetLineOffset())->buffer);
		}
	}
	return block_size(&bkm) > 0;
}

SHELL void op_block_yanc()	/* ブロックをコピーするのみ。*/
{
	if(!BlockCommand()) {
		return;
	}
	bstack_copy();
}

SHELL void op_block_cut()
{
	if(!BlockCommand()) {
		return;
	}
	if(!bstack_copy()) {
		return;
	}
	block_cut();
}

const char *str_paste(char *s, const char *p)
{
	while(*p != '\n' && *p != '\0') {
		*s++ = *p++;
	}
	*s = '\0';
	return *p == '\n' ? p + 1 : NULL;
}

void bstack_paste()
{
	char buf[MAXEDITLINE + 1], buf_a[MAXEDITLINE + 1], *p;
	const char *q;
	EditLine *ed, *edn;
	int x, y;

	if(bstack_nums == 0) {
		return;
	}

	x = y = 0;
	q = bstack[bstack_nums - 1].s;
	csr_leupdate();

	if(bstack[bstack_nums - 1].blkm == BLKM_y) {
		edn = ed = GetList(GetLineOffset() - 1);
		for(;;) {
			q = str_paste(buf, q);
			if(q == NULL) {
				break;
			}
			y++;
			InsertLine(ed, MakeLine(buf));
			ed = ed->next;
		}
		csrse.ed = edn->next;
	} else {
		ed = GetList(GetLineOffset());
		strcpy(buf, ed->buffer);
		buf[GetBufferOffset()] = '\0';
		strcpy(buf_a, ed->buffer + GetBufferOffset());

		x = get_delete_count(q);
		q = str_paste(buf + GetBufferOffset(), q);
		if(q == NULL) {
			strcat(buf, buf_a);
			Realloc(ed, buf);
		} else {
			Realloc(ed, buf);

			for(;;) {
				 q = str_paste(buf, q);
				 if(q == NULL) {
				 	break;
				}
				InsertLine(ed, MakeLine(buf));
				ed = ed->next;
			}
			strcat(buf, buf_a);
			InsertLine(ed, MakeLine(buf));
		}
	}
	SetFileChangeFlag();

	csr_lenew();
	OffsetSetByColumn();
	if(sysinfo.pastemovef) {
		while(x > 0) {
			op_cursor_right();
			x--;
		}
		while(y > 0) {
			op_cursor_down();
			y--;
		}
	}
}

SHELL void op_block_paste()
{
	bstack_paste();
	if(bstack_nums > 0) {
		--bstack_nums;
	}
}

SHELL void op_block_copy()
{
	bstack_paste();
}

SHELL void op_block_dup()
{
	if(!BlockCommand()) {
		return;
	}

	bstack_copy();
	op_block_paste();
}

SHELL void op_block_kill()
{
	if(keysel_ynq(DELETE_YANK_BUFFER_MSG)) {
		bstack_fin();
	}
}

SHELL void op_block_chlast()
{
	long a;
	int b;
	blkm_t blkm;

	if(GetBlockFlg()) {
		a = blck.y_st;
		blck.y_st = GetLineOffset();

		b = blck.x_st;
		blck.x_st = GetBufferOffset();

		blkm = blck.blkm;

		csr_setly(a);
		csr_setlx(b);

		blck.blkm = blkm;
	}
}

