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
#include "filer.h"
#include "../lib/term_inkey.h"
#include "sh.h"
#include <ctype.h>
#include <regex.h>

enum {
	optionCrMark,
	optionTabMark,
	optionEofMark,
	optionNumber,
	optionZenSpace,
	optionSystemInfo,
	optionUnderLine,
	optionAutoIndent,
	optionBackup,
	optionPasteMove,
	optionJapanese,
	optionKanjiCode,
	optionRetMode,
	optionAmbiguous,
	optionTabStop,

	optionMax
};

enum {
	optionSearchWord,
	optionSearchNoCase,
	optionSearchReg,

	optionSearchMax
};

static char *opt_kc[] = {"EUC", "JIS", "ShiftJIS", "UTF-8", "UTF-8 BOM" };
static char *opt_rm[] = {"LF", "CRLF", "CR"};
static char *opt_am[] = {"1", "2", "Emoji2" };

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
		keyf_set(2, "s",             "effSort");
		keyf_set(2, "e",             "effMaskExt");
	}
}

void clear_string_item(int no)
{
	sitem_t *item, *next;

	item = sysinfo.sitem[no];
	while(item != NULL) {
		next = item->next;
		if(item->str != NULL) {
			free(item->str);
		}
		free(item);
		item = next;
	}
	sysinfo.sitem[no] = NULL;
}

char *get_string_item(int no, int pos)
{
	if(pos >= 0) {
		sitem_t *item;

		item = sysinfo.sitem[no];
		while(item != NULL) {
			if(pos == 0) {
				return item->str;
			}
			pos--;
			item = item->next;
		}
	}
	return NULL;
}

int get_string_item_count(int no)
{
	sitem_t *item;
	int count = 0;

	item = sysinfo.sitem[no];
	while(item != NULL) {
		count++;
		item = item->next;
	}
	return count;
}

void add_string_item(int no, const char *str)
{
	sitem_t *item, *last;

	if((item = (sitem_t *)malloc(sizeof(sitem_t))) != NULL) {
		item->str = strdup(str);
		item->next = NULL;
		if(sysinfo.sitem[no] == NULL) {
			sysinfo.sitem[no] = item;
		} else {
			last = sysinfo.sitem[no];
			while(last->next != NULL) {
				last = last->next;
			}
			last->next = item;
		}
	}
}

void set_ext_item(int no, const char *p)
{
	clear_string_item(no);
	if(p != NULL) {
		char str[MAXLINESTR + 1];
		int length = 1;

		str[0] = '*';
		if(sysinfo.maskregf) {
			add_string_item(no, p);
		} else {
			while(*p != '\0') {
				if(*p == ' ' || *(p + 1) == '\0') {
					if(*(p + 1) == '\0') {
						str[length++] = *p;
					}
					if(length > 1) {
						str[length] = '\0';
						if(str[1] == '.') {
							add_string_item(no, str);
						} else {
							add_string_item(no, &str[1]);
						}
						length = 1;
					}
				} else if(length < MAXLINESTR) {
					str[length++] = *p;
				}
				p++;
			}
		}
	}
}

int wildcard(const char *name, const char *pattern)
{
	if(*pattern == '\0') {
		return (*name == '\0');
	} else if(*pattern == '*') {
		return wildcard(name, pattern + 1) || (*name != '\0') && wildcard(name + 1, pattern);
	} else if(*pattern == '?') {
		return (*name != '\0') && wildcard(name + 1, pattern + 1);
	}
	return (toupper(*pattern) == toupper(*name)) && wildcard(name + 1, pattern + 1);
}

static int reg_hide_flag;
static int reg_use_flag;
static regex_t reg_hide;
static regex_t reg_use;

void start_mask_reg()
{
	if(sysinfo.maskregf) {
		sitem_t *item;

		reg_hide_flag = FALSE;
		reg_use_flag = FALSE;
		if((item = sysinfo.sitem[itemHide]) != NULL) {
			reg_hide_flag = !regcomp(&reg_hide, item->str, REG_EXTENDED | REG_NOSUB | REG_NEWLINE);
		}
		if((item = sysinfo.sitem[itemUse]) != NULL) {
			reg_use_flag = !regcomp(&reg_use, item->str, REG_EXTENDED | REG_NOSUB | REG_NEWLINE);
		}
	}
}

