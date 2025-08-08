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

#include "config.h"
#include "eaw.h"
#include "emoji.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/ioctl.h>

#ifdef HAVE_UNISTD_H
 #include <unistd.h>
#endif

#include <sys/types.h>
#include <errno.h>

#ifdef HAVE_STDARG_H
 #include <stdarg.h>
#else
 #ifdef HAVE_VARARGS_H
  #include <varargs.h>
 #endif
#endif

#ifdef HAVE_FCNTL_H
 #include <fcntl.h>
#endif

#ifdef HAVE_SELECT
 #ifdef HAVE_SYS_TIME_H
  #include <sys/time.h>
 #endif
#else
 #ifdef HAVE_SYS_POLL_H
  #include <sys/poll.h>
 #endif
#endif

#ifdef HAVE_TERMCAP_H
 #include <termcap.h>
#endif

char PC = '\0';
char *BC = NULL;
char *UP = NULL;

#ifdef HAVE_TERMIOS_H
 #include <termios.h>
 typedef struct termios term_ioctl_t;
 #define term_getattr(t)	tcgetattr(fileno(term.fp_tty), t)
 #define term_setattr(t)	tcsetattr(fileno(term.fp_tty), TCSAFLUSH, t)
 #define term_getspeed()	cfgetospeed(&term.tty)
#else
 #ifdef HAVE_TERMIO_H
  #include <termio.h>
  typedef struct termio term_ioctl_t;
  #define term_getattr(t)	ioctl(fileno(term.fp_tty), TCGETA, t)
  #define term_setattr(t)	ioctl(fileno(term.fp_tty), TCSETAF, t)
  #define term_getspeed()	(term.tty.c_cflag & CBAUD)
 #endif
#error	"SGTTY 等を定義して下さい。"
#endif

#ifndef STDIN_FILENO
 #define STDIN_FILENO	0
#endif

#ifndef IEXTEN
 #define IEXTEN		0
#endif

#include "generic.h"
#include "term.h"

#define TM_none 		0x00
#define TM_keypad		0x01
#define TM_init			0x02
#define TM_tinit		0x04
#define TM_ansicolor	0x08
#define TM_color256		0x10

#define LN_dispbuf	2048
#define TERMCAPSIZE	2048
#define TTYNAME "/dev/tty"

static void term_locate_flush();
static void term_color_flush();
static void term_tputs(const char *str);

static void term_all_flush();

static int exit_failed = FALSE;
static char term_buf[TERMCAPSIZE + 1];

#ifndef TIOCSTI
static unsigned char ungetbuf[16];
static int ungetnum = 0;
#endif

extern char *tparm(const char *str, ...);

typedef struct
{
	int mode;

	/* control TTY */
	char *name;
	FILE *fp_tty;
	term_ioctl_t tty;

	/* control screen */
	char *t_init;
	char *t_keypad;
	char *t_nokeypad;
	char *t_end;
	char *cn_locate;

	char *t_clear;
	char *t_normal;
	char *t_reverse;
	char *t_underline;
	char *t_bold;
	char *l_insert;
	char *l_delete;
	char *ln_insert;
	char *ln_delete;
	char *l_clear;
	char *end_underline;

	char *t_hidecursor;
	char *t_normalcursor;

	char *t_vbell;


	int sizex, sizey;

	int md_cursor, md_cursor0;
	int x, y;		/* cursor location */
	int x0, y0;		/* latest cursor location */
	color_t cl;		/* now color */
	color_t cl0;	/* latest color */

	unsigned long **scr, **scr0;
	color_t **attr, **attr0;

	int *tq;
	bool f_cls;
	int ambiguous;
} term_t;

static term_t term;

#define CS_ignore	0
#define CS_normal	1
#define CS_hide		2

#define SCR_ignore	(unsigned long)-1

FILE *tty_fp()
{
	return term.fp_tty;
}

static void term_flush()
{
	fflush(term.fp_tty);
}

int term_sizex()
{
	return term.sizex;
}

int term_sizey()
{
	return term.sizey;
}

