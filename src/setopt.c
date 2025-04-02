/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:setopt.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 

	n8
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#include "n8.h"
#include "setopt.h"
#include "crt.h"
#include "file.h"
#include "menu.h"
#include "keyf.h"
#include "keys.h"
#include "../lib/term_inkey.h"
#include "sh.h"
#include <ctype.h>

enum {
	optionNumber,
	optionOverWrite,
	optionFreeCursor,
	optionAutoIndent,
	optionBackup,
	optionTabMark,
	optionCrMark,
	optionNoCase,
	optionJapanese,
	optionKanjiCode,
	optionRetMode,
	optionTabStop,
	optionPasteMove,

	optionMax
};

extern keydef_t keydef[MAX_region][MAXKEYDEF];

void key_set()
{
	int n;

	term_escdefault();

	n = keydef_num(0);
	if(n == 0) {
		keyf_set(0, "^KW",           "file_toggle");
		keyf_set(0, "[F02]",         "file_toggle");
		keyf_set(0, "^KT",           "file_select");
		keyf_set(0, "[F03]",         "file_select");

		keyf_set(0, "[ESC]O",        "file_open");
		keyf_set(0, "[ESC]C",        "file_close");
		keyf_set(0, "[ESC]L",        "file_copen");
		keyf_set(0, "[ESC]S",        "file_save");
		keyf_set(0, "[ESC]Q",        "file_quit");

		keyf_set(0, "[ESC]P",        "file_rename");

		keyf_set(0, "[ESC]U",        "file_undo");
		keyf_set(0, "[ESC]I",        "file_insert");
		keyf_set(0, "[ESC]X",        "file_aclose");


		keyf_set(0, "^E",            "cursor_up");
		keyf_set(0, "[UP]",          "cursor_up");
		keyf_set(0, "^X",            "cursor_down");
		keyf_set(0, "[DOWN]",        "cursor_down");
		keyf_set(0, "^S",            "cursor_left");
		keyf_set(0, "[LEFT]",        "cursor_left");
		keyf_set(0, "^D",            "cursor_right");
		keyf_set(0, "[RIGHT]",       "cursor_right");
		keyf_set(0, "^A",            "cursor_tkprev");
		keyf_set(0, "^F",            "cursor_tknext");
		keyf_set(0, "^QS",           "cursor_sleft");
		keyf_set(0, "[HOME]",        "cursor_sleft");
		keyf_set(0, "^QD",           "cursor_sright");
		keyf_set(0, "[END]",         "cursor_sright");
		keyf_set(0, "^W",            "cursor_rup");
		keyf_set(0, "^Z",            "cursor_rdown");
		keyf_set(0, "^R",            "cursor_pup");
		keyf_set(0, "[PGUP]",        "cursor_pup");
		keyf_set(0, "^C",            "cursor_pdown");
		keyf_set(0, "[PGDN]",        "cursor_pdown");
		keyf_set(0, "^QE",           "cursor_sup");
		keyf_set(0, "^QX",           "cursor_sdown");
		keyf_set(0, "^QR",           "cursor_top");
		keyf_set(0, "^QC",           "cursor_bottom");


		keyf_set(0, "^QP",           "jump_before");
		keyf_set(0, "^QM",           "jump_mark");
		keyf_set(0, "^QJ",           "Jump_line");

		keyf_set(0, "^K1",           "jump_Mark 1");
		keyf_set(0, "^K2",           "jump_Mark 2");
		keyf_set(0, "^K3",           "jump_Mark 3");
		keyf_set(0, "^K4",           "jump_Mark 4");

		keyf_set(0, "^Q1",           "Jump_before 1");
		keyf_set(0, "^Q2",           "jump_before 2");
		keyf_set(0, "^Q3",           "Jump_before 3");
		keyf_set(0, "^Q4",           "Jump_before 4");

		keyf_set(0, "^QG",           "jump_tag");



		keyf_set(0, "[CR]",          "line_cr");

		keyf_set(0, "^U",            "char_undo");
		keyf_set(0, "^P",            "char_input");

		keyf_set(0, "^N",            "line_cr");
		keyf_set(0, "^QL",           "line_undo");


		keyf_set(0, "^H",            "del_bs");
		keyf_set(0, "[BS]",          "del_bs");
		keyf_set(0, "^G",            "del_char");
		keyf_set(0, "[DEL]",         "del_char");
		keyf_set(0, "^QH",           "del_tkprev");
		keyf_set(0, "^T",            "del_tknext");
		keyf_set(0, "^QT",           "del_sleft");
		keyf_set(0, "^QY",           "del_sright");


		keyf_set(0, "^B",            "block_start");
		keyf_set(0, "[F10]",         "block_start");
		keyf_set(0, "^Y",            "block_cut");
		keyf_set(0, "[F08]",         "block_cut");
		keyf_set(0, "^J",            "block_paste");
		keyf_set(0, "[F09]",         "block_paste");
		keyf_set(0, "^KK",           "block_yanc");
		keyf_set(0, "^KC",           "block_copy");
		keyf_set(0, "^KY",           "block_kill");
		keyf_set(0, "^QB",           "block_chlast");

		keyf_set(0, "^KD",           "block_dup");


		keyf_set(0, "^@",            "search_paging");

		keyf_set(0, "^QF",           "search_in");
		keyf_set(0, "[F06]",         "search_in");
		keyf_set(0, "^QA",           "search_repl");
		keyf_set(0, "[F07]",         "search_repl");
		keyf_set(0, "^QO",           "search_repl_redo");

		keyf_set(0, "^L",            "search_getword");
		keyf_set(0, "[F05]",         "search_getword");



		keyf_set(0, "^_",            "misc_kmacro");
		keyf_set(0, "^V",            "opt_set OverWrite");
		keyf_set(0, "[INS]",         "opt_set OverWrite");
		keyf_set(0, "[ESC]E",        "misc_exec");

		keyf_set(0, "[F01]",         "menu_file");
		keyf_set(0, "[F04]",         "menu_opt");
		keyf_set(0, "^KI",           "opt_tab");

		keyf_set(0, "[ESC]D",        "misc_redraw");
	}

	n = keydef_num(1);
	if(n == 0) {
		keyf_set(1, "[ESC]",         "sysEscape");
		keyf_set(1, "[CR]",          "sysReturn");

		keyf_set(1, "^E",            "sysCursorup");
		keyf_set(1, "[UP]",          "sysCursorup");
		keyf_set(1, "^X",            "sysCursordown");
		keyf_set(1, "[DOWN]",        "sysCursordown");
		keyf_set(1, "^S",            "sysCursorleft");
		keyf_set(1, "[LEFT]",        "sysCursorleft");
		keyf_set(1, "^D",            "sysCursorright");
		keyf_set(1, "[RIGHT]",       "sysCursorright");

		keyf_set(1, "^H",            "sysBackspace");
		keyf_set(1, "[BS]",          "sysBackspace");
		keyf_set(1, "^G",            "sysDeletechar");
		keyf_set(1, "[DEL]",         "sysDeletechar");
		keyf_set(1, "^P",            "sysCntrlinput");

		keyf_set(1, "^R",            "sysScrolldown");
		keyf_set(1, "^C",            "sysScrollup");
		keyf_set(1, "^W",            "sysRollup");
		keyf_set(1, "^Z",            "sysRolldown");

		keyf_set(1, "^QE",           "sysCursorupside");
		keyf_set(1, "^QX",           "sysCursordownside");
		keyf_set(1, "^QS",           "sysCursorleftside");
		keyf_set(1, "^QD",           "sysCursorrightside");
		keyf_set(1, "^QR",           "sysCursortopside");
		keyf_set(1, "^QC",           "sysCursorendside");
	}

	n = keydef_num(2);
	if(n == 0) {
		keyf_set(2, "[ESC]",         "effEscape");
		keyf_set(2, "^E",            "effCursorUp");
		keyf_set(2, "[UP]",          "effCursorUp");
		keyf_set(2, "^X",            "effCursorDown");
		keyf_set(2, "[DOWN]",        "effCursorDown");
		keyf_set(2, "^S",            "effWindowChange");
		keyf_set(2, "[LEFT]",        "effWindowChange");
		keyf_set(2, "^D",            "effWindowChange");
		keyf_set(2, "[RIGHT]",       "effWindowChange");
		keyf_set(2, "[tab]",         "effWindowChange");
		keyf_set(2, "w",             "effWindowNumChange");
		keyf_set(2, "@",             "effReRead");
		keyf_set(2, "r",             "effRename");
		keyf_set(2, "k",             "effMkdir");
		keyf_set(2, "^R",            "effPageUp");
		keyf_set(2, "[PPAGE]",       "effPageUp");
		keyf_set(2, "^C",            "effPageDown");
		keyf_set(2, "[NPAGE]",       "effPageDown");
		keyf_set(2, "^W",            "effRollUp");
		keyf_set(2, "^Z",            "effRollDown");
		keyf_set(2, "l",             "effChangeDir");
		keyf_set(2, "/",             "effChangeDir /");
		keyf_set(2, "\\",            "effChangeDir /");
		keyf_set(2, "~",             "effChangeDir ~");
		keyf_set(2, "q",             "effChangeDir ~");
		keyf_set(2, "[BS]",          "effChangeDir ..");
		keyf_set(2, "[CR]",          "effReturn");
		keyf_set(2, "c",             "effFileCp");
		keyf_set(2, "m",             "effFileMv");
		keyf_set(2, "d",             "effFileRm");
		keyf_set(2, "[SPACE]",       "effMarkChange");
		keyf_set(2, "*",             "effMarkChangeAll");
		keyf_set(2, "x",             "effExec");
		keyf_set(2, "h",             "effExec");
	}
}