void end_mask_reg()
{
	if(sysinfo.maskregf) {
		if(reg_hide_flag) {
			regfree(&reg_hide);
		}
		if(reg_use_flag) {
			regfree(&reg_use);
		}
	}
}

bool check_cmode_ext(const char *filename)
{
	bool flag = FALSE;
	sitem_t *item;
	char *name = strrchr(filename, '/');
	if(name != NULL) {
		name++;
		if((item = sysinfo.sitem[itemCext]) != NULL) {
			if(sysinfo.maskregf) {
				regex_t reg_cext;
				if(!regcomp(&reg_cext, item->str, REG_EXTENDED | REG_NOSUB | REG_NEWLINE)) {
					if(!regexec(&reg_cext, name, 0, NULL, 0)) {
						flag = TRUE;
					}
					regfree(&reg_cext);
				}
			} else {
				item = item->next;
				while(item != NULL) {
					if(wildcard(name, item->str)) {
						flag = TRUE;
						break;
					}
					item = item->next;
				}
			}
		}
	}
	return flag;
}

bool check_use_ext(const char *name, const char *path)
{
	char buff[LN_path + 1];

	sprintf(buff, "%s%s", path, name);
	if(dir_isdir(buff)) {
		return TRUE;
	}
	sitem_t *item;
	if((item = sysinfo.sitem[itemHide]) != NULL) {
		if(sysinfo.maskregf) {
			if(reg_hide_flag && !regexec(&reg_hide, name, 0, NULL, 0)) {
				return FALSE;
			}
		} else {
			while(item != NULL) {
				if(wildcard(name, item->str)) {
					return FALSE;
				}
				item = item->next;
			}
		}
	}
	if((item = sysinfo.sitem[itemUse]) != NULL) {
		if(sysinfo.maskregf) {
			if(reg_use_flag && !regexec(&reg_use, name, 0, NULL, 0)) {
				return TRUE;
			}
		} else {
			while(item != NULL) {
				if(wildcard(name, item->str)) {
					return TRUE;
				}
				item = item->next;
			}
		}
		return FALSE;
	}
	return TRUE;
}

void dir_init()
{
	char *p;

	clear_string_item(itemDir);
	add_string_item(itemDir, "~ <home dir>");
	if((p = getenv("N8_PATH")) != NULL) {
		char str[MAXLINESTR + 1];
		int length;

		length = 0;
		while(*p != '\0') {
			if(*p == ':') {
				if(length > 0) {
					str[length] = '\0';
					add_string_item(itemDir, str);
				}
			} else if(length < MAXLINESTR) {
				str[length++] = *p;
			}
			p++;
		}
		if(length > 0) {
			str[length] = '\0';
			add_string_item(itemDir, str);
		}
	}
}