void term_getwsize()
{
	int x, y;

	x = y = -1;

#ifdef TIOCGWINSZ
	{
		struct winsize ws;

		if(ioctl(fileno(term.fp_tty), TIOCGWINSZ, &ws) >= 0) {
			x = ws.ws_col;
			y = ws.ws_row;
		}
	}
#endif
#ifdef WIOCGETD
	if(x < 0 && y < 0) {
		struct uwdate wd;

		if(ioctl(fileno(term.fp_tty), WIOCGETD, &ws) >= 0) {
			x = ws.uw_width / ws.uw_hs;
			y = ws.uw_height / ws.uw_vs;
		}
	}
#endif
	if(x > 0 && y > 0) {
		term.sizex = x;
		term.sizey = y;
	}
}

char *term_getent(const char *s_id, const char *s_def)
{
	char buf[TERMCAPSIZE + 1], *p;
	const char *cp;

	p = buf;
	cp = tgetstr((char *)s_id, &p);
	if(cp == NULL || *cp == '\0') {
		cp = s_def;
	}
	return mem_strdup(cp);
}

#if 0
static void term_tparam(char *s, const char *fmt, int arg1, int arg2)
{
	while(*fmt != '\0') {
		if(*fmt != '%') {
			*s++ = *fmt++;
			continue;
		}
		++fmt;
		switch(*fmt) {
		case 'd':
			sprintf(s, "%d", arg1);
			s += strlen(s);
			arg1 = arg2;
			arg2 = 0;
			break;
		case 'i':
			++arg1;
			++arg2;
			break;
		case 'p':
			++fmt;
			break;
		case '%':
			*s++ = '%';
		}
		if(*fmt != '\0') {
			++fmt;
		}
	}
	*s = '\0';
}
#endif

void term_queue_clear()
{
	int i;

	for(i = 0 ; i < term.sizey ; ++i) {
		term.tq[i] = 0;
	}
}

void term_scroll(int n)
{
	term.tq[term.y] += n;
}

static void term_scr_clr(unsigned long *scr, color_t *attr, int x, int ac)
{
	while(x < term.sizex) {
		scr[x] = ' ';
		attr[x] = ac;
		x++;
	}
}

void term_cls()
{
	int i;

	term_queue_clear();
	term.f_cls = TRUE;
	for(i = 0 ; i < term.sizey ; ++i) {
		term_scr_clr(term.scr[i], term.attr[i], 0, AC_normal);
	}
}

static void term_scr_init()
{
	int i;

	term.scr = mem_alloc(term.sizey * sizeof (void *));
	term.scr0 = mem_alloc(term.sizey * sizeof (void *));
	term.attr = mem_alloc(term.sizey * sizeof (void *));
	term.attr0 = mem_alloc(term.sizey * sizeof (void *));
	for(i = 0 ; i < term.sizey ; ++i) {
		term.scr[i] = mem_alloc(term.sizex * sizeof(unsigned long));
		term.scr0[i] = mem_alloc(term.sizex * sizeof(unsigned long));
		term.attr[i] = mem_alloc(term.sizex * sizeof(color_t));
		term.attr0[i] = mem_alloc(term.sizex * sizeof(color_t));

		term_scr_clr(term.scr[i], term.attr[i], 0, AC_normal);
		term_scr_clr(term.scr0[i], term.attr0[i], 0, AC_normal);
	}
	term.f_cls = FALSE;
	term.tq = mem_alloc(term.sizey * sizeof(int));
	term_queue_clear();
}

void term_reinit()
{
	int sizex, sizey;

	if(tgetent(term_buf, term.name) <= 0) {
		sizey = 24;
		sizex = 79;
	} else {
		sizey = tgetnum("li");
		sizex = tgetnum("co");
		if(!tgetflag("am")) {
			sizex--;
		}
	}
	if(term.sizex != sizex || term.sizey != sizey) {
		int i;

		for(i = 0 ; i < term.sizey ; ++i) {
			free(term.scr[i]);
			free(term.scr0[i]);
			free(term.attr[i]);
			free(term.attr0[i]);
		}
		free(term.scr);
		free(term.scr0);
		free(term.attr);
		free(term.attr0);

		term.sizex = sizex;
		term.sizey = sizey;
		term_scr_init();
	}
}

