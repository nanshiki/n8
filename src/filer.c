/*--------------------------------------------------------------------
  filer 0.2.7

    Copyright (c) 1997,1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 

	n8
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#include "n8.h"
#include "filer.h"
#include "keyf.h"
#include "input.h"
#include "crt.h"
#include "setopt.h"
#include "sh.h"
#include "../lib/misc.h"

#include <sys/time.h>
#include <time.h>
#include <ctype.h>

static fw_t fw[2];
static eff_t eff;

#define	FILE_MENU_X		1
#define	FILE_MENU_COUNT	7
#define	DIR_MENU_X		7
#define	MASK_MENU_X		12
#define	SORT_MENU_X		24
#define	SORT_MENU_COUNT	6
#define	MENU_Y			1

enum {
	fileCopy,
	fileMove,
	fileDelete,
	fileRename,
	fileMkdir,
	fileNew,
	fileExec
};

/* ディレクトリ移動遍歴処理 */
typedef struct
{
	int ct;

	char path[LN_path + 1];
	char name[LN_path + 1];
} dm_t;

#define MAX_dm 64

static dm_t dm[MAX_dm];
static int dm_num, dm_loc;

void dm_init()
{
	int i;

	dm_num = 0;
	dm_loc = 0;

	for (i = 0 ; i < MAX_dm ; ++i) {
		dm[i].ct = 0;
	}
}

void dm_set(const char *p, const char *s)
{
	int i, a, b;
	char buf[LN_path + 1];
	char path[LN_path + 1];

	realpath(p, path);
	strcpy(buf, s);
	if(*buf != '\0' && buf[strlen(buf) - 1] == '/') {
		buf[strlen(buf) - 1] = '\0';
	}
	for (i = 0 ; i < dm_num ; ++i) {
		if(strcmp(dm[i].path, path) == 0) {
			strcpy(dm[i].name, buf);
			dm[i].ct = ++dm_loc;
			return;
		}
	}

	if(dm_num < MAX_dm) {
		strcpy(dm[dm_num].path, path);
		strcpy(dm[dm_num].name, buf);
		dm[dm_num].ct = ++dm_loc;
		++dm_num;
		return;
	}

	a = dm[0].ct;
	b = 0;
	for (i = 1 ; i < MAX_dm ; ++i) {
		if(a > dm[i].ct) {
			a = dm[i].ct;
			b = i;
		}
	}

	strcpy(dm[b].path, path);
	strcpy(dm[b].name, buf);
	dm[b].ct = ++dm_loc;
}

void dm_get(const char *p, char *s)
{
	int i;
	char path[LN_path + 1];

	realpath(p, path);

	*s = '\0';
	for(i = 0 ; i < dm_num ; ++i) {
		if(strcmp(dm[i].path, path) == 0) {
			strcpy(s, dm[i].name);
			return;
		}
	}
	return;
}

void fw_init(fw_t *fwp, const char *s, int a)
{
	char path[LN_path + 1];

	*fwp->match = '\0';
	if(s != NULL) {
		strcpy(fwp->path, s);
	} else {
		getcwd(path, LN_path);
		sprintf(fwp->path, "%.*s/", LN_path - 1, path);
	}

	fwp->flist.fitem = NULL;
	fwp->flist.n = 0;
	fwp->findex = NULL;

	menu_iteminit(&fwp->menu);
	fwp->menu.title = fwp->title;

	fwp->menu.drp->sizex = term_sizex() / 2;
	if(sysinfo.framechar >= frameCharFrame) {
		fwp->menu.drp->sizex--;
		if(sysinfo.ambiguous != AM_FIX1) {
			fwp->menu.drp->sizex &= ~1;
		}
	}
	if(a == 1) {
		fwp->menu.drp->x += fwp->menu.drp->sizex;
	}
}

void eff_reinit()
{
	int sizex = term_sizex() / 2;
	if(sysinfo.framechar >= frameCharFrame) {
		sizex--;
		if(sysinfo.ambiguous != AM_FIX1) {
			sizex &= ~1;
		}
	}
	fw[0].menu.drp->sizex = sizex;
	fw[1].menu.drp->sizex = sizex;
	fw[1].menu.drp->x = fw[0].menu.drp->sizex;
}

void eff_set_sort(int sort)
{
	eff.sort = sort;
}

void eff_init(const char *s1, const char *s2)
{
	eff.wa = 0;
	eff.wn = 1;
	eff.sort = SA_none;

	fw_init(&fw[0], s1, 0);
	fw_init(&fw[1], s2, 1);
	dm_init();
}

struct stat *fw_getstat(fw_t *fwp, int a)
{
	if(a == -1) {
		a = fwp->menu.cy + fwp->menu.sy;
	}
	return &fwp->findex[a]->stat;
}

fitem_t *fw_getfi(fw_t *fwp, int a)
{
	if(a == -1) {
		a = fwp->menu.cy + fwp->menu.sy;
	}
	return fwp->findex[a];
}