static void add_hide_ext(const char *str)
{
	hiext_t *hiext;

	if((hiext = (hiext_t *)malloc(sizeof(hiext_t))) != NULL) {
		hiext->ext = strdup(str);
		hiext->next = sysinfo.hiext;
		sysinfo.hiext = hiext;
	}
}

static void set_hide_ext()
{
	char *p;
	char str[MAXLINESTR + 1];
	int length;
	hiext_t *next;
	hiext_t *hiext = sysinfo.hiext;

	while(hiext != NULL) {
		next = hiext->next;
		if(hiext->ext != NULL) {
			free(hiext->ext);
		}
		free(hiext);
		hiext = next;
	}
	sysinfo.hiext = NULL;
	p = hash_get(sysinfo.vp_def, "HideExt");
	if(p != NULL) {
		length = 0;
		while(*p != '\0') {
			if(*p == '.') {
				if(length > 0) {
					str[length] = '\0';
					add_hide_ext(str);
					length = 0;
				}
			} else {
				str[length++] = *p;
			}
			p++;
		}
		if(length > 0) {
			str[length] = '\0';
			add_hide_ext(str);
		}
	}
}

bool check_use_ext(char *name)
{
	char *ext = dir_pext(name);

	if(ext != NULL) {
		hiext_t *hiext = sysinfo.hiext;
		while(hiext != NULL) {
			if(!strcasecmp(ext, hiext->ext)) {
				return FALSE;
			}
			hiext = hiext->next;
		}
	}
	return TRUE;
}


