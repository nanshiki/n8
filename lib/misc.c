/*
 *    misc module.
 *
 * Copyright (c) 1999, 2000 SASAKI Shunsuke.
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
#include	"config.h"

#include	<stdio.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#ifdef _WIN32
#include	<sys/utime.h>
#include	<direct.h>
int utf8_to_wchar(const char *src, wchar_t *dst, int len);
int check_unc_root(const char *path);
#else
#include	<utime.h>
#include	<unistd.h>
#endif
#include	<fcntl.h>
#include	"generic.h"

#define LN_path 2048

/* misc */
// UTF-8
void strjncpy(char *dst, const char *src, size_t ln)
{
	int i, ch;

	if(strlen(src) <= ln) {
		strcpy(dst, src);
		return;
	}
	i = 0;
	while(i < ln) {
		ch = *src & 0xf0;
		*dst = *src++;
		if(ch == 0xc0 || ch == 0xd0) {
			if(i + 2 > ln) {
				break;
			}
			i++;
			*++dst = *src++;
		} else if(ch == 0xe0) {
			if(i + 3 > ln) {
				break;
			}
			i += 2;
			*++dst = *src++;
			*++dst = *src++;
		} else if(ch == 0xf0) {
			if(i + 4 > ln) {
				break;
			}
			i += 3;
			*++dst = *src++;
			*++dst = *src++;
			*++dst = *src++;
		}
		dst++;
		i++;
	}
	*dst = '\0';
}

int touchfile(const char *path, time_t atime, time_t mtime)
{
#ifdef _WIN32
	struct _utimbuf times;
	wchar_t wpath[LN_path + 1];
#else
	struct utimbuf times;
#endif
	times.actime = atime;
	times.modtime = mtime;
#ifdef _WIN32
	utf8_to_wchar(path, wpath, LN_path);
	return _wutime(wpath, &times);
#else
	return utime(path, &times);
#endif
}

int mole_dir(const char *s)
{
	const char *p;
	char buf[LN_path + 1];	// !! buf size
#ifdef _WIN32
	wchar_t wbuf[LN_path + 1];
	struct _stat st;
#else
	struct stat st;
#endif

	p = s;
	if(*p == '/') {
		++p;
	}
	for(;;) {
		if(*p == '\0' || p == NULL) {
			return TRUE;
		}
		p = strchr(p,'/');
		if(p == NULL) {
			strcpy(buf, s);
		} else {
			memcpy(buf, s, p - s);
			buf[p - s] = '\0';
			++p;
		}
#ifdef _WIN32
		if(check_unc_root(buf) || strlen(buf) <= 2) {
			continue;
		}
		utf8_to_wchar(buf, wbuf, LN_path);
		if(_wstat(wbuf, &st) == 0) {
#else
		if(stat(buf, &st) == 0) {
#endif
			if((st.st_mode & S_IFMT) == S_IFDIR) {
				continue;
			}
			return FALSE;
		}
#ifdef _WIN32
		if(_wmkdir(wbuf) < 0) {
#else
		if(mkdir(buf, 0777) < 0) {
#endif
			return FALSE;
		}
	}
	return TRUE;
}

/*
  strstr, mkdir などがない環境はここにそのコードを追加
*/

#ifndef	HAVE_FLOCK

#ifndef	F_WRLCK
int flock(int fd, int op)
{
	return 0;
}
#else

#ifndef LOCK_SH
#define LOCK_SH	1
#define LOCK_EX	2
#define LOCK_NB	4
#define LOCK_UN	8
#endif

int flock(int fd, int op)
{
	struct flock fl;
	int cmd;

	fl.l_start = 0;
	fl.l_len = 0;
	fl.l_whence = SEEK_SET;

	if(op & LOCK_EX) {
		fl.l_type = F_WRLCK;
	}
	if(op & LOCK_SH) {
		fl.l_type = F_RDLCK;
	}
	if(op & LOCK_UN) {
		fl.l_type = F_UNLCK;
	}
	cmd = (op & LOCK_NB) ? F_SETLK : F_SETLKW;
	return fcntl(fd, cmd, &fl);
}

#endif

#endif

#if !defined(HAVE_STRSEP) || defined(_WIN32)
char *strsep(char **stringp, const char *delim)
{
	char *p;

	p = *stringp;
	if(p == NULL) {
		return p;
	}
	while(**stringp !='\0') {
		if(strchr(delim, **stringp) != NULL) {
			**stringp = '\0';
			++*stringp;
			return p;
		}
		++*stringp;
	}
	*stringp = NULL;
	return p;
}

#endif

#if !defined(HAVE_REALPATH) || defined(_WIN32)
void realpath(const char *cp, char *s)
{
	strcpy(s, cp);
	reg_pf(NULL, s, TRUE);
}
#endif

#ifdef _WIN32
#include <windows.h>
#include <io.h>

int access_win(const char *filename, int mode)
{
	wchar_t wfilename[LN_path + 1];
	utf8_to_wchar(filename, wfilename, LN_path);
	return _waccess(wfilename, mode);
}

FILE *fopen_win(const char *filename, char *mode)
{
	wchar_t wfilename[LN_path + 1], wmode[4 + 1];
	utf8_to_wchar(filename, wfilename, LN_path);
	utf8_to_wchar(mode, wmode, 4);
	return _wfopen(wfilename, wmode);
}

int chdir_win(const char *path)
{
	wchar_t wpath[LN_path + 1];
	utf8_to_wchar(path, wpath, LN_path);
	return _wchdir(wpath);
}

int unlink_win(const char *path)
{
	wchar_t wpath[LN_path + 1];
	utf8_to_wchar(path, wpath, LN_path);
	return _wunlink(wpath);
}

int mkdir_win(const char *path, int mode)
{
	wchar_t wpath[LN_path + 1];
	utf8_to_wchar(path, wpath, LN_path);
	return _wmkdir(wpath);
}

int rmdir_win(const char *path)
{
	wchar_t wpath[LN_path + 1];
	utf8_to_wchar(path, wpath, LN_path);
	return _wrmdir(wpath);
}

void change_dir_char(char *dir)
{
	if(*(dir + 1) == ':') {
		*dir = toupper(*dir);
	}
	while(*dir != '\0') {
		if(*dir == '\\') {
			*dir = '/';
		}
		dir++;
	}
}

int wchar_to_utf8(const wchar_t *src, char *dst, int len)
{
	return WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, len, NULL, NULL);
}

int utf8_to_wchar(const char *src, wchar_t *dst, int len)
{
	return MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, len);
}