static void term_setmode(int mode)
{
	if((mode & TM_init) && !(term.mode & TM_init)) {
		if(mode & TM_tinit) {
			term_tputs(term.t_init);
		}
		term_tputs(term.t_normal);

		term_getwsize();
		term_scr_init();

		term.x0 = 0;
		term.y0 = 0;
		term.cl = AC_normal;
		term.cl0= AC_normal;
		term.md_cursor0 = CS_ignore;
	}
	if((mode & TM_keypad) && !(term.mode & TM_keypad)) {
		term_tputs(term.t_keypad);
	}
	if((term.mode & TM_keypad) && !(mode & TM_keypad)) {
		term_tputs(term.t_nokeypad);
	}
	if((term.mode & TM_init) && !(mode & TM_init)) {
		term_tputs(term.t_normal);
		term_tputs(term.t_clear);
		term_tputs(term.t_normalcursor);

		if(term.mode & TM_tinit) {
			term_tputs(term.t_end);
		}
	}
	term.mode = mode;
	term_flush();
}

static int term_getmode()
{
	return term.mode;
}

void term_stop()	/* atexitされるので基本的には明示的に呼ぶ必要はない */
{
	if(exit_failed) {
		return;
	}
	exit_failed = TRUE;

	term_setattr(&term.tty);
	term_setmode(TM_none);

	term.fp_tty = stdout;
}

void term_start()
{
	term_ioctl_t tty;

	memcpy(&tty, &term.tty, sizeof(term_ioctl_t));
	tty.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ISIG | ICANON | IEXTEN);
	tty.c_iflag |= IGNBRK;
	tty.c_iflag &= ~(ICRNL | BRKINT | IXON);
	tty.c_oflag &= ~(ONLCR| OPOST);
	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 1;
	term_setattr(&tty);

	term_setmode(TM_init| TM_tinit| TM_keypad| TM_ansicolor);

	exit_failed = FALSE;
	atexit(term_stop);
}

void term_init()
{
	term.fp_tty = fopen(TTYNAME, "w+");
	if(term.fp_tty == NULL) {
		term.fp_tty = stdout;
	}
	term.mode = TM_none;
	term_getattr(&term.tty);
	term.name = getenv("TERM");
	if(term.name == NULL) {
		term.name = "";
	}
	if(tgetent(term_buf, term.name) <= 0) {
		report_puts("No termcap presented.");
		term.sizey = 24;
		term.sizex = 79;

		term.t_init         = "";
		term.t_end          = "";
		term.t_keypad       = "\033[?1h\033=";
		term.t_nokeypad     = "\033[?1l\033>";
		term.t_normalcursor = "";
		term.t_hidecursor   = "";
		term.t_vbell        = "\007";
		term.t_clear        = "\033[;H\033[2J";
		term.t_normal       = "\033[m";
		term.t_bold         = "\033[1m";
		term.t_reverse      = "\033[7m";
		term.t_underline    = "\033[4m";
		term.end_underline  = "\033[m";
		term.l_clear        = "\033[K";
		term.l_insert       = "\033[L";
		term.l_delete       = "\033[M";
		term.ln_insert      = "\033[%dL";
		term.ln_delete      = "\033[%dM";
		term.cn_locate      = "\033[%i%d;%dH";
	} else {
		term.sizey = tgetnum("li");
		term.sizex = tgetnum("co");
		if(!tgetflag("am")) {
		 	term.sizex--;
		}
		PC = *term_getent("pc", ""); //!! 面倒なのでFreeしていない
		BC = term_getent("bc", "\010");
		UP = term_getent("up", "\033[A");

		term.t_init         = term_getent("ti", "");
		term.t_end          = term_getent("te", "");
		term.t_keypad       = term_getent("ks", "");
		term.t_nokeypad     = term_getent("ke", "");
		term.t_normalcursor = term_getent("ve", "");
		term.t_hidecursor   = term_getent("vi", "");
		term.t_vbell        = term_getent("vb", "\007");
		term.t_clear        = term_getent("cl", "\033[;H\033[2J");
		term.t_normal       = term_getent("me", "\033[m");
		term.t_bold         = term_getent("md", "\033[1m");
		term.t_reverse      = term_getent("mr", "\033[7m");
		term.t_underline    = term_getent("us", "\033[4m");
		term.end_underline  = term_getent("ue", "\033[m");
		term.l_clear        = term_getent("ce", "\033[K");
		term.l_insert       = term_getent("al", "\033[L");
		term.l_delete       = term_getent("dl", "\033[M");
		term.ln_insert      = term_getent("AL", "");
		term.ln_delete      = term_getent("DL", "");
		term.cn_locate      = term_getent("cm", "\033[%i%d;%dH");
	}
}

