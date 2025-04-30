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
#include "sh.h"
#include <dirent.h>
#include <ctype.h>


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
	le_setlx(lep, strlen(lep->buf));
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



void le_edit(le_t *lep, int ch, int cm)
{
	int i;
	int strlength;
	void *p;

	strlength = strlen(lep->buf);
	if(lep->lx > strlength) {
	 	lep->lx = strlength;
	}

	switch(cm) {
	case BACKSPACE:
		if(lep->lx <= 0) {
			break;
		}
		le_csrleft(lep);
	case DELETE:
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
			a = (pos / sysinfo.tabstop + 1) * sysinfo.tabstop;
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
				ac[n] = sysinfo.c_ctrl;
			}
			t[n++] = '^';
			if(ac != NULL) {
				ac[n] = sysinfo.c_ctrl;
			}
			t[n++] = *s + '@';
			s++;
			pos += 2;
		} else {
			width = kanji_countbuf(s);
			memcpy(&t[n], s, width);
			if(ac != NULL) {
				memset(&ac[n], 0, width);
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

bool legets_histprev(le_t *lep, int hn, HistoryData **hi)
{
	char path[LN_path + 1];

	if(hn == -1) {
		return FALSE;
	}
	if(hn == FOPEN_SYSTEM) {
		strcpy(path, edbuf[CurrentFileNo].path);
	}
	if(*hi == NULL) {
		*hi = history_get_last(hn);
		legets_hist(lep, hn, hi);
		if(hn != FOPEN_SYSTEM || strcmp(lep->buf, path)) {
			return TRUE;
		}
	}
	while(1) {
		if((*hi)->prev == NULL || (*hi)->prev == history_get_top(hn)) {
			return FALSE;
		}
		*hi = (*hi)->prev;
		legets_hist(lep, hn, hi);
		if(hn != FOPEN_SYSTEM || strcmp(lep->buf, path)) {
			break;
		}
	}
	return TRUE;
}

bool legets_histnext(le_t *lep, int hn, HistoryData **hi)
{
	if(hn == -1 || *hi == NULL || (*hi)->next == NULL) {
		return FALSE;
	}
	*hi = (*hi)->next;
	legets_hist(lep, hn, hi);
	return TRUE;
}

le_t *legets_lep;	// !!暫定的

dspfmt_t *dspreg_legets(void *vp, int a, int sizex, int sizey)
{
	dspfmt_t *dfp, *dfpb;
	char buf[MAXEDITLINE + 1];
	int n;

	dfpb = dfp = dsp_fmtinit((char *)vp, NULL);
	dfp->col = sysinfo.c_sysmsg;

	n = le_regbuf(legets_lep->buf, buf, NULL);
	dfp = dsp_fmtinit(buf + legets_lep->sx, dfp);

	return dfpb;
}

dspfmt_t *dspreg_path(void *vp, int a, int sizex, int sizey)
{
	dspfmt_t *dfp;

	dfp = dsp_fmtinit((char *)vp, NULL);
	dfp->col = sysinfo.c_sysmsg;

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
	DIR *dir;
	struct dirent *entry;
	fitem_t *fitem, *next;
	struct stat file_stat;
	char name[LN_path + 1];
	int header_length = strlen(header);
	int length = strlen(path);

	flp->fitem = NULL;
	flp->n = 0;

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
}

void set_input_string(le_t *le, char *input)
{
	strcpy(le->buf, input);
	le->sx = 0;
	le->cx = 0;
	le->lx = 0;
	le->l_sx = 0;
	le_setlx(le, strlen(input));
}

int legets_gets(const char *msg, char *s, int dsize, int size, int hn)
{
	dspreg_t *drp, *path_drp;
	le_t le;
	int key, ret;
	flist_t fl;
	fitem_t *fitem;
	char path[LN_path + 1];
	int input_length = 0;
	char input[LN_path + 1];
	HistoryData *hi = NULL;

	legets_lep = &le;
	OnMessage_Flag = TRUE;

	drp = dsp_reginit();
	drp->y = dspall.sizey - 1;
	drp->func = dspreg_legets;
	drp->sizex = dsize;
	drp->sizey = 1;
	drp->vp = (void *)msg;

	dsp_regadd(drp);

	path[0] = '\0';
	input[0] = '\0';
	if(hn != -1) {
		if(hn == FOPEN_SYSTEM) {
			if(CurrentFileNo == MAX_edbuf) {
				getcwd(path, LN_path);
			} else {
				char *pt;

				strcpy(path, edbuf[CurrentFileNo].path);
				if((pt = strrchr(path, '/')) != NULL) {
					*pt = '\0';
				}
			}
			get_file_list(&fl, path, input);
			fitem = fl.fitem;

			path_drp = dsp_reginit();
			path_drp->y = dspall.sizey - 2;
			path_drp->func = dspreg_path;
			path_drp->sizex = dsize;
			path_drp->sizey = 1;
			path_drp->vp = (void *)path;
			dsp_regadd(path_drp);
		}
	}
	le.dx = get_display_length(msg);
	le.dsize = dsize - strlen(msg) - 2;
	le.size = size;
	strcpy(le.buf, s);
	le.sx = 0;
	le.cx = 0;
	le.lx = 0;
	le.l_sx = 0;
	if(hn != SHELLS_SYSTEM) {
		le_setlx(&le, strlen(s));
	}

	for(;;) {
		dsp_allview();
		term_locate(drp->y, le.dx + le.cx);

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
			if(hn != -1 && hn != FOPEN_SYSTEM && *s != '\0') {
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
			if(hn == FOPEN_SYSTEM) {
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
			le_edit(&le, ' ', BACKSPACE);
			continue;
		case KF_SysDeletechar:
			le_edit(&le, ' ', DELETE);
			continue;
		case KF_SysCntrlInput:
			putDoubleKey(CNTRL('P'));
			system_guide(); // !!
			term_locate(drp->y, le.dx + le.cx);
			key = term_inkey();
			delDoubleKey();
			le_edit(&le, key, NONE);
			continue;
		default:
			if(key & KF_normalcode) {
				int c;
				unsigned long ch, code;
				code = key & ~KF_normalcode;
				if(code == 0x09) {
					if(hn == FOPEN_SYSTEM) {
						if(fitem != NULL) {
							strcpy(le.buf, fitem->fn);
							le.sx = 0;
							le.cx = 0;
							le.lx = 0;
							le.l_sx = 0;
							le_setlx(&le, strlen(fitem->fn));
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
					le_edit(&le, code, NONE);
					if(hn == FOPEN_SYSTEM) {
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
	if(hn == FOPEN_SYSTEM) {
		if(*s != '/') {
			char temp[LN_path + 1];

			strcpy(temp, s);
			strcpy(s, path);
			strcat(s, "/");
			strcat(s, temp);
		}
		clear_file_list(&fl);
		term_locate(path_drp->y, 0);
		term_clrtoe(AC_normal);
		dsp_regfin(path_drp);
	}
	dsp_regfin(drp);
	RefreshMessage();
	return ret;
}