void set_path_title()
{
	strcpy(fw_c.title, fw_c.path);
	if(strlen(fw_c.path) + strlen(fw_c.mask) < LN_path) {
		strcat(fw_c.title, fw_c.mask);
	}
}

void fwc_chdir(const char *s, bool f)
{
	char path[LN_path + 1];
	bool uf;

	if(*s == '\0') {
		return;
	}

	if(strcmp(s, "..") == 0) {
		uf = TRUE;
		sprintf(path, "%.*s..", LN_path - 2, fw_c.path);
		reg_path(NULL, path, FALSE);
	} else {
		uf = FALSE;
		if(f) {
			strcpy(path, s);
		} else {
			sprintf(path, "%s%s", fw_c.path, s);
		}
	}

	reg_path(fw_c.path, path, FALSE);
	if(dir_isdir(path)) {
		dm_set(fw_c.path, fw_c.findex[fw_c.menu.cy + fw_c.menu.sy]->fn);
		if(uf && strcmp(fw_c.path, "/") != 0) {
			dm_set(path, fw_c.path + strlen(path));
		}
		strcpy(fw_c.path, path);
		set_path_title();
		if(!uf || strcmp(fw_c.path, "/") == 0) {
			dm_get(path, fw_c.match);
		} else {
			strcpy(fw_c.match, fw_c.path + strlen(path));
			if(*fw_c.match != '\0' && fw_c.match[strlen(fw_c.match) - 1] == '/') {
				fw_c.match[strlen(fw_c.match) - 1] = '\0';
			}
		}
		fw_c.df = TRUE;
	}
}

void fwc_setdir(char *path)
{
	reg_path(fw_c.path, path, FALSE);
	if(dir_isdir(path)) {
		strcpy(fw_c.path, path);
		fw_make(&fw[eff.wa]);
	}
}

void fw_match(fw_t *fwp)
{
	int i;

	i = fwp->flist.n - 1;
	while(i > 0 && strcmp(fwp->match, fwp->findex[i]->fn) != 0) {
		--i;
	}
	menu_csrmove(&fwp->menu, i);
}

fitem_t *fitem_free(fitem_t *fip)
{
	fitem_t *next;

	if(fip == NULL) {
		return NULL;
	}
	next = fip->next;

	free(fip->fn);
	free(fip);

	return next;
}

void fw_free(fw_t *fwp)
{
	fitem_t *fitem;

	menu_itemfin(&fwp->menu);

	free(fwp->findex);
	fitem = fwp->flist.fitem;
	while(fitem != NULL) {
		fitem = fitem_free(fitem);
	}
	fwp->flist.fitem = NULL;
	fwp->flist.n = 0;
}

fitem_t *fitem_mk(const char *path, char *s)
{
	char buf[LN_path + 1], *p;
	fitem_t *fitem;

	fitem = (fitem_t *)mem_alloc(sizeof(fitem_t));
	fitem->next = NULL;

	fitem->fn = s;

	p = dir_pext(s);
	if(p == NULL) {
		fitem->e = s + strlen(s);
	} else {
		fitem->e = p;
	}

	sprintf(buf, "%s/%s", path, s);
	if(strcmp(s, "..") == 0) {
		reg_path(NULL, buf, FALSE);
	}
	stat(buf, &fitem->stat);

	return fitem;
}

void flist_set(flist_t *flp, const char *path)
{
	char **p;
	fitem_t *fitem = NULL;
	int count = 0;

	p = dir_glob(path, TRUE);

	start_mask_reg();
	if(check_use_ext(p[0], path)) {
		fitem = flp->fitem = fitem_mk(path, p[0]);
		count++;
	}
	flp->n = 1;
	while(p[flp->n] != NULL) {
		if(check_use_ext(p[flp->n], path)) {
			if(fitem == NULL) {
				fitem = flp->fitem = fitem_mk(path, p[flp->n]);
			} else {
				fitem->next = fitem_mk(path, p[flp->n]);
				fitem = fitem->next;
			}
			count++;
		}
		++flp->n;
	}
	end_mask_reg();
	free(p);
	flp->n = count;
}