void getcwd_win(char *path, int len)
{
	wchar_t *wpath;

	if(wpath = (wchar_t *)malloc(len * sizeof(wchar_t))) {
		GetCurrentDirectory(len, wpath);
		wchar_to_utf8(wpath, path, len);
		change_dir_char(path);
		free(wpath);
	}
}

void get_home_dir(char *path, int len)
{
	wchar_t *wpath;

	if(wpath = (wchar_t *)malloc(len * sizeof(wchar_t))) {
		ExpandEnvironmentStrings(L"%USERPROFILE%", wpath, len);
		wchar_to_utf8(wpath, path, len);
		change_dir_char(path);
		free(wpath);
	}
}

void get_exe_dir(char *path, int len)
{
	wchar_t *wpath;

	if(wpath = (wchar_t *)malloc(len * sizeof(wchar_t))) {
		wchar_t *wpt;
		GetModuleFileName(NULL, wpath, len);
		if(wpt = wcsrchr(wpath, L'\\')) {
			*wpt = L'\0';
		}
		wchar_to_utf8(wpath, path, len);
		change_dir_char(path);
		free(wpath);
	}
}

static char *current_dir[26];
void set_current_dir(int drive, char *path)
{
	if(drive >= 0 && drive < 26) {
		if(current_dir[drive] != NULL) {
			free(current_dir[drive]);
		}
		change_dir_char(path);
		current_dir[drive] = _strdup(path);
	}
}

void get_current_dir(int drive, char *path)
{
	if(drive >= 0 && drive < 26) {
		if(current_dir[drive] != NULL) {
			strcpy(path, current_dir[drive]);
		}
	}
}

int check_unc_root(const char *path)
{
	int count = 0;

	if(*path != '/' || *(path + 1) != '/') {
		return FALSE;
	}
	path += 2;
	while(*path != '\0') {
		if(*path == '/' && *(path + 1) != '\0') {
			count++;
		}
		path++;
	}
	return (count < 2);
}

#endif

#ifdef _WIN32
#include <stdarg.h>
void Trace(const char *form , ...)
{
	va_list	ap;
	static char work[1000];

	va_start(ap, form);
	vsprintf(work, form, ap);
	va_end(ap);
	OutputDebugStringA(work);
}
#endif