void sort_init()
{
	char *p;

	if((p = hash_get(sysinfo.vp_def, "sort")) != NULL) {
		int sort = SA_none;

		if(!strcasecmp(p, "file") || !strcasecmp(p, "filename")) {
			sort = SA_fname;
		} else if(!strcasecmp(p, "ext")) {
			sort = SA_ext;
		} else if(!strcasecmp(p, "new")) {
			sort = SA_new;
		} else if(!strcasecmp(p, "old")) {
			sort = SA_old;
		} else if(!strcasecmp(p, "large")) {
			sort = SA_large;
		} else if(!strcasecmp(p, "small")) {
			sort = SA_small;
		}
		eff_set_sort(sort);
	}
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

	sysinfo.tabstop = hash_get_int(sysinfo.vp_def, "tabstop");
	if(sysinfo.tabstop <= 0) {
		sysinfo.tabstop = 8;
		hash_set_int(sysinfo.vp_def, "tabstop", sysinfo.tabstop);
	}
	sysinfo.cmode_tabstop = hash_get_int(sysinfo.vp_def, "ctabstop");
	if(sysinfo.cmode_tabstop <= 0) {
		sysinfo.cmode_tabstop = 4;
		hash_set_int(sysinfo.vp_def, "ctabstop", sysinfo.cmode_tabstop);
	}
	sysinfo.extlength = hash_get_int(sysinfo.vp_def, "extlength");
	if(sysinfo.extlength <= 0) {
		sysinfo.extlength = 4;
		hash_set_int(sysinfo.vp_def, "extlength", sysinfo.extlength);
	}

	sysinfo.file_history_count = hash_get_int(sysinfo.vp_def, "FileHistoryCount");
	sysinfo.dir_history_count = hash_get_int(sysinfo.vp_def, "DirHistoryCount");

	sysinfo.japanesef   = hash_istrue(sysinfo.vp_def, "Japanese");
	sysinfo.crmarkf     = hash_istrue(sysinfo.vp_def, "crmark");
	sysinfo.tabmarkf    = hash_istrue(sysinfo.vp_def, "tabmark");
	sysinfo.backupf     = hash_istrue(sysinfo.vp_def, "backup");
	sysinfo.autoindentf = hash_istrue(sysinfo.vp_def, "autoindent");
	sysinfo.nocasef     = hash_istrue(sysinfo.vp_def, "nocase");
	sysinfo.searchregf  = hash_istrue(sysinfo.vp_def, "searchreg");
	sysinfo.searchwordf = hash_istrue(sysinfo.vp_def, "searchword");
	sysinfo.overwritef  = hash_istrue(sysinfo.vp_def, "overwrite");
	sysinfo.numberf     = hash_istrue(sysinfo.vp_def, "number");
	sysinfo.pastemovef  = hash_istrue(sysinfo.vp_def, "pastemove");
	sysinfo.underlinef  = hash_istrue(sysinfo.vp_def, "underline");
	sysinfo.nfdf        = hash_istrue(sysinfo.vp_def, "nfd");
	sysinfo.maskregf    = hash_istrue(sysinfo.vp_def, "maskreg");
	sysinfo.eoff        = hash_istrue(sysinfo.vp_def, "eof");
	sysinfo.zenspacef   = hash_istrue(sysinfo.vp_def, "zenspace");
	sysinfo.systeminfof = hash_istrue(sysinfo.vp_def, "systeminfo");
	sysinfo.newfilef    = hash_istrue(sysinfo.vp_def, "newfile");
	sysinfo.asksavef    = hash_istrue(sysinfo.vp_def, "asksave");
	sysinfo.afterclose = afterCloseDefault;
	if((p = hash_get(sysinfo.vp_def, "afterclose")) != NULL) {
		if(!strcasecmp(p, "input")) {
			sysinfo.afterclose = afterCloseInput;
		} else if(!strcasecmp(p, "filer")) {
			sysinfo.afterclose = afterCloseFiler;
		} else if(!strcasecmp(p, "quit")) {
			sysinfo.afterclose = afterCloseQuit;
		}
	}
	if((p = hash_get(sysinfo.vp_def, "zenspacechar")) != NULL) {
		int len = kanji_countbuf(p);
		if(len == 3) {
			memcpy(sysinfo.zenspacechar, p, len);
		}
	}
	sysinfo.framechar = frameCharNothing;
	if((p = hash_get(sysinfo.vp_def, "framechar")) != NULL) {
		if(!strcasecmp(p, "ascii")) {
			sysinfo.framechar = frameCharASCII;
		} else if(!strcasecmp(p, "frame")) {
			sysinfo.framechar = frameCharFrame;
		} else if(!strcasecmp(p, "teraterm") || !strcasecmp(p, "tera")) {
			sysinfo.framechar = frameCharTeraTerm;
		}
	}
	sysinfo.ambiguous = AM_FIX1;
	if((p = hash_get(sysinfo.vp_def, "ambiguous")) != NULL) {
		if(*p == '2') {
			sysinfo.ambiguous = AM_FIX2;
		} else if(toupper(*p) == 'E') {
			sysinfo.ambiguous = AM_EMOJI2;
		}
	}
	term_set_ambiguous(sysinfo.ambiguous | ((sysinfo.framechar == frameCharTeraTerm) ? AM_TERATERM : 0));

	f = hash_istrue(sysinfo.vp_def, "Color256");
	if(hash_istrue(sysinfo.vp_def, "AnsiColor") || f) {
		term_color_enable(f);
	} else {
		term_color_disable();
	}


	sysinfo.c_block     = term_cftocol(hash_get(sysinfo.vp_def, "col_block"));
	sysinfo.c_linenum   = term_cftocol(hash_get(sysinfo.vp_def, "col_linenum"));

	sysinfo.c_ctrl      = term_cftocol(hash_get(sysinfo.vp_def, "col_ctrl"));
	sysinfo.c_crmark    = term_cftocol(hash_get(sysinfo.vp_def, "col_crmark"));
	sysinfo.c_tab       = term_cftocol(hash_get(sysinfo.vp_def, "col_tab"));
	sysinfo.c_zenspace  = term_cftocol(hash_get(sysinfo.vp_def, "col_zenspace"));
	sysinfo.c_statusbar = term_cftocol(hash_get(sysinfo.vp_def, "col_statusbar"));
	sysinfo.c_readonly  = term_cftocol(hash_get(sysinfo.vp_def, "col_readonly"));
	sysinfo.c_frame     = term_cftocol(hash_get(sysinfo.vp_def, "col_frame"));

	sysinfo.c_sysmsg    = term_cftocol(hash_get(sysinfo.vp_def, "col_sysmsg"));
	sysinfo.c_search    = term_cftocol(hash_get(sysinfo.vp_def, "col_search"));
	sysinfo.c_menuc     = term_cftocol(hash_get(sysinfo.vp_def, "col_menuc"));
	sysinfo.c_menun     = term_cftocol(hash_get(sysinfo.vp_def, "col_menun"));
	sysinfo.c_eff_dirc  = term_cftocol(hash_get(sysinfo.vp_def, "col_eff_dirc"));
	sysinfo.c_eff_dirn  = term_cftocol(hash_get(sysinfo.vp_def, "col_eff_dirn"));
	sysinfo.c_eff_normc = term_cftocol(hash_get(sysinfo.vp_def, "col_eff_normc"));
	sysinfo.c_eff_normn = term_cftocol(hash_get(sysinfo.vp_def, "col_eff_normn"));

	set_ext_item(itemHide, hash_get(sysinfo.vp_def, "HideExt"));
	set_ext_item(itemCext, hash_get(sysinfo.vp_def, "CExt"));
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
	char *env;
	char code[4] = { (char)0xe3, (char)0x83, (char)0xad, 0 };

	hash_set(sysinfo.vp_def, "TabStop", "8");
	hash_set(sysinfo.vp_def, "ctabstop", "4");
	hash_set(sysinfo.vp_def, "extlength", "4");
	hash_set(sysinfo.vp_def, "tabcode", ">");
	hash_set(sysinfo.vp_def, "Japanese", "on");
	hash_set(sysinfo.vp_def, "AutoIndent", "on");
	hash_set(sysinfo.vp_def, "ansicolor", "on");
	if((env = getenv("LANG")) != NULL) {
		hash_set(sysinfo.vp_def, "Locale", env);
	} else {
		hash_set(sysinfo.vp_def, "Locale", "ja_JP.UTF-8");
	}
	hash_set(sysinfo.vp_def, "nfd", "on");
	hash_set(sysinfo.vp_def, "eof", "on");
	hash_set(sysinfo.vp_def, "newfile", "on");
	hash_set(sysinfo.vp_def, "zenspacechar", code);
	hash_set(sysinfo.vp_def, "ambiguous", "2");
	hash_set(sysinfo.vp_def, "framechar", "ascii");
	hash_set_int(sysinfo.vp_def, "FileHistoryCount", DEFAULT_FILE_HISTORY_COUNT);
	hash_set_int(sysinfo.vp_def, "DirHistoryCount", DEFAULT_DIR_HISTORY_COUNT);
	hash_set(sysinfo.vp_def, "sort", "filename");
	hash_set(sysinfo.vp_def, "asksave", "on");

	hash_set(sysinfo.vp_def, "col_block", "R");
	hash_set(sysinfo.vp_def, "col_linenum", "4");
	hash_set(sysinfo.vp_def, "col_ctrl", "4");
	hash_set(sysinfo.vp_def, "col_crmark", "4");
	hash_set(sysinfo.vp_def, "col_tab", "4");
	hash_set(sysinfo.vp_def, "col_zenspace", "4");
	hash_set(sysinfo.vp_def, "col_readonly", "6R");
	hash_set(sysinfo.vp_def, "col_statusbar", "R");
	hash_set(sysinfo.vp_def, "col_frame", "R");
	hash_set(sysinfo.vp_def, "col_sysmsg", "3B");
	hash_set(sysinfo.vp_def, "col_search", "3R");
	hash_set(sysinfo.vp_def, "col_menuc", "");
	hash_set(sysinfo.vp_def, "col_menun", "R");
	hash_set(sysinfo.vp_def, "col_eff_dirc", "1R");
	hash_set(sysinfo.vp_def, "col_eff_dirn", "6");
	hash_set(sysinfo.vp_def, "col_eff_normc", "1R");
	hash_set(sysinfo.vp_def, "col_eff_normn", "");
}