static int findex_comp(const void *x, const void *y)
{
	fitem_t *fi_x, *fi_y;
	int i, j;
	int sort = eff.sort;

	fi_x = *(fitem_t **)x;
	fi_y = *(fitem_t **)y;

	if(strncmp(fi_x->fn, "..", 2) == 0) {
		return -1;
	}
	if(strncmp(fi_y->fn, "..", 2) == 0) {
		return 1;
	}

	if((fi_x->stat.st_mode & S_IFMT) == S_IFDIR && (fi_y->stat.st_mode & S_IFMT) != S_IFDIR) {
		return -1;
	}
	if((fi_x->stat.st_mode & S_IFMT) != S_IFDIR && (fi_y->stat.st_mode & S_IFMT) == S_IFDIR) {
		return 1;
	}
	if(*fi_x->fn == '.' && *fi_y->fn != '.') {
		return -1;
	}
	if(*fi_x->fn != '.' && *fi_y->fn == '.') {
		return 1;
	}
	if((fi_x->stat.st_mode & S_IFMT) == S_IFDIR && (fi_y->stat.st_mode & S_IFMT) == S_IFDIR) {
		if(eff.sort == SA_large || eff.sort == SA_small) {
			sort = SA_fname;
		}
	}

	j = 0;
	switch(sort) {
	case SA_none:
		return 0;
	case SA_fname:
		if((j = strcmp(fi_x->fn, fi_y->fn)) == 0) {
			j = strcmp(fi_x->e, fi_y->e);
		}
		break;
	case SA_ext:
		if((j = strcmp(fi_x->e, fi_y->e)) == 0) {
			j = strcmp(fi_x->fn, fi_y->fn);
		}
		break;
	case SA_new:
		j = fi_y->stat.st_mtime - fi_x->stat.st_mtime;
		break;
	case SA_old:
		j = fi_x->stat.st_mtime - fi_y->stat.st_mtime;
		break;
	case SA_large:
		j = fi_y->stat.st_size - fi_x->stat.st_size;
		break;
	case SA_small:
		j = fi_x->stat.st_size - fi_y->stat.st_size;
		break;
	}
	return j;
}


fitem_t **findex_get(flist_t *flp)
{
	fitem_t **findex;
	fitem_t *fitem;
	int i;

	findex = (fitem_t **)mem_alloc(flp->n * sizeof(int *));
	fitem = flp->fitem;
	for (i = 0 ; i < flp->n ; ++i) {
		findex[i] = fitem;
		fitem = fitem->next;
	}
	qsort(findex, i, sizeof(fitem_t **), findex_comp);
	return findex;
}

void prt_kmsize(char *s, off_t n)
{
	const char *scale = " KMGT";

	if(n < (off_t)10 * 1000) {
		sprintf(s, " %4d", (int)n);
		return;
	}
	while(n >= (off_t)10 * 1000 && *scale != '\0') {
		n /= 1000;
		++scale;
	}
	sprintf(s, "%4d%c", (int)n, *scale);
}

static void fw_item_proc(int a, mitem_t *mip, void *vp)
{
	char buf[MAXLINESTR + 1];
	char *s, *p;
	fw_t *fwp;

	fwp = vp;
	s = mip->str;

	strjfcpy(s, fwp->findex[a]->fn, MAXLINESTR, fwp->menu.drp->sizex - ((check_frame_ambiguous2() && sysinfo.framechar != frameCharTeraTerm) ? 32 : 30), TRUE);
	p = fwp->findex[a]->e;
	if(*p == '\0') {
		strcat(s, "     ");
	} else {
		int m, n;

		n = strlen(s);
		m = fwp->findex[a]->e - fwp->findex[a]->fn - 1;
		if(m < n) {
			memset(s + m, ' ', n - m);
		}

		strcat(s, ".");
		strjfcpy(s + strlen(s), p, MAXLINESTR, 4, TRUE);
	}

	strcat(s, " ");
	if((fwp->findex[a]->stat.st_mode & S_IFMT) == S_IFDIR) {
		strcat(s, "<DIR>"); 
		mip->cc = sysinfo.c_eff_dirc;
		mip->nc = sysinfo.c_eff_dirn;
	} else {
		prt_kmsize(buf, fwp->findex[a]->stat.st_size);
		strcat(s, buf);

		mip->cc = sysinfo.c_eff_normc;
		mip->nc = sysinfo.c_eff_normn;
	}
	strftime(buf, 15, "%y/%m/%d %R", localtime(&fwp->findex[a]->stat.st_mtime));
	if(strlen(s) + strlen(buf) < MAXLINESTR - 1) {
		strcat(s, " ");
		strcat(s, buf);
	}
}

void fw_make(fw_t *fwp)
{
	int sizex;

	if(sysinfo.nfdf && fwp->menu.drp) {
		term_redraw_box(fwp->menu.drp->x, fwp->menu.drp->y, fwp->menu.drp->sizex, term_sizey());
	}

	sizex = -1;

	if(fwp->flist.fitem != NULL) {
		fw_free(fwp);
		sizex = fwp->menu.drp->sizex;
	}

	dm_get(fwp->path, fwp->match);

	flist_set(&fwp->flist, fwp->path);
	fwp->findex = findex_get(&fwp->flist);

	fwp->marknum = 0;
	fwp->markmax = 0;
	fwp->mark = mem_alloc(sizeof(int) * fwp->flist.n);
	memset(fwp->mark, 0, sizeof(int) * fwp->flist.n);

	fwp->menu.sy = 0;
	fwp->menu.cy = 0;
	menu_itemmake(&fwp->menu, fw_item_proc, fwp->flist.n, fwp);

	if(sizex != -1) {
		fwp->menu.drp->sizex = sizex;
	}

	fw_match(fwp);
}


int fw_getmarkfirst(fw_t *fwp)
{
	int n, m;
	int i;

	m = -1;
	n = 0;
	for (i = 0 ; i < fwp->flist.n ; ++i) {
		if(fwp->mark[i] != 0) {
			if(n == 0) {
				n = fwp->mark[i];
				m = i;
			} else {
				if(n > fwp->mark[i]) {
					n = fwp->mark[i];
					m = i;
				}
			}
		}
	}
	return m;
}