void sysinfo_optset()
{
	const char *p;
	bool f;

	p = hash_get(sysinfo.vp_def, "tabcode");
	if(p == NULL) {
		sysinfo.tabcode = -1;
	} else {
		sysinfo.tabcode = *p;
	}

	p = hash_get(sysinfo.vp_def, "tabstop");
	if(p != NULL) {
		sysinfo.tabstop = atoi(p);
		if(sysinfo.tabstop <= 0) {
			char tmp[20 + 1];
 
			sysinfo.tabstop = 8;
			sprintf(tmp, "%d", sysinfo.tabstop);
			hash_set(sysinfo.vp_def, "tabstop", tmp);
		}
	}

	sysinfo.japanesef   = hash_istrue(sysinfo.vp_def, "Japanese");
	sysinfo.crmarkf     = hash_istrue(sysinfo.vp_def, "crmark");
	sysinfo.tabmarkf    = hash_istrue(sysinfo.vp_def, "tabmark");
	sysinfo.backupf     = hash_istrue(sysinfo.vp_def, "backup");
	sysinfo.autoindentf = hash_istrue(sysinfo.vp_def, "autoindent");
	sysinfo.nocasef     = hash_istrue(sysinfo.vp_def, "nocase");
	sysinfo.overwritef  = hash_istrue(sysinfo.vp_def, "overwrite");
	sysinfo.freecursorf = hash_istrue(sysinfo.vp_def, "freecursor");
	sysinfo.numberf     = hash_istrue(sysinfo.vp_def, "number");
	sysinfo.pastemovef  = hash_istrue(sysinfo.vp_def, "pastemove");

	if(hash_istrue(sysinfo.vp_def, "AnsiColor")) {
		term_color_enable();
	} else {
		term_color_disable();
	}


	sysinfo.c_block     = term_cftocol(hash_get(sysinfo.vp_def, "col_block"));
	sysinfo.c_linenum   = term_cftocol(hash_get(sysinfo.vp_def, "col_linenum"));

	sysinfo.c_ctrl      = term_cftocol(hash_get(sysinfo.vp_def, "col_ctrl"));
	sysinfo.c_crmark    = term_cftocol(hash_get(sysinfo.vp_def, "col_crmark"));

	// !!!!!!
	sysinfo.c_crmark    = (sysinfo.c_crmark & 240) | (AC_color(sysinfo.c_ctrl));

	sysinfo.c_sysmsg    = term_cftocol(hash_get(sysinfo.vp_def, "col_sysmsg"));
	sysinfo.c_search    = term_cftocol(hash_get(sysinfo.vp_def, "col_search"));
	sysinfo.c_menuc     = term_cftocol(hash_get(sysinfo.vp_def, "col_menuc"));
	sysinfo.c_menun     = term_cftocol(hash_get(sysinfo.vp_def, "col_menun"));
	sysinfo.c_eff_dirc  = term_cftocol(hash_get(sysinfo.vp_def, "col_eff_dirc"));
	sysinfo.c_eff_dirn  = term_cftocol(hash_get(sysinfo.vp_def, "col_eff_dirn"));
	sysinfo.c_eff_normc = term_cftocol(hash_get(sysinfo.vp_def, "col_eff_normc"));
	sysinfo.c_eff_normn = term_cftocol(hash_get(sysinfo.vp_def, "col_eff_normn"));

	set_hide_ext();
}

