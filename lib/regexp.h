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

#ifdef _WIN32
#include <sys/types.h>
#define REG_ICASE          (1<<0)
#define REG_NEWLINE        (1<<1)
#define REG_NOTBOL         (1<<2)
#define REG_NOTEOL         (1<<3)
#define REG_EXTENDED       (1<<4)
#define REG_NOSUB          (1<<5)

typedef struct {
	void* onig;
	size_t re_nsub;
	int comp_options;
} regex_t;

typedef struct
{
	off_t rm_so;
	off_t rm_eo;
} regm_t;
int regexp_init();
int get_regexp_enable();
#else
 #ifdef HAVE_REGEX_H
  #include	<regex.h>
  typedef regmatch_t regm_t;
 #else
  #include <sys/types.h>
  typedef struct
  {
	off_t rm_so;
	off_t rm_eo;
  } regm_t;
 #endif
#endif

extern int regexp_seeknext(const char *s, const char *t, int x, regm_t *rmp, int regf, int casef);
extern int regexp_seekprev(const char *s, const char *t, int x, regm_t *rmp, int regf, int casef);