void fw_chmark(fw_t *fwp, int a)
{
	int n;

	n = a != -1 ? a : fwp->menu.cy + fwp->menu.sy;
	if(strcmp(fwp->findex[n]->fn, "..") == 0) {
		return;
	}
	if(fwp->mark[n] == 0) {
		fwp->menu.mitem[n].mf = TRUE;
		fwp->mark[n] = ++fwp->markmax;
		++fwp->marknum;
	} else {
		fwp->mark[n] = 0;
		fwp->menu.mitem[n].mf = FALSE;
		--fwp->marknum;
		if(fwp->marknum == 0) {
			fwp->markmax = 0;
		}
	}
}

void fw_chmarkall(fw_t *fwp)
{
	int i;

	if(fwp->marknum == 0) {
		for (i = 0 ; i < fwp->flist.n ; ++i) {
			if((fw_getstat(fwp, i)->st_mode & S_IFMT) != S_IFDIR) {
				fw_chmark(fwp, i);
			}
		}
	} else {
		while((i = fw_getmarkfirst(fwp)) != -1) {
			fw_chmark(fwp, i);
		}
	}
}

/* file opration */
static bool select_readonly(fop_t *fop)
{
	int res;

	if(fop->pfm != 0) {
		return fop->pfm == FP_force;
	}
	system_msg(READ_ONLY_MSG);
	res = menu_vselect(NULL, term_sizex() / 2, term_sizey() / 2
					, 4, PROCESS_MSG, NOT_PROCESS_MSG
					, PROCESS_ALL_MSG, NOT_PROCESS_ALL_MSG);
	RefreshMessage();
	switch (res) {
	case 2:
		fop->pfm = FP_force;
	case 0:
		return TRUE;
	case 3:
		fop->pfm = FP_ignore;
	}
	return FALSE;
}

int fw_fop_file(const char *srcpath, const char *fn, struct stat *srcstp, const char *dstpath, fop_t *fop)
{
	char dstfn[LN_path + 1], srcfn[LN_path + 1];
	char tmp[LN_path + 1];
	struct stat dstst;
	int res;

	sprintf(srcfn, "%s%s", srcpath, fn);
	if(dstpath != NULL) {
		sprintf(dstfn, "%s%s", dstpath, fn);
	}
	for (;;) {
		if(dstpath != NULL) {
			if(stat(dstfn, &dstst) == 0 || errno != ENOENT) {
				if(fop->om == 0) {
					fop->om = 1 + menu_vselect(NULL, term_sizex() / 2, term_sizey() / 2, 4
						, OVERWRITE_ALL_MSG, NEWER_DATE_MSG
						, SAME_NAME_NO_COPY_MSG, RENAME_AND_COPY_MSG);
				}

				switch(fop->om) {
				case 0:
					return FR_end;
				case FO_owrite:
					break;
				case FO_update:
					if(dstst.st_mtime < srcstp->st_mtime) {
						break;
					}
					return FR_ok;
				case FO_rename:
					fop->om = 0;
					strcpy(tmp, fn);
					if(GetS(NEW_NAME_MSG, tmp, -1) == ESCAPE) {
						return FALSE;
					}
					sprintf(dstfn, "%s%s", dstpath, tmp);
					continue;
				case FO_none:
					return FR_ok;
				}
				if(access(dstfn, W_OK) < 0 && !select_readonly(fop)) {
					return FR_ok;
				}

				unlink(dstfn);
			}
		}

		res = fop->file_func(srcfn, srcstp, dstfn, fop);
		if(res != FR_err) {
			return res;
		}

		res = menu_vselect(NULL, term_sizex() / 2, term_sizey() / 2, 3, ABORT_MSG, RETRY_MSG, CONTINUE_MSG);
		switch(res) {
		case -1:
		case 0:
			return FR_err;
		case 1:
			continue;
		case 2:
			return FR_ok;
		}
	}
}

int fw_fop_dir(const char *srcpath, const char *fn, struct stat *srcstp, const char *dstpath, fop_t *fop, bool wf)
{
	char dstfn[LN_path + 1], srcfn[LN_path + 1];
	struct stat dstst;
	int res;
	bool f;

	sprintf(srcfn, "%s%s", srcpath, fn);
	if(dstpath != NULL) {
		sprintf(dstfn, "%s%s", dstpath, fn);
		if(stat(dstfn,&dstst) == 0) {
			if((dstst.st_mode &S_IFMT) == S_IFDIR) {
				chmod(dstfn, srcstp->st_mode);
				touchfile(dstfn, srcstp->st_atime, srcstp->st_mtime);
				return FR_ok;
			}
			inkey_wait(DIR_ALREADY_FILE_MSG);
			return FR_err;
		}
	}
	f = TRUE;
	for (;;) {
		res = fop->dir_func(srcfn, srcstp, dstfn, wf, fop);
		if(res != FR_err) {
			return res;
		}
		res = menu_vselect(NULL, term_sizex() / 2, term_sizey() / 2, 3, ABORT_MSG, RETRY_MSG, CONTINUE_MSG);
		switch(res) {
		case -1:
		case 0:
			return FR_err;
		case 1:
			continue;
		case 2:
			return FR_ok;
		}
	}
}