void opt_set(const char *s, const char *t)
{
	if(t != NULL) {
		hash_set(sysinfo.vp_def, s, t);
	} else {
		if(hash_istrue(sysinfo.vp_def, s)) {
			hash_set(sysinfo.vp_def, s, "false");
		} else {
			hash_set(sysinfo.vp_def, s, "true");
		}
	}
	sysinfo_optset();
}

SHELL void op_opt_set()
{
	int n;

	n = keyf_numarg();
	if(n >= 1) {
		opt_set(keyf_getarg(0), keyf_getarg(1));
	}
}

void opt_default()
{
	hash_set(sysinfo.vp_def, "TabStop", "8");
	hash_set(sysinfo.vp_def, "tabcode", ">");
	hash_set(sysinfo.vp_def, "Japanese", "true");
	hash_set(sysinfo.vp_def, "AutoIndent", "true");
	hash_set(sysinfo.vp_def, "ansicolor", "true");

	hash_set(sysinfo.vp_def, "col_block", "R");
	hash_set(sysinfo.vp_def, "col_linenum", "4");
	hash_set(sysinfo.vp_def, "col_ctrl", "4");
	hash_set(sysinfo.vp_def, "col_crmark", "4");
	hash_set(sysinfo.vp_def, "col_sysmsg", "B");
	hash_set(sysinfo.vp_def, "col_search", "3R");
	hash_set(sysinfo.vp_def, "col_menuc", "6R");
	hash_set(sysinfo.vp_def, "col_eff_dirc", "3R");
	hash_set(sysinfo.vp_def, "col_eff_dirn", "6");
	hash_set(sysinfo.vp_def, "col_eff_normc", "3R");
}

