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

#ifndef __ELE_GENERIC_H__
#define __ELE_GENERIC_H__

typedef int bool;

#ifndef FALSE
#define FALSE	0
#define TRUE	!FALSE
#endif

#ifndef max
#define max(x, y)	(x>y?x:y)
#define min(x, y)	(x<y?x:y)
#endif

extern void *mem_alloc(size_t n);
extern void *mem_realloc(void *p, size_t n);

extern void *mem_alloca(size_t n);
extern void mem_alloca_gc();

extern char *mem_strdup(const char *s);
extern char *mem_strdupa(const char *s);

extern void reg_pf(const char *cp, char *s, bool f);
extern void reg_path(const char *cp, char *s, bool f);
extern void cut_pf(const char *s, char *path, char *file);

extern char *dir_pext(const char *s);
extern char **dir_glob(const char *s, bool f_dotfile);
extern bool dir_isdir(const char *s);



extern	void	error(const char *s);

extern	void	report_set(FILE *fp);
extern	void	report_printf(const char *fmt, ...);
extern	void	report_puts(const char *s);

#ifdef	NDEBUG
#	define	report_assert(exp)
#else
#	define	report_assert(exp)\
	if(!exp)\
		report_printf(__FILE__ ":%d: " #exp " falied.\n", __LINE__)
#endif



#endif