int fw_fop_list(fitem_t **findex, size_t fi_nums, const char *srcpath, const char *dstpath, fop_t *fop)
{
	struct stat *srcstp;
	int res;
	char *p;
	int i;

	fitem_t **fip;
	flist_t fl;
	fitem_t *fitem;
	char srctmp[LN_path + 1], dsttmp[LN_path + 1];

	for (i = 0 ; i < fi_nums ; ++i) {
		srcstp = &findex[i]->stat;

		if((srcstp->st_mode & S_IFMT) != S_IFDIR) {
			res = fw_fop_file(srcpath, findex[i]->fn, srcstp, dstpath, fop);
		} else {
			if(strcmp(findex[i]->fn, ".") == 0 || strcmp(findex[i]->fn, "..") == 0) {
				continue;
			}
			sprintf(srctmp, "%s%s/", srcpath, findex[i]->fn);
			if(dstpath != NULL) {
				sprintf(dsttmp, "%s%s/", dstpath, findex[i]->fn);
			}
			system_msg(srctmp);

			res = fw_fop_dir(srcpath, findex[i]->fn, srcstp, dstpath, fop, TRUE);
			if(res == FR_err) {
				return FR_err;
			}
			if(res == FR_nonrec) {
				continue;
			}

			flist_set(&fl, srctmp);
			fip = findex_get(&fl);
			fw_fop_list(fip, fl.n, srctmp, dstpath == NULL ? NULL : dsttmp, fop);
			free(fip);
			fitem = fl.fitem;
			while(fitem != NULL) {
				fitem = fitem_free(fitem);
			}
			res = fw_fop_dir(srcpath, findex[i]->fn, srcstp, dstpath, fop, FALSE);
		}
		if(res == FR_err || res == FR_end) {
			return res;
		}
	}
	return FR_ok;
}

void fw_fop(fw_t *srcfwp, const char *dstpath, char *title
	, int file_func(const char *, struct stat *, const char *, fop_t *)
	, int dir_func(const char *, struct stat *, const char *, bool, fop_t *))
{
	int mn;
	int n;
	int res;
	fop_t fo;

	fo.title = title;
	fo.file_func = file_func;
	fo.dir_func = dir_func;
	fo.om = 0;
	fo.pfm = 0;

	mn = fw_c.menu.cy + fw_c.menu.sy;
	for (;;) {
		fitem_t	*fip;

		if(srcfwp->marknum == 0) {
			n = -1;
		} else {
			n = fw_getmarkfirst(srcfwp);
		}

		if(n != -1) {
			menu_csrmove(&fw_c.menu, n);
		}
		fip = fw_getfi(srcfwp, n);

		res = fw_fop_list(&fip, 1, srcfwp->path, dstpath, &fo);
		if(n != -1) {
			fw_chmark(srcfwp, n);
		}
		if(res == FR_err || res == FR_end) {
			break;
		}
		if(srcfwp->marknum == 0) {
			break;
		}
	}
	if(n != -1) {
		menu_csrmove(&fw_c.menu, mn);
	}
}


static int cp_proc_dir(const char *src, struct stat *srcstp, const char *dst, bool wf, fop_t *fop)
{
	if(!wf) {
		return FR_ok;
	}
	if(mkdir(dst,0777) != 0) {
		return FR_err;
	}
	chmod(dst, srcstp->st_mode);
	touchfile(dst, srcstp->st_atime, srcstp->st_mtime);
	return FR_ok;
}

#define MAX_cpbuf 4096

static int cp_proc_file(const char *src, struct stat *srcstp, const char *dst, fop_t *fop)
{
	FILE *fpr, *fpw;
	bool ef;

	fpr = fopen(src, "r");
	if(fpr == NULL) {
		return FR_err;
	}
	fpw = fopen(dst, "w");
	if(fpw == NULL) {
		fclose(fpr);
		return FR_err;
	}
	ef = FALSE;
	for (;;) {
		char buf[MAX_cpbuf];
		int n, m;

		n = fread(buf, 1, MAX_cpbuf, fpr);
		if(n == 0) {
			break;
		}
		m = 0;
		while(n - m > 0) {
			m = fwrite(buf + m, 1, n - m, fpw);
		}
		if(m == 0) {
			ef = TRUE;
			break;
		}
	}
	fclose(fpw);
	if(ef || ferror(fpr)) {
		fclose(fpr);
		unlink(dst);
		return FR_err;
	}
	fclose(fpr);
	chmod(dst, srcstp->st_mode);
	touchfile(dst, srcstp->st_atime, srcstp->st_mtime);
	return FR_ok;
}

