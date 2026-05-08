/*--------------------------------------------------------------------
  lineedit module.

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 

	n8
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#include "n8.h"
#include "lineedit.h"
#include "crt.h"
#include "list.h"
#include "keyf.h"
#include "filer.h"
#include "setopt.h"
#include "search.h"
#include "cursor.h"
#include "sh.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif
#include <ctype.h>
#include "../lib/term.h"
#include "../lib/misc.h"


/* 新型lineedit処理系 */
int le_getcsx(le_t *lep)	// local関数
{
	int csx, i;

	csx = 0;
	for(i = 0 ; i < lep->lx ; ) {
		if(lep->buf[i] == '\0') {
			break;
		}
		csx += kanji_countdsp(&lep->buf[i], csx);
		i = kanji_posnext(i, lep->buf);
	}
	csx += lep->lx - i;
	return csx;
}

#define N_scr 8

void le_setlx(le_t *lep, int lx)
{
	int csx;

	lep->lx = kanji_poscanon(lx, lep->buf);

	csx = le_getcsx(lep);
	if(csx == lep->cx + lep->sx) {
		return;
	}

	if(csx <= lep->sx) { // スクロール位置より左に
		lep->sx = (csx - 1) / N_scr * N_scr;
	} else {
		if(csx >= lep->sx + lep->dsize - 1) { // スクロール位置より右に
			lep->sx = ((csx - lep->dsize) / N_scr + 1) * N_scr;
		}
	}
	lep->cx = csx - lep->sx;
}

void le_csrleftside(le_t *lep)
{
	le_setlx(lep, 0);
}

void le_csrrightside(le_t *lep)
{
	le_setlx(lep, (int)strlen(lep->buf));
}

void le_csrleft(le_t *lep)
{
	if(lep->lx > 0) {
		le_setlx(lep, lep->lx - 1);
	}
}

void le_csrright(le_t *lep)
{
	if(lep->lx < strlen(lep->buf)) {
		le_setlx(lep, lep->lx + kanji_countbuf(&lep->buf[lep->lx]));
	}
}



void le_edit(le_t *lep, unsigned long ch, int cm)
{
	int strlength;
	char *p;

	strlength = (int)strlen(lep->buf);
	if(lep->lx > strlength) {
	 	lep->lx = strlength;
	}

	switch(cm) {
	case EDIT_BACKSPACE:
		if(lep->lx <= 0) {
			break;
		}
		le_csrleft(lep);
	case EDIT_DELETE:
		if(lep->lx < strlength) {
			p = lep->buf + lep->lx;
			int w = kanji_countbuf(&lep->buf[lep->lx]);
			memmove(p, p + w, strlength - lep->lx - (w - 1));
		}
		break;
	default:
		if(strlength < lep->size) {
			p = lep->buf + lep->lx;
			int count = 1;
			int bit = 0;
			if(ch >= 0x1000000) {
				count = 4;
				bit = 24;
			} else if(ch >= 0x10000) {
				count = 3;
				bit = 16;
			} else if(ch >= 0x100) {
				count = 2;
				bit = 8;
			}
			while(count > 0) {
				memmove(p + 1, p, strlength - lep->lx + 1);
				*(char *)p = (ch >> bit) & 0xff;
				++p;
				count--;
				bit -= 8;
			}

			le_csrright(lep);
		}
	}
}