void term_report()
{
	report_puts("端末情報:\n");
	report_printf("  x = %d, y = %d\n", term.sizex, term.sizey);

	report_puts("表示シーケンス:\n");

	report_printf("  term.t_init = %s\n", term.t_init);
	report_printf("  term.t_end = %s\n", term.t_end);
	report_printf("  term.t_keypad = %s\n", term.t_keypad);
	report_printf("  term.t_nokeypad = %s\n", term.t_nokeypad);
	report_printf("  term.t_normalcursor = %s\n", term.t_normalcursor);
	report_printf("  term.t_hidecursor = %s\n", term.t_hidecursor);
	report_printf("  term.t_vbell = %s\n", term.t_vbell);
	report_printf("  term.t_clear = %s\n", term.t_clear);
	report_printf("  term.t_normal = %s\n", term.t_normal);
	report_printf("  term.t_bold = %s\n", term.t_bold);
	report_printf("  term.t_reverse = %s\n", term.t_reverse);
	report_printf("  term.t_underline = %s\n", term.t_underline);
	report_printf("  term.end_underline = %s\n", term.end_underline);
	report_printf("  term.l_clear = %s\n", term.l_clear);
	report_printf("  term.l_insert = %s\n", term.l_insert);
	report_printf("  term.l_delete = %s\n", term.l_delete);
	report_printf("  term.ln_insert = %s\n", term.ln_insert);
	report_printf("  term.ln_delete = %s\n", term.ln_delete);
	report_printf("  term.cn_locate = %s\n", term.cn_locate);
}

void term_kflush()
{
	tcflush(fileno(term.fp_tty), TCIFLUSH);
}

static int term_dputc(int c)
{
	return fputc(c, term.fp_tty);
}

static void term_tputs(const char *s)
{
	tputs(s, term.sizey, term_dputc);
}

int term_getch()
{
	unsigned char ch;
	int i;

	while((i = read(fileno(term.fp_tty), &ch, sizeof(unsigned char))) < 0 && errno == EINTR) {
		// 
	}
	if(i < sizeof(unsigned char)) {
		return(EOF);
	}
	return((int)ch);
}

int term_kbhit(unsigned long usec)
{
#ifdef HAVE_SELECT
	fd_set readfds;
	struct timeval timeout;

	timeout.tv_sec  = 0;
	timeout.tv_usec = usec;
	FD_ZERO(&readfds);
	FD_SET (STDIN_FILENO, &readfds);

	return (select(1, &readfds, NULL, NULL, &timeout));

#else
 #ifdef HAVE_SYS_POLL_H
	struct poolfd pf;

	pf.fd      = STDIN_FILENO;
	pf.events  = POLLIN;
	pf.revents = 0;

	return (poll(&pf, 1, usec * 1000));
 #else
	return 1;
 #endif
#endif
}

void term_bell()
{
	term_tputs(term.t_vbell);
	term_flush();
}

void term_csr_flush()
{
	int x, y;

	x = term.x;
	y = term.y;

	if(term.md_cursor0 != CS_hide) {
		term_tputs(term.t_hidecursor);
		term.md_cursor0 = CS_hide;
	}
	term_all_flush();
	if(term.md_cursor0 != term.md_cursor) {
		if(term.md_cursor == CS_normal) {
			term_tputs(term.t_normalcursor);
		} else {
			term_tputs(term.t_hidecursor);
		}
		term.md_cursor0 = term.md_cursor;
	}
	if(term.md_cursor == CS_hide) {
		term_locate(0, term.sizex - 1);
	} else {
		term_locate(y, x);
	}
	term_locate_flush();
	term_flush();
}

void term_csrn()
{
	term.md_cursor = CS_normal;
}

void term_csrh()
{
	term.md_cursor = CS_hide;
}

void term_ungetch(unsigned char c)
{
#ifdef	TIOCSTI
	ioctl(fileno(term.fp_tty), TIOCSTI, &c);
#else
	if(ungetnum >= sizeof(ungetbuf) / sizeof(unsigned char) - 1) {
		return;
	}
	ungetbuf[ungetnum++] = c;
#endif
}

int term_regetch()
{
#ifndef	TIOCSTI
	if(ungetnum > 0) {
		return (int)ungetbuf[--ungetnum];
	}
#endif
	return -1;
}