void search_option_set_proc(int a, mitem_t *mip, void *vp)
{
	switch(a) {
	case optionSearchWord:
		sprintf(mip->str, "%s%-9s", SEARCH_WORD_MSG, (hash_istrue(sysinfo.vp_def, "searchword") ? "ON" : "OFF"));
		break;
	case optionSearchNoCase:
		sprintf(mip->str, "%s%-9s", SEARCH_NOCASE_MSG, (hash_istrue(sysinfo.vp_def, "nocase") ? SEARCH_NOCASE_ITEM : SEARCH_CASE_ITEM));
		break;
	case optionSearchReg:
		sprintf(mip->str, "%s%-9s", SEARCH_REG_MSG, (hash_istrue(sysinfo.vp_def, "searchreg") ? "ON" : "OFF"));
		break;
	}
}

void search_option(int x, int y)
{
	menu_t menu;
	int res = 0;

	menu_iteminit(&menu);
	menu.title = SEARCH_OPTION_MSG;
	menu.cy = res;
	menu.drp->x = x + 1;
	if(check_frame_ambiguous2() && (menu.drp->x & 1)) {
		menu.drp->x--;
	}
	menu.drp->y = y;
	if(menu.drp->y > dspall.sizey - optionSearchMax - 3) {
		menu.drp->y = dspall.sizey - optionSearchMax - 3;
	}
	menu_itemmake(&menu, search_option_set_proc, optionSearchMax, NULL);
	res = menu_select(&menu);
	menu_itemfin(&menu);

	switch(res) {
	case optionSearchWord:
		opt_set("searchword", NULL);
		break;
	case optionSearchNoCase:
		opt_set("nocase", NULL);
		break;
	case optionSearchReg:
		opt_set("searchreg", NULL);
		break;
	}
	CrtDrawAll();
}