size_t le_regbuf(const char *s, char *t, char *ac)
{
	int n, a;
	int width;
	int pos = 0;

	n = 0;
	while(n < MAXEDITLINE) {
		if(*s == '\0') {
			break;
		} else if(*s == '\t') {
			char c;

			c = sysinfo.tabcode;
			t[n] = sysinfo.tabmarkf && c != -1 ? c : ' ';
			if(ac != NULL) {
				ac[n] = sysinfo.tabmarkf && c != -1 ? sysinfo.c_tab : 0;
			}
			a = (pos / edbuf[CurrentFileNo].tabstop + 1) * edbuf[CurrentFileNo].tabstop;
			while(pos < a) {
			 	t[n + 1] = ' ';
				if(ac != NULL) {
			 		ac[n + 1] = 0;
			 	}
			 	n++;
			 	pos++;
			}
			s++;
		} else if(iscnt(*s)) {
			if(ac != NULL) {
				ac[n] = (char)sysinfo.c_ctrl;
			}
			t[n++] = '^';
			if(ac != NULL) {
				ac[n] = (char)sysinfo.c_ctrl;
			}
			t[n++] = *s + '@';
			s++;
			pos += 2;
		} else {
			width = kanji_countbuf(s);
			if(is_zen_space(s)) {
				memcpy(&t[n], sysinfo.zenspacechar, width);
				if(ac != NULL) {
					ac[n] = (char)sysinfo.c_zenspace;
				}
			} else {
				memcpy(&t[n], s, width);
				if(ac != NULL) {
					memset(&ac[n], 0, width);
				}
			}
			pos += kanji_countdsp(s, -1);
			n += width;
			s += width;
		}
	}
	t[n] = '\0';
	return n;
}

/* legets 処理系 */

void legets_hist(le_t *lep, int hn, HistoryData **hi)
{
	lep->lx = 0;
	lep->sx = 0;

	if(*hi == NULL || (*hi)->buffer == NULL) { // buffer
		*lep->buf = '\0';
	} else {
		strcpy(lep->buf, (*hi)->buffer);		// buffer
		le_csrrightside(lep);
	}
}

int legets_histprev(le_t *lep, int hn, HistoryData **hi)
{
	char path[LN_path + 1];

	if(hn == -1) {
		return FALSE;
	}
	if(hn == historyOpen) {
		path[0] = '\0';
		if(CurrentFileNo >= 0 && CurrentFileNo < MAX_edbuf) {
			strcpy(path, edbuf[CurrentFileNo].path);
		}
	}
	if(*hi == NULL) {
		*hi = history_get_last(hn);
		legets_hist(lep, hn, hi);
		if(hn != historyOpen || strcmp(lep->buf, path)) {
			return TRUE;
		}
	}
	while(1) {
		if((*hi)->prev == NULL || (*hi)->prev == history_get_top(hn)) {
			return FALSE;
		}
		*hi = (*hi)->prev;
		legets_hist(lep, hn, hi);
		if(hn != historyOpen || strcmp(lep->buf, path)) {
			break;
		}
	}
	return TRUE;
}

int legets_histnext(le_t *lep, int hn, HistoryData **hi)
{
	if(hn == -1 || *hi == NULL || (*hi)->next == NULL) {
		return FALSE;
	}
	*hi = (*hi)->next;
	legets_hist(lep, hn, hi);
	return TRUE;
}

static le_t *legets_lep;

dspfmt_t *dspreg_legets(void *vp, int a, int sizex, int sizey)
{
	dspfmt_t *dfp, *dfpb;
	char buf[MAXEDITLINE + 1];
	int n;

	dfpb = dfp = dsp_fmtinit((char *)vp, NULL);
	dfp->col = sysinfo.c_frame;

	n = (int)le_regbuf(legets_lep->buf, buf, NULL);
	dfp = dsp_fmtinit(buf + legets_lep->sx, dfp);

	return dfpb;
}

dspfmt_t *dspreg_path(void *vp, int a, int sizex, int sizey)
{
	dspfmt_t *dfp;

	dfp = dsp_fmtinit((char *)vp, NULL);
	dfp->col = sysinfo.c_frame;

	return dfp;
}

void clear_file_list(flist_t *flp)
{
	fitem_t *fitem;

	fitem = flp->fitem;
	while(fitem != NULL) {
		fitem = fitem_free(fitem);
	}
	flp->fitem = NULL;
}