static int mv_proc_dir(const char *src, struct stat *srcstp, const char *dst, bool wf, fop_t* fop)
{
	if(!wf) {
		rmdir(src);
		return FR_ok;
	}
	if(rename(src, dst) == 0) {
		touchfile(dst, srcstp->st_atime, srcstp->st_mtime);
		return FR_nonrec;
	}
	return cp_proc_dir(src, srcstp, dst, wf, fop);
}

static int mv_proc_file(const char *src, struct stat *srcstp, const char *dst, fop_t *fop)
{
	int res;

	if(rename(src, dst) == 0) {
		return FR_ok;
	}
	res = cp_proc_file(src, srcstp, dst, fop);
	if(res == FR_ok) {
		unlink(src);
	}
	return res;
}

static int rm_proc_file(const char *src, struct stat *srcstp, const char *dst, fop_t *fop)
{
	if(access(src,W_OK) < 0 && !select_readonly(fop)) {
		return FR_ok;
	}
	if(unlink(src) == 0) {
		return FR_ok;
	}
	return FR_err;
}

static int  rm_proc_dir(const char *src, struct stat *srcstp, const char *dst, bool wf, fop_t* fop)
{
	if(wf) {
		return FR_ok;
	}
	if(rmdir(src) == 0) {
		return FR_ok;
	}
	return FR_err;
}

bool fw_cpdest(char *s, fw_t *srcfwp, fw_t *dstfwp, bool move)
{
	char srcpath[LN_path + 1];

	strcpy(srcpath, srcfwp->path);
	reg_path(NULL, srcpath, TRUE);
	if(dstfwp != NULL) {
		strcpy(s, dstfwp->path);
		reg_path(NULL, s, TRUE);
		if(strcmp(srcpath, s) != 0) {
			return TRUE;
		}
	}
	*s = '\0';
	if(GetS(move ? MOVE_TO_MSG : COPY_TO_MSG, s, -1) == ESCAPE) {
		return FALSE;
	}
	reg_path(srcpath, s, FALSE);
	if(strcmp(srcpath, s) == 0) {
		return FALSE;
	}
	return mole_dir(s);
}

void fw_copy(fw_t *srcfwp, fw_t *dstfwp)
{
	char buf[LN_path + 1];

	if(!fw_cpdest(buf, srcfwp, dstfwp, FALSE)) {
		return;
	}

	fw_fop(srcfwp, buf, COPY_MSG, cp_proc_file, cp_proc_dir);
	if(dstfwp != NULL) {
		fw_make(dstfwp);
	}
}

void fw_move(fw_t *srcfwp, fw_t *dstfwp)
{
	char buf[LN_path + 1];

	if(!fw_cpdest(buf, srcfwp, dstfwp, TRUE)) {
		return;
	}

	fw_fop(srcfwp, buf, MOVE_MSG, mv_proc_file, mv_proc_dir);
	fw_make(srcfwp);

	if(dstfwp != NULL) {
		fw_make(dstfwp);
	}
}

void fw_remove(fw_t *srcfwp, fw_t *dstfwp)
{
	if(keysel_ynq(DO_DELETE_MSG) != TRUE) {
		return;
	}
	fw_fop(srcfwp, NULL, DELETE_MSG, rm_proc_file, rm_proc_dir);
	fw_make(srcfwp);
}

void fw_rename(fw_t *fwp)
{
	char buf[LN_path + 1], tmp[LN_path + 1];
	char *fn;

	fn = fw_c.findex[fw_c.menu.cy + fw_c.menu.sy]->fn;
	strcpy(tmp, fn);
	if(GetS(NEW_NAME_MSG, tmp, -1) == ESCAPE) {
		return;
	}

	getcwd(buf, LN_path);
	chdir(fwp->path);
	if(rename(fn, tmp) == 0) {
		dm_set(fwp->path, tmp);
	}
	chdir(buf);
}

void fw_mkdir(fw_t *fwp)
{
	char buf[LN_path + 1], tmp[LN_path + 1];

	*tmp = '\0';
	if(GetS(MKDIR_MSG, tmp, -1) == ESCAPE) {
		return;
	}
	strcpy(buf, fwp->path);
	if(strlen(buf) + strlen(tmp) < LN_path) {
		strcat(buf, tmp);
		mkdir(buf, 0777);
	}
}

void CommandCom(char *sys_buff);

