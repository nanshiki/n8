/*
 *    regular expression module.
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

#include "config.h"

#include <stdio.h>
#define __USE_GNU
#include <string.h>
#include <sys/types.h>

#include "generic.h"
#include "regexp.h"

#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#define	strcasestr	StrStrIA
#pragma comment(lib, "shlwapi.lib")

#define REG_POSIX_ENCODING_UTF8      3

typedef int (*f_regcomp)(regex_t *, const char *, int);
typedef int (*f_regexec)(regex_t *,const char *, size_t, regm_t *, int);
typedef void (*f_regfree)(regex_t *);
typedef size_t (*f_regerror)(int, const regex_t *reg, char *, size_t);
typedef void (*f_reg_set_encoding)(int enc);

static int regexp_flag;
f_regcomp regcomp;
f_regexec regexec;
f_regfree regfree;
f_regerror regerror;
f_reg_set_encoding reg_set_encoding;

int regexp_init()
{
	HMODULE hm;

	if((hm = LoadLibraryA("onigmo.dll")) != NULL) {
		regcomp = (f_regcomp)GetProcAddress(hm, "regcomp");
		regexec = (f_regexec)GetProcAddress(hm, "regexec");
		regfree = (f_regfree)GetProcAddress(hm, "regfree");
		regerror = (f_regerror)GetProcAddress(hm, "regerror");
		reg_set_encoding = (f_reg_set_encoding)GetProcAddress(hm, "reg_set_encoding");
		if(regcomp != NULL && regexec != NULL && regfree != NULL && regerror != NULL && reg_set_encoding != NULL) {
			reg_set_encoding(REG_POSIX_ENCODING_UTF8);
			regexp_flag = TRUE;
		}
	}
	return regexp_flag;
}

int get_regexp_enable()
{
	return regexp_flag;
}
#endif

/* search */

int regexp_exec(const char *s, int x, const char *t, regm_t *rmp, int nocasef)
{
	static char regexp_str[2048 + 1] = {""};
	static int regexp_casef;
	static regex_t regexp_exp;

	int a;

	if(strcmp(t, regexp_str) != 0 || nocasef != regexp_casef) {
		if(*regexp_str != '\0') {
			regfree(&regexp_exp);
		}
		a = nocasef ? REG_EXTENDED | REG_ICASE : REG_EXTENDED;
		if(regcomp(&regexp_exp, t, a) != 0) {
			return FALSE;
		}
		strcpy(regexp_str, t);
		regexp_casef = nocasef;
	}
	a = x == 0 ? 0 : REG_NOTBOL;
	if(regexec(&regexp_exp, s + x, 1, rmp, a) != 0 || rmp->rm_so == -1) {
		return FALSE;
	}
	rmp->rm_so += x;
	rmp->rm_eo += x;
	return TRUE;
}

int no_regexp_exec(const char *s, int x, const char *t, regm_t *rmp, int nocasef)
{
	const char *p;

	if(nocasef) {
		p = strcasestr(s + x, t);
	} else {
		p = strstr(s + x, t);
	}
	if(p == NULL) {
		return FALSE;
	}
	rmp->rm_so = (off_t)(p - s);
	rmp->rm_eo = (off_t)(rmp->rm_so + strlen(t));
	return TRUE;
}

int regexp_seeknext(const char *s, const char *t, int x, regm_t *rmp, int regf, int nocasef)
{
#ifdef _WIN32
	if(regf && regexp_flag) {
#else
	if(regf) {
#endif
		return *t != '\0' && regexp_exec(s, x, t, rmp, nocasef);
	}
	return *t != '\0' && no_regexp_exec(s, x, t, rmp, nocasef);
}

int regexp_seekprev(const char *s, const char *t, int x, regm_t *rmp, int regf, int nocasef)
{
	regm_t rm;

	if(x < 0) {
		return FALSE;
	}
	if(x > strlen(s)) {
		x = (int)strlen(s);
	}
	rmp->rm_so = -1;
	rm.rm_so = 0;
	for(;;) {
#ifdef _WIN32
		if(regf && regexp_flag) {
#else
		if(regf) {
#endif
			if(!regexp_exec(s, rm.rm_so, t, &rm, nocasef)) {
				break;
			}
		} else {
			if(!no_regexp_exec(s, rm.rm_so, t, &rm, nocasef)) {
				break;
			}
		}
		if(rm.rm_so > x) {
			break;
		}
		rmp->rm_so = rm.rm_so;
		rmp->rm_eo = rm.rm_eo;
		if(rm.rm_so == x) {
			return TRUE;
		}
		++rm.rm_so;
	}
	return rmp->rm_so != -1;
}