static void term_locate_flush()
{
	char *p, *q, buf[LN_dispbuf + 1];
	int n;

	if(term.x == term.x0 && term.y == term.y0) {
		return;
	}
	p = tgoto(term.cn_locate, term.x, term.y);
	q = buf;
	n = strlen(p);
	if(term.y0 > term.y) {
		goto failed;
	}
	if(term.x < term.x0) {
		if(term.x0 - term.x >= term.x + 1) {
			*q++ = '\r';
			term.x0 = 0;
			--n;
		} else {
			while(n > 0 && term.x0 > term.x) {
				*q++ = '\b';
				--term.x0;
				--n;
			}
		}
	}
	while(n > 0 && term.y0 < term.y) {
		*q++ = '\n';
		++term.y0;
		--n;
	}
	if(n <= 0 || term.x0 != term.x || term.y0 != term.y) {
failed:
		term_tputs(p);
	} else {
		*q = '\0';
		term_tputs(buf);
	}
	term.x0 = term.x;
	term.y0 = term.y;
}

void term_locate(int y, int x)
{
	term.x = x;
	term.y = y;
}

void term_clrtoe(int ac)
{
	term_scr_clr(term.scr[term.y], term.attr[term.y], term.x, ac);
}

static int nbytes(int c)
{
	int ch = c & 0xf0;

	if(c == 0) {
		return 0;
	}
	if(ch == 0xc0 || ch == 0xd0) {
		return 2;
	}
	if(ch == 0xe0) {
		return 3;
	}
	if(ch == 0xf0) {
		return 4;
	}
	return 1;
}

void term_redraw_box(int sx, int sy, int width, int height)
{
	int w, h;

	h = 0;
	while(h < height) {
		if(sy + h < term.sizey) {
			w = 0;
			while(w < width) {
				if(sx + w < term.sizex) {
					term.scr0[sy + h][sx + w] = 0;
				}
				w++;
			}
		}
		h++;
	}
}

void term_redraw_line()
{
	int x = 0;
	while(x < term.sizex) {
		term.scr0[term.y][x] = 0;
		if(term.y < term.sizey - 1) {
			term.scr0[term.y + 1][x] = 0;
		}
		x++;
	}
}

int term_utf8_half_code(unsigned long c)
{
	char type = 0;
	wchar_t code;
	int ambiguous = term.ambiguous & AM_MASK;

	if(c < 0x100) {
		return TRUE;
	} else if(c < 0x10000) {
		code = ((c & 0x1f00) >> 2) | (c & 0x3f);
	} else if(c < 0x1000000) {
		code = ((c & 0xf0000) >> 4) | ((c & 0x3f00) >> 2) | (c & 0x3f);
	} else {
		code = ((c & 0x7000000) >> 6) | ((c & 0x3f0000) >> 4) | ((c & 0x3f00) >> 2) | (c & 0x3f);
	}
	if(code < 0x0080) {
		return TRUE;
	} else if(code <= 0x04ff) {
		if(ambiguous == AM_EMOJI2) {
			// (C) or (R)
			if(code == 0xa9 || code == 0xae) {
				return FALSE;
			}
		}
		type = byte2_width[code - 0x0080];
	} else if(code <= 0x1fff) {
		return TRUE;
	} else if(code >= 0x2000 && code <= 0x2bef) {
		if((term.ambiguous & AM_TERATERM) && code >= 0x2500 && code <= 0x257f) {
			return TRUE;
		}
		type = byte3_width[code - 0x2000];
	} else if((code >= 0xe000 && code <= 0xf8ff) || (code >= 0xf0000 && code <= 0xffffd) || (code >= 0x100000 && code <= 0x10fffd)) {
		// Private Use Areas
		type = 'A';
	}
	if(ambiguous == AM_EMOJI2 && code >= 0x2000 && code <= 0x329f) {
		wchar_t ecode = code - 0x2000;
		if(emoji[ecode / 8] & (1 << (ecode % 8))) {
			return FALSE;
		}
	}
	if(type == 'N' || type == 'H') {
		return TRUE;
	} else if(type == 'A') {
		if(ambiguous == AM_FIX2 || ambiguous == AM_EMOJI2) {
			return FALSE;
		} else if(ambiguous == AM_FIX1) {
			return TRUE;
		}
	}
	return FALSE;
}