int fw_file(char *fn)
{
	int f = FALSE;
	int res;
	char fname[LN_path + 1];
	char path[LN_path + 1];

	res = menu_vselect(NULL, FILE_MENU_X, MENU_Y, FILE_MENU_COUNT, FILE_COPY_MSG, FILE_MOVE_MSG, FILE_DELETE_MSG, FILE_RENAME_MSG, FILE_MKDIR_MSG, FILE_NEW_MSG, FILE_EXEC_MSG);
	if(res != -1) {
		switch(res) {
		case fileCopy:
			fw_copy(&fw[eff.wa], eff.wn == 2 ? &fw[eff.wa == 0 ? 1 : 0]: NULL);
			break;
		case fileMove:
			fw_move(&fw[eff.wa], eff.wn == 2 ? &fw[eff.wa == 0 ? 1 : 0]: NULL);
			break;
		case fileDelete:
			fw_remove(&fw[eff.wa],NULL);
			break;
		case fileRename:
			fw_rename(&fw[eff.wa]);
			fw_make(&fw[eff.wa]);
			break;
		case fileMkdir:
			fw_mkdir(&fw[eff.wa]);
			fw_make(&fw[eff.wa]);
			break;
		case fileNew:
			fname[0] = '\0';
			if(HisGets(fname, OPEN_MSG, -1) != NULL) {
				fname[LN_path] = '\0';
				if(fname[0] == '/' || fname[0] == '.') {
					strcpy(path, fname);
				} else if(strlen(fw_c.path) + strlen(fname) < LN_path) {
					strcpy(path, fw_c.path);
					strcat(path, fname);
				} else {
					break;
				}
				if(dir_isdir(path)) {
					fwc_chdir(path, TRUE);
					fw_make(&fw[eff.wa]);
				} else {
					strcpy(fn, path);
					f = TRUE;
				}
			}
			break;
		case fileExec:
			fname[0] = '\0';
			if((fw_c.findex[fw_c.menu.cy + fw_c.menu.sy]->stat.st_mode & S_IFMT) != S_IFDIR) {
				strcpy(fname, fw_c.findex[fw_c.menu.cy + fw_c.menu.sy]->fn);
			}
			if(HisGets(fname, GETS_SHELL_MSG, historyShell) != NULL) {
				CommandCom(fname);
			}
			break;
		}
	}
	return f;
}

void dir_set_proc(int a, mitem_t *mip, void *vp)
{
	char *str;

	if((str = get_string_item(itemDir, a)) != NULL) {
		strcpy(mip->str, str);
	} else {
		strcpy(mip->str, "");
	}
}

void fw_dir()
{
	menu_t menu;
	char *str;
	int count, res;

	if((count = get_string_item_count(itemDir)) > 0) {
		menu_iteminit(&menu);
		menu.drp->x = DIR_MENU_X;
		menu.drp->y = MENU_Y;
		menu_itemmake(&menu, dir_set_proc, count, NULL);
		res = menu_select(&menu);
		menu_itemfin(&menu);
		if((str = get_string_item(itemDir, res)) != NULL) {
			if(*str == '0') {
				str = "~";
			} else {
				str += 2;
				while(*str == ' ') {
					str++;
				}
			}
			fwc_chdir(str, TRUE);
			fw_make(&fw[eff.wa]);
		}
	}
}

void fw_sort()
{
	int res;

	res = menu_vselect(NULL, SORT_MENU_X, MENU_Y, SORT_MENU_COUNT, SORT_FILENAME_MSG, SORT_EXT_MSG, SORT_NEW_MSG, SORT_OLD_MSG, SORT_LARGE_MSG, SORT_SMALL_MSG);
	if(res != -1) {
		if(eff.sort != res) {
			eff.sort = res;
			fw_make(&fw[eff.wa]);
		}
	}
}

void mask_set_proc(int a, mitem_t *mip, void *vp)
{
	char *str;

	if((str = get_string_item(itemMask, a)) != NULL) {
		strcpy(mip->str, str);
	} else {
		strcpy(mip->str, "");
	}
}

void fw_mask()
{
	menu_t menu;
	char *str;
	int count, res;

	if((count = get_string_item_count(itemMask)) > 0) {
		menu_iteminit(&menu);
		menu.drp->x = MASK_MENU_X;
		menu.drp->y = MENU_Y;
		menu_itemmake(&menu, mask_set_proc, count, NULL);
		res = menu_select(&menu);
		menu_itemfin(&menu);
		if((str = get_string_item(itemMask, res)) != NULL) {
			int len = strlen(str);
			clear_string_item(itemUse);
			if(len >= 2) {
				str += 2;
				set_ext_item(itemUse, str);
				if(strcmp(str, "*.*")) {
					strcpy(fw_c.mask, str);
				} else {
					fw_c.mask[0] = '\0';
					clear_string_item(itemUse);
				}
			}
			set_path_title();
			fw_make(&fw[eff.wa]);
		}
	}
}

void fw_change_dir()
{
	if(keyf_numarg() > 0) {
		fwc_chdir(keyf_getarg(0), TRUE);
		fw_make(&fw[eff.wa]);
	} else {
		char buf[LN_path + 1];
		buf[0] = '\0';
		if(HisGets(buf, PATH_MASK_MSG, historyMask) != NULL) {
			if(buf[0] != '\0') {
				if(dir_isdir(buf)) {
					if(strcmp(buf, ".")) {
						fwc_chdir(buf, TRUE);
					}
				} else {
					set_ext_item(itemUse, buf);
					strcpy(fw_c.mask, buf);
				}
			} else {
				fw_c.mask[0] = '\0';
				clear_string_item(itemUse);
			}
			set_path_title();
			fw_make(&fw[eff.wa]);
		}
		CrtDrawAll();
	}
}