void get_file_list(flist_t *flp, char *path, char *header)
{
#ifdef _WIN32
	HANDLE h;
	WIN32_FIND_DATA fd;
	wchar_t wpath[LN_path + 1];
	int unc_root;
#else
	DIR *dir;
	struct dirent *entry;
	struct stat file_stat;
#endif
	char name[LN_path + 1];
	fitem_t *fitem, *next;
	int header_length = (int)strlen(header);
	int length = (int)strlen(path);

	fitem = NULL;
	flp->fitem = NULL;
	flp->n = 0;
#ifdef _WIN32
	unc_root = check_unc_root(path);
	utf8_to_wchar(path, wpath, LN_path);
	lstrcat(wpath, L"/*.*");
	h = FindFirstFile(wpath, &fd);
	if(h == INVALID_HANDLE_VALUE) {
		return;
	}
	start_mask_reg();
	do {
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}
		wchar_to_utf8(fd.cFileName, name, LN_path);
		if(header_length > 0 && _strnicmp(name, header, header_length)) {
			continue;
		}
		if(unc_root && !strcmp(name, "..")) {
			continue;
		}
		if(check_use_ext(name, path)) {
			next = (fitem_t *)mem_alloc(sizeof(fitem_t));
			next->next = NULL;
			next->fn = _strdup(name);
			if(flp->fitem == NULL) {
				flp->fitem = next;
			} else {
				fitem->next = next;
			}
			fitem = next;
			++flp->n;
		}
	} while(FindNextFile(h, &fd));
	end_mask_reg();
	FindClose(h);
#else
	dir = opendir(path);
	if(dir == NULL) {
		return;
	}

	start_mask_reg();
	for(;;) {
		entry = readdir(dir);
		if(entry == NULL) {
		 	break;
		}
		sprintf(name, "%.*s/%.*s", length, path, LN_path - length - 1, entry->d_name);
		if(!stat(name, &file_stat)) {
			if(S_ISDIR(file_stat.st_mode)) {
				continue;
			}
		}
		if(header_length > 0 && strncasecmp(entry->d_name, header, header_length)) {
			continue;
		}
		if(check_use_ext(entry->d_name, path)) {
			next = (fitem_t *)mem_alloc(sizeof(fitem_t));
			next->next = NULL;
			next->fn = strdup(entry->d_name);
			if(flp->fitem == NULL) {
				flp->fitem = next;
			} else {
				fitem->next = next;
			}
			fitem = next;
			++flp->n;
		}
	}
	end_mask_reg();
	closedir(dir);
#endif
}

void set_input_string(le_t *le, char *input)
{
	strcpy(le->buf, input);
	le->sx = 0;
	le->cx = 0;
	le->lx = 0;
	le->l_sx = 0;
	le_setlx(le, (int)strlen(input));
}