int term_utf8_half_char(const char *p)
{
	const unsigned char *pu = (const unsigned char *)p;
	unsigned long c = *pu++;
	unsigned long ch = c & 0xf0;

	if(ch >= 0xc0) {
		c = (c << 8) | *pu++;
		if(ch >= 0xe0) {
			c = (c << 8) | *pu++;
			if(ch == 0xf0) {
				c = (c << 8) | *pu++;
			}
		}
	}
	return term_utf8_half_code(c);
}

int term_puts(const char *s, const char *ac)
{
	int len;
	int redraw = FALSE;
	unsigned long n;
	color_t attr;

	if(term.y >= term.sizey) {
		return FALSE;
	}
	if(term.x > 0 && term.scr[term.y][term.x] == SCR_ignore) {
		term.scr[term.y][term.x - 1] = ' ';
		term.attr[term.y][term.x - 1] = AC_normal;
	}
	while(*s != '\0' && term.x < term.sizex) {
		len = nbytes(*s);
		if(len == 1) {
			term.scr[term.y][term.x] = *s & 0xff;
			if(ac != NULL && *ac) {
				term.attr[term.y][term.x] = AC_color(*ac) | AC_attrib(term.cl);
			} else {
				term.attr[term.y][term.x] = term.cl;
			}
		} else {
			if(term.x + len > term.sizex) {
				break;
			}
			if(ac != NULL && *ac) {
				attr = AC_color(*ac) | AC_attrib(term.cl);
			} else {
				attr = term.cl;
			}
			if(len == 2) {
				n = (*s & 0xff) << 8 | (*(s + 1) & 0xff);
				term.scr[term.y][term.x] = n;
				term.attr[term.y][term.x] = attr;
				if(!term_utf8_half_code(n)) {
					term.x++;
					term.scr[term.y][term.x] = SCR_ignore;
					term.attr[term.y][term.x] = 0;
				}
				s++;
				if(ac != NULL) {
					ac++;
				}
			} else if(len == 3) {
				n = (*s & 0xff) << 16 | (*(s + 1) & 0xff) << 8 | (*(s + 2) & 0xff);
				if(n >= 0xefbda1 && n <= 0xefbe9f) {
					// 半角カナ
					term.scr[term.y][term.x] = n;
					term.attr[term.y][term.x] = attr;
					s += 2;
					if(ac != NULL) {
						ac += 2;
					}
				} else {
					if(n == 0xe38299 || n == 0xe3829a) {
						// 濁点・半濁点合成文字
						redraw = TRUE;
					} else if(n >= 0xefb880 && n <= 0xefb88f) {
						// 異体字・絵文字セレクタ
						redraw = TRUE;
					}
					term.scr[term.y][term.x] = n;
					term.attr[term.y][term.x] = attr;
					if(!term_utf8_half_code(n)) {
						term.x++;
						term.scr[term.y][term.x] = SCR_ignore;
						term.attr[term.y][term.x] = 0;
					}
					s += 2;
					if(ac != NULL) {
						ac += 2;
					}
				}
			} else if(len == 4) {
				n = ((unsigned long)(*s & 0xff)) << 24 | (*(s + 1) & 0xff) << 16 | (*(s + 2) & 0xff) << 8 | (*(s + 3) & 0xff);
				if(n >= 0xf3a08480L && n <= 0xf3a087afL) {
					// 異体字セレクタ
					redraw = TRUE;
				}
				term.scr[term.y][term.x] = n;
				term.attr[term.y][term.x] = attr;
				term.x++;
				term.scr[term.y][term.x] = SCR_ignore;
				term.scr[term.y][term.x] = 0;
				s += 3;
				if(ac != NULL) {
					ac += 3;
				}
			}
		}
		++term.x;
		if(ac != NULL) {
			++ac;
		}
		++s;
	}
	if(term.x < term.sizex && term.scr[term.y][term.x] == SCR_ignore) {
		term.scr[term.y][term.x] = ' ';
		term.attr[term.y][term.x] = AC_normal;
	}
	return redraw;
}

void term_putch(int c)
{
	char buf[1 + 1];

	*buf = c & 0xff;
	buf[1] = '\0';

	term_puts(buf, NULL);
}

void term_printf(const char *fmt, ...)
{
	va_list args;
	char buf[LN_dispbuf + 1];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	term_puts(buf, NULL);
}

#define isreverse(cl)	((cl) & AC_reverse)
#define isunder(cl)		((cl) & AC_under)
#define isbold(cl)		((cl) & AC_bold)

