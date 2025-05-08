/*
 *    Terminal Module.
 *
 *    based on NxEdit2.04     :: term.c
 *             FD clone 1.03g :: term.c
 *             ne 3.00pre17   :: term.c
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


#ifndef	__TERM_H_
#define	__TERM_H_


void term_init();
void term_start();
void term_stop();
void term_reinit();

char *term_getent(const char *s_id, const char *s_def);


void term_kflush();		/* 入力フラッシュ */

int term_kbhit(unsigned long usec);
int term_getch();


void term_scroll(int n);
void term_bell();
void term_csrn();
void term_csrh();

void term_cls();
void term_clrtoe(int ac);

void term_locate(int y, int x);
void term_movex(int x);

int term_puts(const char *s, const char *ac);
void term_putch(int c);
void term_printf(const char *fmt,...);

typedef unsigned short color_t;

#define AC_normal	0

#define AC_black	0+8
#define AC_red		1+8
#define AC_green	2+8
#define AC_yellow	3+8
#define AC_blue		4+8
#define AC_magenta	5+8
#define AC_cyan		6+8
#define AC_white	7+8

#define AC_color(cl)	((cl) & 0x0ff)
#define AC_attrib(cl)	((cl) & 0xf00)

#define AC_reverse	0x100
#define AC_under	0x200
#define AC_bold		0x400

#define AC_ignore	255

#define	AM_FIX1		0x00
#define	AM_FIX2		0x01
#define	AM_EMOJI2	0x02
#define	AM_MASK		0xff

#define	AM_TERATERM	0x100

void term_color(color_t c);
void term_color_normal();

void term_color_reverse();
void term_color_underline();
void term_color_bold();

color_t term_cftocol(const char *s);


void term_color_enable(int color256);
void term_color_disable();


/* term_inkey */
void term_escset(int n, const char *e, const char *d);
int term_inkey();
int keysdef_getcode(const char *s, int k[], int num);
void keys_set(const char *k, const char *e, const char *d);

void term_report();
void term_keyreport();
int term_sizex();
int term_sizey();
void term_csr_flush();
void term_redraw_line();
void term_redraw_box(int sx, int sy, int width, int height);
void term_set_ambiguous(int mode);
int term_utf8_half_char(const char *p);

#endif	/* __TERM_H_ */