void option_set_proc(int a, mitem_t *mip, void *vp)	// !!!!
{
	char *opt_kc[] = {"EUC", "JIS", "ShiftJIS", "UTF-8", "UTF-8 BOM" };
	char *opt_rm[] = {"LF", "CRLF", "CR"};

	switch(a) {
	case optionNumber:
		sprintf(mip->str, "%s%-9s", OPT_NUMBER_MSG, (hash_istrue(sysinfo.vp_def, "number") ? "ON" : "OFF"));
		break;
	case optionOverWrite:
		sprintf(mip->str, "%s%-9s", OPT_INPUT_MODE_MSG, (hash_istrue(sysinfo.vp_def, "overwrite") ? "ON" : "OFF"));
		break;
	case optionFreeCursor:
		sprintf(mip->str, "%s%-9s", OPT_CURSOR_MODE_MSG, (hash_istrue(sysinfo.vp_def, "freecursor") ? "ON" : "OFF"));
		break;
	case optionAutoIndent:
		sprintf(mip->str, "%s%-9s", OPT_AUTOINDENT_MODE_MSG, (hash_istrue(sysinfo.vp_def, "autoindent") ? "ON" : "OFF"));
		break;
	case optionBackup:
		sprintf(mip->str, "%s%-9s", OPT_BACKUP_MSG, (hash_istrue(sysinfo.vp_def, "backup") ? "ON" : "OFF"));
		break;
	case optionTabMark:
		sprintf(mip->str, "%s%-9s", OPT_TAB_MARK_MSG, (hash_istrue(sysinfo.vp_def, "tabmark") ? "ON" : "OFF"));
		break;
	case optionCrMark:
		sprintf(mip->str, "%s%-9s", OPT_CR_MARK_MSG, (hash_istrue(sysinfo.vp_def, "crmark") ? "ON" : "OFF"));
		break;
	case optionNoCase:
		sprintf(mip->str, "%s%-9s", OPT_SEARCH_CH_MSG, (hash_istrue(sysinfo.vp_def, "nocase") ? "OFF" : "ON"));
		break;
	case optionJapanese:
		sprintf(mip->str, "%s%-9s", OPT_MESSAGE_MSG, (hash_istrue(sysinfo.vp_def, "Japanese") ? "ON" : "OFF"));
		break;
	case optionKanjiCode:
		sprintf(mip->str, "%s%-9s", OPT_KANJICODE_MSG, opt_kc[edbuf[CurrentFileNo].kc]);
		break;
	case optionRetMode:
		sprintf(mip->str, "%s%-9s", "R)CR/LF                  : ", opt_rm[edbuf[CurrentFileNo].rm]);
		break;
	case optionTabStop:
		sprintf(mip->str, "%s%-9d", OPT_TAB_STOP_MSG, atoi(hash_get(sysinfo.vp_def, "TabStop")));
		break;
	case optionPasteMove:
		sprintf(mip->str, "%s%-9s", OPT_PASTE_MOVE_MSG,  (hash_istrue(sysinfo.vp_def, "pastemove") ? "ON" : "OFF"));
		break;
	}
}

void SeeOption()
{
	menu_t menu;
	char title[MAXLINESTR + 1];
	int res = 0;

	sprintf(title, "n8 Version %.4s", VER);
	do {
		menu_iteminit(&menu);
		menu.cy = res;
		menu.title = title;
		menu_itemmake(&menu, option_set_proc, optionMax, NULL);
		res = menu_select(&menu);
		menu_itemfin(&menu);

		switch(res) {
		case optionNumber:
			opt_set("number", NULL);
			break;
		case optionOverWrite:
			opt_set("overwrite", NULL);
			break;
		case optionFreeCursor:
			opt_set("freecursor", NULL);
			break;
		case optionAutoIndent:
			opt_set("autoindent", NULL);
			break;
		case optionBackup:
			opt_set("backup", NULL);
			break;
		case optionTabMark:
			opt_set("tabmark", NULL);
			break;
		case optionCrMark:
			opt_set("crmark", NULL);
			break;
		case optionNoCase:
			opt_set("NoCase", NULL);
			break;
		case optionJapanese:
			opt_set("Japanese", NULL);
			break;
		case optionKanjiCode:
			op_opt_kanji();
			break;
		case optionRetMode:
			op_opt_retmode();
			break;
		case optionTabStop:
			op_opt_tab();
			break;
		case optionPasteMove:
			opt_set("pastemove", NULL);
			break;
		}
		CrtDrawAll();
	} while(res != -1);
}