static void term_color_flush()
{
	if((isunder(term.cl0) && !isunder(term.cl))
	 || (isreverse(term.cl0) && !isreverse(term.cl))
	 || (isbold(term.cl0)    && !isbold(term.cl))) {
		term_tputs(term.t_normal);
		term.cl0 = AC_normal;
	}
	if(term.mode & TM_ansicolor) {
		if(AC_color(term.cl0) != AC_normal && AC_color(term.cl) == AC_normal) {
			term_tputs(term.t_normal);
			term.cl0 = AC_normal;
		}
		if(AC_color(term.cl) != AC_color(term.cl0)) {
			char buf[20 + 1];
			if(term.mode & TM_color256) {
				sprintf(buf, "\x1b[38;5;%dm", AC_color(term.cl));
			} else {
				sprintf(buf, "\x1b[3%dm", AC_color(term.cl) - 8);
			}
			term_tputs(buf);
		}
	}
	if(!isreverse(term.cl0) && isreverse(term.cl)) {
		term_tputs(term.t_reverse);
	}
	if(!isunder(term.cl0) && isunder(term.cl)) {
		term_tputs(term.t_underline);
	}
	if(!isbold(term.cl0) && isbold(term.cl)) {
		term_tputs(term.t_bold);
	}
	term.cl0 = term.cl;
}

void term_color(color_t cl)
{
	term.cl = cl;
}

void term_color_normal()
{
	term.cl = AC_normal;
}

void term_color_reverse()
{
	term.cl |= AC_reverse;
}

void term_color_underline()
{
	term.cl |= AC_under;
}

void term_color_bold()
{
	term.cl |= AC_bold;
}

color_t term_cftocol(const char *s)
{
	color_t cl;

	cl = AC_normal;
	if(s == NULL) {
		return cl;
	}
	for( ; *s != '\0' ; ++s) {
		switch(toupper(*s)) {
		case 'U':
			cl |= AC_under;
			continue;
		case 'R':
			cl |= AC_reverse;
			continue;
		case 'B':
			cl |= AC_bold;
			continue;
		}
		if(isdigit(*s)) {
			if(term.mode & TM_color256) {
				cl |= atoi(s);
				while(isdigit(*(s + 1))) {
					s++;
				}
			} else {
				cl |= AC_color(*s - '0' + 8);
			}
		}
	}
	return cl;
}

void term_color_enable(int color256)
{
	if((term_getmode() & TM_ansicolor) == 0 || (color256 && (term_getmode() & TM_color256) == 0)) {
		term_setmode(term_getmode() | TM_ansicolor | (color256 ? TM_color256 : 0));
	}
}

void term_color_disable()
{
	if((term_getmode() & TM_ansicolor) != 0) {
		term_setmode(term_getmode() & ~(TM_ansicolor | TM_color256));
	}
}

static void term_scroll_dn(int n)
{
	char buf[LN_dispbuf + 1];

	if(*term.ln_insert != '\0') {
		term_tputs(tparm(term.ln_insert, n));
	} else {
		while(n-- > 0) {
			term_tputs(term.l_insert);
		}
	}
}

static void term_scroll_up(int n)
{
	char buf[LN_dispbuf + 1];

	if(*term.ln_delete != '\0') {
		term_tputs(tparm(term.ln_delete, n));
	} else {
		while(n-- > 0) {
			term_tputs(term.l_delete);
		}
	}
}

void term_set_ambiguous(int mode)
{
	term.ambiguous = mode;
}