void option_set_proc(int a, mitem_t *mip, void *vp)
{
	switch(a) {
	case optionCrMark:
		sprintf(mip->str, "%s%-9s", OPT_CR_MARK_MSG, (hash_istrue(sysinfo.vp_def, "crmark") ? "ON" : "OFF"));
		break;
	case optionTabMark:
		sprintf(mip->str, "%s%-9s", OPT_TAB_MARK_MSG, (hash_istrue(sysinfo.vp_def, "tabmark") ? "ON" : "OFF"));
		break;
	case optionEofMark:
		sprintf(mip->str, "%s%-9s", OPT_EOF_MARK_MSG, (hash_istrue(sysinfo.vp_def, "eof") ? "ON" : "OFF"));
		break;
	case optionNumber:
		sprintf(mip->str, "%s%-9s", OPT_NUMBER_MSG, (hash_istrue(sysinfo.vp_def, "number") ? "ON" : "OFF"));
		break;
	case optionZenSpace:
		sprintf(mip->str, "%s%-9s", OPT_ZEN_SPACE_MSG, (hash_istrue(sysinfo.vp_def, "zenspace") ? "ON" : "OFF"));
		break;
	case optionSystemInfo:
		sprintf(mip->str, "%s%-9s", OPT_STATUS_SYSTEM_MSG, (hash_istrue(sysinfo.vp_def, "systeminfo") ? "ON" : "OFF"));
		break;
	case optionUnderLine:
		sprintf(mip->str, "%s%-9s", OPT_UNDERLINE_MSG,  (hash_istrue(sysinfo.vp_def, "underline") ? "ON" : "OFF"));
		break;
	case optionAutoIndent:
		sprintf(mip->str, "%s%-9s", OPT_AUTOINDENT_MODE_MSG, (hash_istrue(sysinfo.vp_def, "autoindent") ? "ON" : "OFF"));
		break;
	case optionBackup:
		sprintf(mip->str, "%s%-9s", OPT_BACKUP_MSG, (hash_istrue(sysinfo.vp_def, "backup") ? "ON" : "OFF"));
		break;
	case optionPasteMove:
		sprintf(mip->str, "%s%-9s", OPT_PASTE_MOVE_MSG,  (hash_istrue(sysinfo.vp_def, "pastemove") ? "ON" : "OFF"));
		break;
	case optionJapanese:
		sprintf(mip->str, "%s%-9s", OPT_MESSAGE_MSG, (hash_istrue(sysinfo.vp_def, "Japanese") ? "ON" : "OFF"));
		break;
	case optionKanjiCode:
		sprintf(mip->str, "%s%-9s", OPT_KANJICODE_MSG, opt_kc[edbuf[CurrentFileNo].kc]);
		break;
	case optionRetMode:
		sprintf(mip->str, "%s%-9s", "R CR/LF                  ", opt_rm[edbuf[CurrentFileNo].rm]);
		break;
	case optionAmbiguous:
		sprintf(mip->str, "%s%-9s", OPT_AMBIGUOUS_MSG, opt_am[sysinfo.ambiguous]);
		break;
	case optionTabStop:
		sprintf(mip->str, "%s%-9d", OPT_TAB_STOP_MSG, atoi(hash_get(sysinfo.vp_def, "TabStop")));
		break;
	}
}