SHELL void op_menu_opt()
{
	SeeOption();
}

SHELL void op_opt_kanji()
{
	++edbuf[CurrentFileNo].kc;
	if(edbuf[CurrentFileNo].kc > KC_utf8bom) {
		edbuf[CurrentFileNo].kc = 0;
	}
	edbuf[CurrentFileNo].cf = TRUE;
}

SHELL void op_opt_retmode()
{
	++edbuf[CurrentFileNo].rm;
	if(edbuf[CurrentFileNo].rm >= 3) {
		edbuf[CurrentFileNo].rm = 0;
	}
	edbuf[CurrentFileNo].cf = TRUE;
}

SHELL void op_opt_tab()
{
	char tmp[20 + 1];

	sysinfo.tabstop = sysinfo.tabstop == 8 ? 4 : 8;
	sprintf(tmp, "%d", sysinfo.tabstop);
	hash_set(sysinfo.vp_def, "tabstop", tmp);
}

#define	isspc(c)	((c) == ' ' || (c) == '\t')
#define	isdelim(c)	((c) == ':' || (c) == '=')
#define	isend(c)	((c) == EOF || (c) == '\r' || (c) == '\n' || (c) == '\0')

#define	MAX_nbuf	16
void config_read(char *path)
{
	int i, j;
	int n;
	FILE *fp;

	int mode;
	char com_chr;
	char q_chr;
	bool ex_flag;
	char reg_chr;
	char zone_buf[MAXLINESTR + 1];

	int c;
	char *p;
	char name_buf[MAX_nbuf][MAXLINESTR + 1];
	char val_buf[MAX_nbuf][MAXLINESTR + 1];
	int name_num;
	int val_num;

	if(strchr(path, '/') != NULL) {
		 fp = fopen(path, "r");
		 if(fp == NULL) {
		 	return;
		}
	} else {
		char buf[MAXLINESTR + 1];

		sysinfo_path(buf, path);
		fp = fopen(buf, "r");
		if(fp == NULL) {
			sprintf(buf, SYSCONFDIR "/%s", path);
			fp = fopen(buf, "r");
			if(fp == NULL) {
				return;
			}
		}
	}
	reg_chr = '\0';
	*zone_buf = '\0';

	for(;;) {
		*name_buf[0] = '\0';
		*val_buf[0] = '\0';
		name_num = 0;
		val_num = 0;

		mode = 0;
		com_chr = '\0';
		q_chr = '\0';
		ex_flag = FALSE;
		p = name_buf[0];

		for(;;) {
			c = fgetc(fp);
			if(isend(c)) {
				break;
			}

			if(!ex_flag && !q_chr && c == '#') {
				mode = 9;
			}
			if(mode == 9) {
			 	continue;
			}

			if((mode == 0 || mode == 3 || mode == 4 || mode == 6) && isspc(c)) {
				continue;
			}
			if(!ex_flag && !q_chr && isspc(c) && ((mode == 2) || (mode == 5))) {
				++mode;
				continue;
			}

			if(!ex_flag && !q_chr && isdelim(c) && mode < 4) {
				mode = 4;
				p = val_buf[0];
				continue;
			}
			if(mode == 1) {
				if(c >= 'A' && c <= 'z' && isalpha(c)) {
					com_chr = tolower(c);
					mode = 0;
					continue;
				}
				mode = 0;
				ex_flag = TRUE;
			}
			if(c == ')' && q_chr == ')') {
				q_chr = '\0';
				continue;
			}

			if(!ex_flag && !q_chr && c == '!') {
				if(mode == 0) {
					mode = 1;
					continue;
				}
				ex_flag = TRUE;
				continue;
			}

			if(ex_flag && c == '(') {
				ex_flag = FALSE;
				q_chr = ')';
				continue;
			}
			if(!ex_flag && c == '"') { //!!
				q_chr = (q_chr == '"') ? '\0' : '"';
				continue;
			}

			if(mode == 0) {
				mode = 2;
			}
			if(mode == 3) {
				mode = 2;
				++name_num;
				if(name_num < MAX_nbuf) { // !!
					p = name_buf[name_num];
					*p = '\0';
				}
			}
			if(mode == 4) {
				mode = 5;
			}
			if(mode == 6) {
				mode = 5;
				++val_num;
				if(val_num < MAX_nbuf) { // !!
					p = val_buf[val_num];
					*p = '\0';
				}
			}

			if((!q_chr || q_chr == '"') && c == '$') {
				char *q;
				char dval_buf[MAXLINESTR + 1];
				char c_chr;
				char qt_chr;

				c = fgetc(fp);
				if(c == '$') {
					*p++ = c;
					*p = '\0';
					continue;
				}

				q = dval_buf;
				qt_chr = '\0';
				c_chr = '\0';

				if(c == '*' || c == '?') {
					c_chr = c;
					c = fgetc(fp);
				}

				if(c == '(') {
					qt_chr = ')';
					c = fgetc(fp);
				}
				if(c == '{') {
					qt_chr = '}';
					c = fgetc(fp);
				}

				while(c != EOF && c != qt_chr && (qt_chr != '\0' || c != ' ')) {
					*q++ = c;
					c = fgetc(fp);
				}
				 *q = '\0';

				switch(c_chr) {
				case '?':
					*p++ = (char)strtol(dval_buf, NULL, 16);
					*p = '\0';
					continue;

				case '*':
					q = getenv(dval_buf);
					break;
				default:
					q = hash_get(sysinfo.vp_def, dval_buf);
				}

				if(q != NULL) {
					strcpy(p, q);
					p += strlen(p);
				}

				if(c == EOF) {
					break;
				}
				continue;
			}
			ex_flag = FALSE;
			*p++ = c;								// !! buffer overrun.
			*p = '\0';
		}

		if(*name_buf[name_num] != '\0') {
			++name_num;
		}
		if(*val_buf[val_num] != '\0') {
			++val_num;
		}
		if(com_chr == '\0') {
			int region;
			char buf[MAXLINESTR + 1];
			int key[2];
			keydef_t *kdp;

			switch(reg_chr) {
			case 'd':
				keys_set(name_buf[0], val_buf[0], val_num < 2 ? NULL : val_buf[1]);
				break;
			case 'k':
				region = 0;
				if(strcasecmp(zone_buf, "sys") == 0) {
					region = 1;
				}
				if(strcasecmp(zone_buf, "eff") == 0) {
					region = 2;
				}
				strcpy(buf, "op_");
				if(3 + strlen(zone_buf) < MAXLINESTR) {
					strcat(buf, zone_buf);
					if(strlen(buf) + strlen(val_buf[0]) < MAXLINESTR) {
						strcat(buf, val_buf[0]);
					}
				}
				n = keyf_getname(buf, region);
				if(n == -1) {
				 	break;
				}

				for(i = 0 ; i < name_num ; ++i) {
					keysdef_getcode(name_buf[i], key, 2);
					kdp = keydef_get(region, key[0], key[1]);
					if(kdp == NULL || kdp == (keydef_t *)-1) {
						kdp = keydef_set(region, KDM_func, n, key[0], key[1]);
					}
					if(kdp != NULL || kdp != (keydef_t *)-1) {
						for(j = 1 ; j < val_num ; ++j) {
							kdp->args[j - 1] = (char *)mem_alloc(strlen(val_buf[j]) + 1);
							strcpy(kdp->args[j - 1], val_buf[j]);
						}
						kdp->args[j - 1] = NULL;
					}
				}
				break;

			default:
				if(name_num > 0) {
					hash_set(sysinfo.vp_def, name_buf[0], val_buf[0]);
				}
			}
		}

		if(c == EOF) {
			break;
		}
		if(com_chr == 'r') {
			reg_chr = tolower(*name_buf[0]);
			strcpy(zone_buf, val_buf[0]);
			continue;
		}

		if(com_chr == 'i') {
			config_read(name_buf[0]);
			continue;
		}
	}
	fclose(fp);
}