static void term_all_flush()
{
	int i, j;
	int n, x;
	bool f, cf;

	unsigned long *p, *p0;
	color_t *a, *a0;

	if(term.f_cls) {
		term_color_normal();
		term_color_flush();
		term_tputs(term.t_clear);

		term.x  = 0;
		term.y  = 0;
		term.x0 = 0;
		term.y0 = 0;

		for(i = 0 ; i < term.sizey ; ++i) {
			term_scr_clr(term.scr0[i], term.attr0[i], 0, AC_normal);
		}
		term.f_cls = FALSE;
	}
	for(i = 0 ; i < term.sizey ; ++i) {
		if(term.tq[i] > 0) {
			term_color_normal();
			term_color_flush();
			term_locate(i, 0);
			term_locate_flush();
			term_scroll_dn(term.tq[i]);
			while(term.tq[i] > 0) {
				void *p, *a;
				p = term.scr0[term.sizey - 1];
				a = term.attr0[term.sizey - 1];
				memmove(term.scr0 + i + 1, term.scr0 + i, sizeof(void *) * (term.sizey - i - 1));
				memmove(term.attr0 + i + 1, term.attr0 + i, sizeof(void *) * (term.sizey - i - 1));
				term.scr0[i] = p;
				term.attr0[i] = a;
				term_scr_clr(term.scr0[i], term.attr0[i], 0, AC_normal);
				--term.tq[i];
			}
		}

		if(term.tq[i] < 0) {
			term_color_normal();
			term_color_flush();
			term_locate(i, 0);
			term_locate_flush();
			term_scroll_up(0 - term.tq[i]);
			while(term.tq[i] < 0) {
				void *p, *a;
				p = term.scr0[i];
				a = term.attr0[i];
				memmove(term.scr0 + i, term.scr0 + i + 1, sizeof(void *) * (term.sizey - i - 1));
				memmove(term.attr0 + i, term.attr0 + i + 1, sizeof(void *) * (term.sizey - i - 1));
				term.scr0[term.sizey - 1] = p;
				term.attr0[term.sizey - 1] = a;
				term_scr_clr(term.scr0[term.sizey - 1], term.attr0[term.sizey - 1], 0, AC_normal);
				++term.tq[i];
			}
		}
	}
	for(term.y = 0 ; term.y < term.sizey ; ++term.y) {
		term.x = 0;
		cf = FALSE;
		for(n = term.sizex ; n > 0 ; --n) {
			if(term.scr[term.y][n - 1] != ' ' || term.attr[term.y][n - 1] != AC_normal) {
				break;
			}
			if(term.scr[term.y][n - 1] != term.scr0[term.y][n - 1] || term.attr[term.y][n - 1] != term.attr0[term.y][n - 1]) {
				cf = TRUE;
			}
		}
		p = term.scr[term.y];
		p0 = term.scr0[term.y];
		a = term.attr[term.y];
		a0 = term.attr0[term.y];
		while(term.x < n) {
			int x;
			f = FALSE;
			x = term.x;
			if((p[x] & 0xffffff00UL) != 0 || (p0[x] & 0xffffff00UL) != 0) {
				while((p[x] & 0xffffff00UL) != 0 || (p0[x] & 0xffffff00UL) != 0) {
					if(p[x] != p0[x] || a[x] != a0[x]) {
						f = TRUE;
					}
					++x;
				}
			} else {
				while(x < n && (p[x] & 0xffffff00UL) == 0 && (p0[x] & 0xffffff00UL) == 0) {
					if(!f && (p[x] != p0[x] || a[x] != a0[x])) {
						term.x = x;
						f = TRUE;
					}
					if(f && p[x] == p0[x] && a[x] == a0[x]) {
						break;
					}
					++x;
				}
			}
			if(x > n) {
				x = n;	// error
			}
			if(!f) {
				term.x = x;
				continue;
			}
			term_locate_flush();
			while(term.x < x) {
				unsigned long c;

				term_color(a[term.x]);
				term_color_flush();

				c = p[term.x];
				if((c & 0xffffff00UL) == 0) {
					fputc(c, term.fp_tty);
				} else {
					int cc;
					cc = ((c & 0xff000000UL) >> 24) & 0xff;
					if(cc != 0) {
						fputc(cc, term.fp_tty);
					}
					cc = ((c & 0xff0000UL) >> 16) & 0xff;
					if(cc != 0) {
						fputc(cc, term.fp_tty);
					}
					cc = ((c & 0xff00UL) >> 8) & 0xff;
					if(cc != 0) {
						fputc(cc, term.fp_tty);
					}
					fputc(c & 0xff, term.fp_tty);
			 		p0[term.x] = p[term.x];
			 		a0[term.x] = a[term.x];
			 		if(c < 0xefbda1 || c > 0xefbe9f) {
						// 半角カナ以外かつ曖昧幅が 1 でない場合
						if(!term_utf8_half_code(c)) {
							++term.x0;
							++term.x;
						}
					}
				}
				p0[term.x] = p[term.x];
				a0[term.x] = a[term.x];
				++term.x0;
				++term.x;
			}
		}
		if(cf) {
			term_locate_flush();
			term_color(AC_normal);
			term_color_flush();
			term_tputs(term.l_clear);

			term_scr_clr(p0, a0, n, AC_normal);
		}
	}
	term_flush();
}