void correct_path(char *path, const char *current_path)
{
#ifdef _WIN32
	change_dir_char(path);
	if(*(path + 1) != ':' && !(*path == '/' && *(path + 1) == '/')) {
#else
	if(*path != '/') {
#endif
		char temp[LN_path + 1];
		if(*path == '.' && *(path + 1) == '/') {
			strcpy(temp, path + 2);
		} else {
			strcpy(temp, path);
		}
#ifdef _WIN32
		if(*path == '/') {
			path[0] = current_path[0];
			path[1] = current_path[1];
			path[2] = '\0';
		} else {
#endif
			strcpy(path, current_path);
			strcat(path, "/");
#ifdef _WIN32
		}
#endif
		strcat(path, temp);
	}
}

extern char vertical_line_char[];

int legets_gets(const char *msg, char *s, int dsize, int size, int hn, int py)
{
	dspreg_t *drp[4];
	le_t le;
	int key, ret;
	int cx, cy;
	flist_t fl;
	fitem_t *fitem;
	char top[LN_dspbuf + 1];
	char bottom[LN_dspbuf + 1];
	char title[LN_dspbuf + 1];
	char path[LN_path + 1];
	int input_length = 0;
	char input[LN_path + 1];
	HistoryData *hi = NULL;

	legets_lep = &le;
	OnMessage_Flag = TRUE;

	fitem = NULL;
	strcpy(title, msg);
	path[0] = '\0';
	input[0] = '\0';
	if(hn != -1) {
		if(hn == historyOpen) {
			char temp[LN_dspbuf + 1];
			if(CurrentFileNo == MAX_edbuf) {
				getcwd(path, LN_path);
			} else {
				char *pt;

				strcpy(path, edbuf[CurrentFileNo].path);
				if((pt = strrchr(path, '/')) != NULL) {
					*pt = '\0';
#ifdef _WIN32
					if(strlen(path) == 2) {
						*pt = '/'; *(pt + 1) = '\0';
					}
#endif
				}
			}
			get_file_list(&fl, path, input);
			fitem = fl.fitem;
			strcpy(title, msg);
			strjfcpy(temp, path, LN_dspbuf, dsize - get_display_length(msg) - 6, FALSE);
			strcat(title, " ");
			strcat(title, temp);
		} else if(hn == historySearch) {
			sprintf(title, "%s OPT:%s%s%s", msg, sysinfo.searchwordf ? "W" : "", sysinfo.nocasef ? "C" : "", sysinfo.searchregf ? "R" : "");
		}
	}
	if(CurrentFileNo >= 0 && CurrentFileNo < MAX_edbuf) {
		cx = GetCol();
		if(split_mode == splitVertical && CurrentFileNo == split_file_no[splitRight]) {
			cx += get_split_start(splitVertical);
		}
		if(cx > dspall.sizex - dsize - 2) {
			cx = dspall.sizex - dsize - 2;
		}
		cy = GetRow() + 1;
		if(split_mode == splitHorizon && CurrentFileNo == split_file_no[splitLower]) {
			cy += get_split_start(splitHorizon);
		}
		if(cy > dspall.sizey - 4) {
			cy = dspall.sizey - 4;
		}
	} else {
		cx = 0;
		if(hn == historyOpen) {
			cy = term_starty();
			if(cy >= dspall.sizey - 4) {
				cy = dspall.sizey - 4;
			}
		} else if(py!= -1) {
			cy = py;
			if(cy >= term_sizey() - 2) {
				cy = term_sizey() - 6;
			}
		} else {
			cy = 1;
		}
	}
	if(hn == historyMask) {
		cy = 1;
	}
	make_frame_top_bottom(top, title, dsize + 1, FALSE);
	drp[0] = dsp_reginit();
	drp[0]->func = dspreg_path;
	drp[0]->x = cx;
	drp[0]->y = cy;
	drp[0]->sizex = dsize + 1;
	drp[0]->sizey = 1;
	drp[0]->vp = (void *)top;
	dsp_regadd(drp[0]);
	cy++;

	drp[1] = dsp_reginit();
	drp[1]->func = dspreg_legets;
	drp[1]->x = cx;
	drp[1]->y = cy;
	drp[1]->sizex = dsize;
	drp[1]->sizey = 1;
	if(sysinfo.framechar >= frameCharFrame) {
		drp[1]->vp = vertical_line_char;
	} else {
		drp[1]->vp = (sysinfo.framechar == frameCharASCII) ? "|" : " ";
	}
	dsp_regadd(drp[1]);

	drp[2] = dsp_reginit();
	drp[2]->func = dspreg_path;
	drp[2]->x = cx + dsize;
	drp[2]->y = cy;
	drp[2]->sizex = 1;
	drp[2]->sizey = 1;
	if(sysinfo.framechar >= frameCharFrame) {
		if(check_frame_ambiguous2() && sysinfo.framechar != frameCharTeraTerm) {
			drp[2]->x--;
			drp[2]->sizex++;
		}
		drp[2]->vp = vertical_line_char;
	} else {
		drp[2]->vp = (sysinfo.framechar == frameCharASCII) ? "|" : " ";
	}
	dsp_regadd(drp[2]);

	make_frame_top_bottom(bottom, "", dsize + 1, TRUE);
	drp[3] = dsp_reginit();
	drp[3]->func = dspreg_path;
	drp[3]->x = cx;
	drp[3]->y = cy + 1;
	drp[3]->sizex = dsize + 1;
	drp[3]->sizey = 1;
	drp[3]->vp = (void *)bottom;
	dsp_regadd(drp[3]);

	le.dx = (check_frame_ambiguous2() &&  sysinfo.framechar != frameCharTeraTerm) ? 2 : 1;
	le.dsize = dsize - 2;
	le.size = size;
	strcpy(le.buf, s);
	le.sx = 0;
	le.cx = 0;
	le.lx = 0;
	le.l_sx = 0;
	if(hn != historyShell) {
		le_setlx(&le, (int)strlen(s));
	}

	CrtDrawAll();
	for(;;) {
		dsp_allview();
		term_locate(cy, cx + le.dx + le.cx);

		term_csrn();
		key = get_keyf(1);
		switch(key) {
		case -1:
			continue;
		case KF_SysCursorleft:
			le_csrleft(&le);
			continue;
		case KF_SysCursorright:
			le_csrright(&le);
			continue;
		case KF_SysCursorleftside:
			le_csrleftside(&le);
			continue;
		case KF_SysCursorrightside:
			le_csrrightside(&le);
			continue;
		case KF_SysCursorup:
			legets_histprev(&le, hn, &hi);
			continue;
		case KF_SysCursordown:
			legets_histnext(&le, hn, &hi);
			continue;
		case KF_SysReturn:
			strcpy(s, le.buf);
			if(hn != -1 && hn != historyOpen && *s != '\0') {
				HistoryData *hi;

				hi = history_make_data(s);
				history_append_last(hn, hi);
			}
			ret = TRUE;
			break;
		case KF_SysEscape:
			ret = ESCAPE;
			break;
		case KF_SysBackspace:
			if(hn == historyOpen) {
				if(input_length > 0) {
					input_length--;
					while(input_length > 0 && iskanji(input[input_length])) {
						input_length--;
					}
					input[input_length] = '\0';
					clear_file_list(&fl);
					get_file_list(&fl, path, input);
					fitem = fl.fitem;
					set_input_string(&le, input);
					continue;
				}
				input_length = 0;
				input[0] = '\0';
				clear_file_list(&fl);
				get_file_list(&fl, path, input);
				fitem = fl.fitem;
			}
			le_edit(&le, ' ', EDIT_BACKSPACE);
			continue;
		case KF_SysDeletechar:
			le_edit(&le, ' ', EDIT_DELETE);
			continue;
		case KF_SysCntrlInput:
			putDoubleKey(CNTRL('P'));
			system_guide();
			term_locate(cy, le.dx + le.cx);
			key = term_inkey();
			delDoubleKey();
			le_edit(&le, key, EDIT_NONE);
			continue;
		case KF_SysOptionMenu:
			if(hn == historySearch) {
				search_option(cx, cy + 1);
				sprintf(title, "%s OPT:%s%s%s", msg, sysinfo.searchwordf ? "W" : "", sysinfo.nocasef ? "C" : "", sysinfo.searchregf ? "R" : "");
				make_frame_top_bottom(top, title, dsize + 1, FALSE);
			}
			continue;
		default:
			if(key & KF_normalcode) {
				int c;
				unsigned long ch, code;
				code = key & ~KF_normalcode;
				if(code == 0x09) {
					if(hn == historyOpen) {
						if(fitem != NULL) {
							strcpy(le.buf, fitem->fn);
							le.sx = 0;
							le.cx = 0;
							le.lx = 0;
							le.l_sx = 0;
							le_setlx(&le, (int)strlen(fitem->fn));
							fitem = fitem->next;
							if(fitem == NULL) {
								fitem = fl.fitem;
							}
						}
					}
				} else {
					if(input_length < LN_path) {
						input[input_length++] = (char)code;
					}
					ch = code & 0xf0;
					if(ch >= 0xc0) {
						code <<= 8;
						c = term_inkey();
						if(input_length < LN_path) {
							input[input_length++] = (char)c;
						}
						code |= c;
						if(ch >= 0xe0) {
							code <<= 8;
							c = term_inkey();
							if(input_length < LN_path) {
								input[input_length++] = (char)c;
							}
							code |= c;
							if(ch >= 0xf0) {
								code <<= 8;
								c = term_inkey();
								if(input_length < LN_path) {
									input[input_length++] = (char)c;
								}
								code |= c;
							}
						}
					}
					input[input_length] = '\0';
					le_edit(&le, code, EDIT_NONE);
					if(hn == historyOpen) {
						set_input_string(&le, input);
						clear_file_list(&fl);
						get_file_list(&fl, path, input);
						fitem = fl.fitem;
					}
				}
		  	}
			continue;
		}
		break;
	}
	if(hn == historyOpen) {
		correct_path(s, path);
		clear_file_list(&fl);
	}
	dsp_regfin(drp[0]);
	dsp_regfin(drp[1]);
	dsp_regfin(drp[2]);
	dsp_regfin(drp[3]);
	RefreshMessage();
	return ret;
}

