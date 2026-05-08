/*
 *    generic module.
 *
 * Copyright (c) 1998, 1999, 2000 SASAKI Shunsuke.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification, immediately at the beginning of the file.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Where this Software is combined with software released under the terms of 
 * the GNU Public License ("GPL") and the terms of the GPL would require the 
 * combined work to also be released under the terms of the GPL, the terms
 * and conditions of this License will apply in addition to those of the
 * GPL with the exception of any terms or conditions of this License that
 * conflict with, or are expressly prohibited by, the GPL.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

#if	HAVE_DIRENT_H
#ifndef _WIN32
#	include	<dirent.h>
#endif
#	define	NAMLEN(dirent)	strlen((dirent)->d_name)
#else
#	define	dirent direct
#	define	NAMLEN(dirent)	(dirent)->d_namlen
#	if	HAVE_SYS_NDIR_H
#		include	<sys/ndir.h>
#	endif
#	if	HAVE_SYS_DIR_H
#		include	<sys/dir.h>
#	endif
#	if	HAVE_NDIR_H
#		include	<ndir.h>
#	endif
#endif

#ifdef	HAVE_STDARG_H
#	include	<stdarg.h>
#else
#	ifdef	HAVE_VARARGS_H
#		include	<varargs.h>
#	endif
#endif


#include "generic.h"
#include "misc.h"


void *mem_alloc(size_t n)
{
	void *p;

	p = malloc(n);
	if(p == NULL) {
		error("malloc");
	}
	return p;
}

void *mem_realloc(void *p, size_t n)
{
	p = realloc(p, n);
	if(p == NULL) {
		error("realloc");
	}
	return p;
}

static void **p_alloca = NULL;
static int  max_alloca = 0, n_alloca = 0;

void *mem_alloca(size_t n)
{
	if(n_alloca >= max_alloca) {
		 p_alloca = (void **)realloc(p_alloca, 64);
		 max_alloca += 64;
	}
	return p_alloca[n_alloca++] = mem_alloc(n);
}

void mem_alloca_gc()
{
	while(n_alloca >= 0) {
		free(p_alloca[--n_alloca]);
	}
}

char *mem_strdup(const char *s)
{
	size_t n;

	n = strlen(s) + 1;
	return (char *)memcpy(mem_alloc(n), s, n);
}

char *mem_strdupa(const char *s)
{
	size_t n;

	n = strlen(s) + 1;
	return (char *)memcpy(mem_alloca(n), s, n);
}

#define LN_path 2048

char *dir_pext(const char *s)
{
	const char *p;

	if(*s == '.') {
		++s;
		if(*s == '.') {
			return NULL;
		}
	}
	p = s + strlen(s);
	while(p > s) {
		if(*p == '.') {
			return (char *)p + 1;
		}
		--p;
	}
	return NULL;
}

void cut_pf(const char *s, char *path, char *file)
{
	const char *p;

	p = strrchr(s, '/');
	if(p == NULL) {
		if(path != NULL) {
			*path = '\0';
		}
		if(file != NULL) {
			strcpy(file, s);
		}
		return;
	}
	if(path != NULL) {
		if(p == s) {
			strcpy(path, "/");
		} else {
			strncpy(path, s, p - s);
			path[p - s] = '\0';
		}
	}
	if(file != NULL) {
		strcpy(file, p + 1);
	}
}

void reg_pf(const char *cp, char *s, int f)
{
	char path[LN_path + 1];
	char fn[LN_path + 1];

	cut_pf(s, path, fn);
	reg_path(cp, path, f);
	if(path[0] == '\0') {
		size_t length;
		getcwd(path, LN_path);
		if((length = strlen(path)) > 0) {
			if(path[length - 1] != '/' && length < LN_path) {
				path[length] = '/'; path[length + 1] = '\0';
			}
		}
	}
	sprintf(s, "%s%s", path, fn);
}

#ifndef _WIN32
void complete_tilde(char *s, const char *t)
{
	const char *p;
	int f;
	int n;
	char buf[LN_path + 1];

	if(t[1] == '\0' || t[1] == '/') {
		p = getenv("HOME");
		strcpy(s, p != NULL ? p : "");

		p = strchr(t, '/');
	} else {
		f = FALSE;
		p = strchr(t, '/');
		if(p == NULL) {
			strcpy(buf, t + 1);
		} else {
			strcpy(buf, t + 1);
			memcpy(buf, t + 1, p - t - 1);
			buf[p - t - 1]='\0';
		}
		setpwent();
		for(;;) {
			struct passwd *pwd;

			pwd = getpwent();
			if(pwd == NULL) {
				break;
			}
			if(strcmp(buf, pwd->pw_name) == 0) {
				strcpy(s, pwd->pw_dir);
				f = TRUE;
				break;
			}
		}
		endpwent();

		if(!f) {
			strcpy(s, t);
			p = NULL;
		}
	}
	strcat(s, "/");
	if(p != NULL) {
		strcat(s, p + 1);
		strcat(s, "/");
	}
}
#endif

#define MAX_pt 1024
void reg_path(const char *cp, char *s, int f)
{
	char path[LN_path + 1];
	char *sp, *p, *pt[MAX_pt], *q;
	int pn;
	int i, j, n;
#ifdef _WIN32
	int flag;

	if(*s == '\0') return;
	flag = (*s == '/' && *(s + 1) == '/');
	q = s;
	if(*(q + 1) != ':' && !flag) {
#else
	if(*s != '~') {
		q = s;
	} else {
		complete_tilde(path, s);
		q = path;
	}
	if(*q != '/') {
#endif
		char cbuf[LN_path + 1];
		if(cp == NULL) {
			getcwd(cbuf, LN_path);
			cp = cbuf;
		}
		if(q == s) {
			strcpy(path, cp);
			if(strlen(path) + strlen(s) + 1 < LN_path) {
				strcat(path, "/");
				strcat(path, s);
			}
			q = path;
		} else {
			sprintf(s, "%s/%s", cp, path);
			q = s;
		}
	}
	pn = 0;
	sp = q;
	p = strsep(&sp, "/");
	while(p != NULL) {
		if(*p != '\0') {
			pt[pn] = (char *)mem_alloc(strlen(p) + 1);
			strcpy(pt[pn], p);
			++pn;
		}
		p = strsep(&sp, "/");
	}
	for(i = 0 ; i < pn ; ++i) {
		if(strcmp(pt[i], "..") == 0) {
			*pt[i] = '\0';
			j = i;
			while(j >= 0) {
				if(*pt[j] != '\0') {
					*pt[j] = '\0';
					break;
				}
				--j;
			}
		}
	}
	n = 0;
	for(i = 0 ; i < pn ; ++i) {
		if(*pt[i] != '\0') {
#ifdef _WIN32
			if(i == 0) {
				if(flag) {
					n += sprintf(s + n, "//%s", pt[i]);
				} else {
					n += sprintf(s + n, "%s", pt[i]);
				}
			} else {
				n += sprintf(s + n, "/%s", pt[i]);
			}
#else
			n += sprintf(s + n, "/%s", pt[i]);
#endif
		}
		free(pt[i]);
	}
	strcpy(s + n, "/");
#ifndef _WIN32
	if(f) {
		getcwd(path, LN_path);

		chdir(s);
		getcwd(s, LN_path);
		if(strcmp(s, "/") != 0) {
			strcat(s, "/");
		}
		chdir(path);
	}
#endif
}

int dir_isdir(const char *s)
{
	struct stat st;
	return !stat(s, &st) && st.st_mode & S_IFDIR;
}

#define N_dir 256

char **dir_glob(const char *s, int f_dotfile)
{
	char **p = NULL;
	int n = 0;
#ifdef _WIN32
	HANDLE h;
	WIN32_FIND_DATA fd;
	wchar_t wpath[LN_path];
	char name[LN_path];
	int unc_root = check_unc_root(s);

	utf8_to_wchar(s, wpath, LN_path);
	lstrcat(wpath, L"/*.*");
	h = FindFirstFile(wpath, &fd);
	if(h == INVALID_HANDLE_VALUE) {
#else
	DIR *p_dir;
	struct dirent *p_de;

	p_dir = opendir(s);
	if(p_dir == NULL) {
#endif
		 p = mem_alloc(sizeof(char **) * 2);
		 p[0] = mem_strdup("..");
		 p[1] = NULL;
		 return p;
	}

#ifdef _WIN32
	do {
		if(fd.cFileName[0] == L'.' && (!f_dotfile || fd.cFileName[1] == L'\0')) {
			continue;
		}
		if(unc_root && fd.cFileName[0] == L'.' && fd.cFileName[1] == L'.' && fd.cFileName[2] == L'\0') {
			continue;
		}
		if(n % N_dir == 0) {
			p = (char **)mem_realloc(p, sizeof(char **) * (n + N_dir));
		}
		wchar_to_utf8(fd.cFileName, name, LN_path);
		p[n++] = mem_strdup(name);
	} while(FindNextFile(h, &fd));
	FindClose(h);
#else
	for(;;) {
		p_de = readdir(p_dir);
		if(p_de == NULL) {
			break;
		}
		if(*p_de->d_name == '.' && (!f_dotfile || p_de->d_name[1] == '\0')) {
			continue;
		}
		if(n % N_dir == 0) {
			p = (char **)mem_realloc(p, sizeof(char **) * (n + N_dir));
		}
		p[n++] = mem_strdup(p_de->d_name);
	}
	closedir(p_dir);
#endif
	p[n] = NULL;
	return p;
}

void error(const char *s)
{
	fprintf(stderr, "%s\n", s);
	exit(EXIT_FAILURE);
}

static FILE *report_fp = NULL;

void report_puts(const char *s)
{
	if(report_fp == NULL) {
		return;
	}
	fputs(s, report_fp);
}

void report_printf(const char *fmt, ...)
{
	va_list args;

	if(report_fp == NULL) {
		return;
	}
	va_start(args, fmt);
	vfprintf(report_fp, fmt, args);
	va_end(args);
}

void report_set(FILE *fp)
{
	report_fp = fp;
}