int eff_check_open()
{
	return eff.open;
}

bool eff_filer(char *fn)
{
	int c;
	char *p;
	bool f;

	eff.open = TRUE;

	fw_make(&fw[0]);
	fw_make(&fw[1]);
	*fn = '\0';
	if(eff.wn == 1) {
		dsp_regrm(fw[1].menu.drp);
	}
	set_path_title();
	for (;;) {
		dsp_allview();
		system_msg("");

		term_csrh();
		c = get_keyf(2);
		switch(c) {
		case -1:
			continue;
		case KF_EffCursorUp:
			menu_csrmove(&fw_c.menu, fw_c.menu.cy + fw_c.menu.sy - 1);
			continue;
		case KF_EffMarkChangeAll:
			fw_chmarkall(&fw[eff.wa]);
			continue;
		case KF_EffMarkChange:
			fw_chmark(&fw[eff.wa], -1);
		case KF_EffCursorDown:
			menu_csrmove(&fw_c.menu, fw_c.menu.cy + fw_c.menu.sy + 1);
			continue;
		case KF_EffPageUp:
			menu_csrmove(&fw_c.menu, fw_c.menu.cy + fw_c.menu.sy - (fw_c.menu.drp->sizey - 1));
			continue;
		case KF_EffPageDown:
			menu_csrmove(&fw_c.menu, fw_c.menu.cy + fw_c.menu.sy + (fw_c.menu.drp->sizey - 1));
			continue;
		case KF_EffRollUp:
			menu_csrmove(&fw_c.menu, fw_c.menu.cy + fw_c.menu.sy - fw_c.menu.drp->sizey / 4);
			continue;
		case KF_EffRollDown:
			menu_csrmove(&fw_c.menu, fw_c.menu.cy + fw_c.menu.sy + fw_c.menu.drp->sizey / 4);
			continue;
		default:
			if(c & KF_normalcode) {
				c &= ~KF_normalcode;
				if(isupper(c)) {
					menu_csrnext(&fw_c.menu, c);
				}
			}
			continue;
		case KF_EffReRead:
			dm_set(fw_c.path, fw_c.findex[fw_c.menu.cy + fw_c.menu.sy]->fn);
			fw_make(&fw[eff.wa]);
			term_cls();
			continue;
		case KF_EffWindowChange:
			if(eff.wn == 2) {
				fw_c.menu.df = TRUE;
				eff.wa = (eff.wa == 0) ? 1 : 0;
				fw_c.menu.df = FALSE;
			}
			continue;
		case KF_EffWindowNumChange:
			if(eff.wn == 1) {
				eff.wn = 2;
				fw[1].menu.df = TRUE;
				dsp_regadd(fw[1].menu.drp);
			} else {
				eff.wa = 0;
				fw_c.menu.df = FALSE;
				eff.wn = 1;
				dsp_regrm(fw[1].menu.drp);
				term_cls();
			}
			continue;
		case KF_EffChangeDir:
			fw_change_dir();
			continue;
		case KF_EffReturn:
			p = fw_c.findex[fw_c.menu.cy + fw_c.menu.sy]->fn;
			if((fw_c.findex[fw_c.menu.cy + fw_c.menu.sy]->stat.st_mode & S_IFMT) == S_IFDIR) {
				fwc_chdir(p, FALSE);
				fw_make(&fw[eff.wa]);
				continue;
			}
			dm_set(fw_c.path, p);
			sprintf(fn, "%s%s", fw_c.path, p);
			f = TRUE;
			break;
		case KF_EffEscape:
			dm_set(fw_c.path, fw_c.findex[fw_c.menu.cy + fw_c.menu.sy]->fn);
			f = FALSE;
			break;
		case KF_EffFileMenu:
			if(fw_file(fn)) {
				break;
			}
			continue;
		case KF_EffDirMenu:
			fw_dir();
			continue;
		case KF_EffSortMenu:
			fw_sort();
			continue;
		case KF_EffMaskMenu:
			fw_mask();
			continue;
		case KF_EffMaskClear:
			clear_string_item(itemUse);
			fw_c.mask[0] = '\0';
			set_path_title();
			fw_make(&fw[eff.wa]);
			continue;
		case KF_EffRedraw:
			op_misc_redraw();
			fw_make(&fw[0]);
			fw_make(&fw[1]);
			if(eff.wn == 1) {
				fw[1].menu.df = FALSE;
				dsp_regrm(fw[1].menu.drp);
			}
			continue;
		}
		break;
	}
	system_msg("");
	fw_free(&fw[0]);
	fw_free(&fw[1]);
	dsp_regrm(fw[0].menu.drp);
	dsp_regrm(fw[1].menu.drp);

	eff.open = FALSE;
	return f;
}

bool need_filer(const char* pszFilename)
{
	if(*pszFilename == '\0') {
		return TRUE;
	}
	if(strchr(pszFilename, '*') != NULL || strchr(pszFilename, '?') != NULL) {
		return TRUE ;
	}
	return dir_isdir(pszFilename);
}

