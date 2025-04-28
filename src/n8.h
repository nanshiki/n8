/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:ed.h
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000
        SASAKI Shunsuke <ele@pop17.odn.ne.jp> All rights reserved. 

	n8
		change file name n8.h
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#include "config.h"

#define	VER "4.05"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/param.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "../lib/generic.h"
#include "../lib/hash.h"
#include "kanji.h"
#include "iskanji.h"

#define SHELL

#ifdef VAL_impl
#define VAL
#else
#define VAL extern
#endif


#define LN_path		2048
#define LN_dspbuf	2048
#define LN_buf		2048

#define MAX_val		64


#include "../lib/term.h"
#include "disp.h"


#define FOPEN_SYSTEM	0
#define SEARCHS_SYSTEM	1
#define FRENAME_SYSTEM	2
#define SHELLS_SYSTEM	3
#define UNDO_SYSTEM		4
#define	HISTORY_MAX		5

#define MAX_edfiles		8
#define MAX_edbuf		MAX_edfiles


#define ESCAPE	(-1)

#define NONE		0
#define DELETE		1
#define BACKSPACE	2
#define STAY		3

#define MAXKEYDEF	200
#define MAXSYSKEYDEF	200
#define NumWidth	(sysinfo.numberf ? 6: 0)

//#define	MAXLINESTR	255
#define MAXLINESTR	2048
//#define	MAXEDITLINE	1024
#define MAXEDITLINE	2048
#define MAXFILEMENU	512

#define DEFAULT_FILE_HISTORY_COUNT	30
#define N8_HISTORY_FILE				"n8file"
#define N8_LOCK_FILE				"n8lock"

typedef struct _ed
{
	struct _ed *prev;
	struct _ed *next;
	char *buffer;

	size_t size;		/* 確保してあるバッファのサイズ */
	size_t bytes;		/* バッファ上の実際の文字サイズ*/
} EditLine;

typedef struct
{
	int x, y;			/* 表示の起点 */
	int sizex, sizey;	/* ディスプレイサイズ */

	int cy;				/* 画面上でのカーソル位置 */
	long ly;			/* 行番号 */
	EditLine *ed;		/* その行のバッファ */

	int f_cx, f_sx;		/* fix されるまで同じ位置を維持しようとする。 */

	int l_cy;			/* latest. */
	long l_sy;			/* これらが変化すれば再描画が必要 */

	size_t bytes;		/* 全体の編集サイズ */

	bool gf;			/* 行末マークを消したフラグ */
} se_t;

typedef struct
{
	char buf[MAXEDITLINE + 1];	/* 編集バッファ */
	int size;					/* 編集バッファのサイズ */

	int dsize;					/* 表示可能サイズ */
	int dx;						/* 表示位置 */

	int cx, sx;					/* 正しい、カーソル/スクロール位置 */
	int lx;						/* 正しい、バッファ上の位置 */

	int l_sx;					/* latest. */
} le_t;

#define csrse edbuf[CurrentFileNo].se
#define csrle edbuf[CurrentFileNo].le

//システム情報
typedef struct _sitem
{
	char *str;
	struct _sitem *next;
} sitem_t;

enum {
	itemHide,
	itemUse,
	itemMask,
	itemDir,

	itemMax
};

typedef struct
{
	char nxpath[LN_path];
	hash_t *vp_def;
	const char *shell;

	int tabstop;
	char tabcode;

	color_t c_crmark;
	color_t c_block;
	color_t c_linenum;
	color_t c_ctrl;
	color_t c_sysmsg;
	color_t c_search;
	color_t c_menuc;
	color_t c_menun;
	color_t c_eff_dirc;
	color_t c_eff_dirn;
	color_t c_eff_normc;
	color_t c_eff_normn;
	color_t c_tab;

	bool crmarkf;	/* crmark処理を行うかどうか */
	bool tabmarkf;
	bool autoindentf;
	bool numberf;
	bool overwritef;
	bool japanesef;

	bool backupf;
	bool nocasef;
	bool pastemovef;
	bool underlinef;
	bool nfdf;
	int file_history_count;
	int ambiguous;

	char systemline[MAXEDITLINE + 1];
	dspreg_t *sl_drp;
	char doublekey[8 + 1];

	sitem_t *sitem[itemMax];
} sysinfo_t;

VAL sysinfo_t sysinfo;
VAL int CurrentFileNo;
VAL int BackFileNo;

//csrse
VAL int OnMessage_Flag;

typedef enum {
	REPLM_all,
	REPLM_before,
	REPLM_after,
	REPLM_block
} replm_t;

typedef enum {
	BLKM_none,
	BLKM_x,
	BLKM_y
} blkm_t;

typedef struct
{
	long y_st, y_ed;
	int x_st, x_ed;

	blkm_t blkm;
} block_t;

typedef struct
{
	char path[LN_path + 1];
	int ct;					/* Create Time */
	bool cf;				/* file Change Flag */
	int kc;					/* EUC/JIS/SJIS/UTF8 */
	int open_kc;
	int rm;					/* LF/CRLF/CR */

	bool pm;				/* Paging Mode */
	replm_t replm;			/* replace Mode */

	se_t se;
	le_t le;

	block_t block;
} edbuf_t;

VAL edbuf_t edbuf[MAX_edbuf];

#define	CNTRL(c)	((c)-'@')

typedef struct _history_data
{
	struct _history_data *prev;
	struct _history_data *next;
	char *buffer;

	int line;
} HistoryData;