void SeeOption()
{
	menu_t menu;
	char title[MAXLINESTR + 1];
	int res = 0;

	sprintf(title, "n8 Version %.5s", VER);
	do {
		menu_iteminit(&menu);
		menu.drp->y = 2;
		menu.cy = res;
		menu.title = title;
		menu_itemmake(&menu, option_set_proc, optionMax, NULL);
		res = menu_select(&menu);
		menu_itemfin(&menu);

		switch(res) {
		case optionCrMark:
			opt_set("crmark", NULL);
			break;
		case optionTabMark:
			opt_set("tabmark", NULL);
			break;
		case optionEofMark:
			opt_set("eof", NULL);
			break;
		case optionNumber:
			opt_set("number", NULL);
			break;
		case optionZenSpace:
			opt_set("zenspace", NULL);
			break;
		case optionSystemInfo:
			opt_set("systeminfo", NULL);
			break;
		case optionUnderLine:
			opt_set("underline", NULL);
			break;
		case optionAutoIndent:
			opt_set("autoindent", NULL);
			break;
		case optionBackup:
			opt_set("backup", NULL);
			break;
		case optionPasteMove:
			opt_set("pastemove", NULL);
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
		case optionAmbiguous:
			{
				sysinfo.ambiguous++;
				if(sysinfo.ambiguous > AM_EMOJI2) {
					sysinfo.ambiguous = AM_FIX1;
				}
				hash_set(sysinfo.vp_def, "ambiguous", opt_am[sysinfo.ambiguous]);
				sysinfo_optset();
			}
			break;
		case optionTabStop:
			op_opt_tab();
			break;
		}
		CrtDrawAll();
	} while(res == optionKanjiCode || res == optionRetMode || res == optionAmbiguous);
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
	int n;
	char tmp[20 + 1];

	sysinfo.tabstop = sysinfo.tabstop == 8 ? 4 : 8;
	sprintf(tmp, "%d", sysinfo.tabstop);
	hash_set(sysinfo.vp_def, "tabstop", tmp);
	for(n = 0 ; n < MAX_edbuf ; n++) {
		if(!edbuf[n].cmode) {
			edbuf[n].tabstop = sysinfo.tabstop;
		}
	}
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
			if(!ex_flag && c == '"') {
				q_chr = (q_chr == '"') ? '\0' : '"';
				continue;
			}
			if(!ex_flag && c == '\'') {
				q_chr = (q_chr == '\'') ? '\0' : '\'';
				continue;
			}

			if(mode == 0) {
				mode = 2;
			}
			if(mode == 3) {
				mode = 2;
				++name_num;
				if(name_num < MAX_nbuf) {
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
				if(val_num < MAX_nbuf) {
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
			*p++ = c;
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
			case 'm':
				if(!strcasecmp(zone_buf, "mask")) {
					for(i = 0 ; i < name_num ; ++i) {
						add_string_item(itemMask, name_buf[i]);
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

