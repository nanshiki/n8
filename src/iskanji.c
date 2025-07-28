/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:iskanji.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 

	n8
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#include "n8.h"
#include "cursor.h"
#include <iconv.h>
#include <ctype.h>

#define	issjis1(c)	(((unsigned char)(c) >= 0x81 &&(unsigned char)(c) <= 0x9f) || ((unsigned char)(c) >= 0xe0 &&(unsigned char)(c) <= 0xfc))
#define	issjis2(c)	((unsigned char)(c) >= 0x40 &&(unsigned char)(c) <= 0xfc)
#define	iskana(c)	((unsigned char)(c) >= 0xa0 &&(unsigned char)(c) <= 0xdf)
#define	iseuc(c)	((unsigned char)(c) >= 0xa1 &&(unsigned char)(c) <= 0xfe)
#define	isjis(c)	((unsigned char)(c) >= 0x21 &&(unsigned char)(c) <= 0x7e)

bool iscnt(unsigned char c)
{
	return iscntrl(c) && c < 0x7f;
}

bool iskanji(int c)
{
	return (c & 0x80) == 0x80;
}

const char *get_utf8_code(const char *str, unsigned long *code)
{
	int ch;

	*code = *(unsigned char *)str++;
	ch = *code & 0xf0;
	if(*str != '\0' && ch >= 0xc0) {
		*code <<= 8;
		*code |= *(unsigned char *)str++;
		if(*str != '\0' && ch >= 0xe0) {
			*code <<= 8;
			*code |= *(unsigned char *)str++;
			if(*str != '\0' && ch >= 0xf0) {
				*code <<= 8;
				*code |= *(unsigned char *)str++;
			}
		}
	}
	return str;
}

int get_utf8_width(int c)
{
	int ch = c & 0xf0;

	if(ch == 0xc0 || ch == 0xd0) {
		return 2;
	} else if(ch == 0xe0) {
		return 3;
	} else if(ch == 0xf0) {
		return 4;
	}
	return 1;
}

int get_utf8_width_string(const char *str)
{
	int ch;
	int width = get_utf8_width(*str++);
	int count = 1;

	while(count < width) {
		ch = *str++ & 0xf0;
		if(ch < 0x80 && ch > 0xb0) {
			break;
		}
		count++;
	}
	return count;
}

int utf8_disp_copy(char *dst, const char *src, int count)
{
	int ch, cc;
	int width;
	int length = 0;

	while(count > 0) {
		if(!is_combining_char(src)) {
			length++;
			count--;
			if((*src & 0xf0) >= 0xc0 && !is_half_kana(src) && !term_utf8_half_char(src)) {
				length++;
				count--;
			}
		}
		width = get_utf8_width_string(src);
		memcpy(dst, src, width);
		dst += width;
		src += width;
	}
	if(is_combining_char(src)) {
		width = get_utf8_width_string(src);
		memcpy(dst, src, width);
		dst += width;
	}
	*dst = '\0';
	return length;
}

int get_utf8(const unsigned char *pt)
{
	int c, ch;

	c = *pt;
	ch = c & 0xf0;
	if(ch >= 0xc0) {
		pt++;
		c = (c << 8) | *pt;
		if(ch >= 0xe0) {
			pt++;
			c = (c << 8) | *pt;
			if(ch >= 0xf0) {
				pt++;
				c = (c << 8) | *pt;
			}
		}
	}
	return c;
}

int IsKanjiPosition()
{
	int ch, i, len;

	len = strlen(csrle.buf);
	i = GetBufferOffset();
	ch = csrle.buf[i] & 0xf0;
	if(ch >= 0xc0) {
		i++;
		if(i < len && (csrle.buf[i] & 0x80)) {
			if(ch == 0xc0 || ch == 0xd0) {
				return 2;
			}
			i++;
			if(i < len && (csrle.buf[i] & 0x80)) {
				if(ch == 0xe0) {
					return 3;
				}
				i++;
				if(i < len && (csrle.buf[i] & 0x80)) {
					return 4;
				}
			}
		}
	}
	return 0;
}

#define	CT_ank		(CT_space| CT_cntrl| CT_other| CT_alnum| CT_kana)
#define	CT_kana 	(CT_hira| CT_kata)

#define	CT_skip		1
#define	CT_other	2
#define	CT_alnum	8

#define	CT_hira 	16
#define	CT_kata 	32
#define	CT_kkigou	64
#define	CT_kalnum	128
#define	CT_kanji	256

int char_getctype(int c)
{
	if(c == 0) {
		return 0;
	}
	if(isspace(c) || iscnt(c)) {
		return CT_skip;
	} else if(isalnum(c) || c == '_') {
		return CT_alnum;
	}
	return CT_other;
}

int kanji_getctype(const char *pt)
{
	int c = get_utf8((const unsigned char *)pt);
	if(c >= 0xe38181 && c <= 0xe38296) {
		return CT_hira;
	} else if(c >= 0xe382a1 && c <= 0xe383bc) {
		return CT_kata;
	} else if((c >= 0xefbc90 && c <= 0xefbc99) || (c >= 0xefbca1 && c <= 0xefbcba)
	  || (c >= 0xefbd81 && c <= 0xefbd9a)) {
		return CT_kalnum;
	} else if((c >= 0xe2ba80 && c <= 0xe2bf95) || (c >= 0xe39080 && c <= 0xe9bfaa)
	 || (c >= 0xefa480 && c <= 0xefab99)) {
		return CT_kanji;
	}
	return CT_kkigou;
}

int kanji_tknext(const char *s, int a, bool f)
{
	int pa, pb;

	if(s[a] == '\0') {
		return a;
	}
	if(a > strlen(s)) {
		return strlen(s);
	}

	if(iskanji(s[a])) {
		pa = kanji_getctype(&s[a]);
		do	{
			a += get_utf8_width_string(&s[a]);
		} while(iskanji(s[a]) && (pa & kanji_getctype(&s[a])) != 0);
		pb = char_getctype(s[a]);
	} else {
		pa = char_getctype(s[a]);
		do	{
			 ++a;
			 pb = char_getctype(s[a]);
		} while(pa == pb && !iskanji(s[a]));
	}
	if(a == 0 || iskanji(s[a]) || (pb != CT_skip && pb != CT_other) || !f) {
		return a;
	}
	++a;
	while(pb == char_getctype(s[a]) && !iskanji(s[a])) {
		++a;
	}
	return a;
}

int kanji_tkleft(const char *s, int a)
{
	int ch;
	do {
		--a;
		ch = s[a] & 0xf0;
	} while(a > 0 && (ch >= 0x80 && ch <= 0xb0));
	return a;
}

int kanji_tkprev(const char *s, int a, bool f)
{
	int pa, pb;

	if(a <= 0) {
		return 0;
	}
	if(a > strlen(s)) {
		return strlen(s);
	}
	a = kanji_tkleft(s, a);
	pa = char_getctype(s[a]);
	while(a > 0 && !iskanji(s[a]) && (pa == CT_skip || pa == CT_other)) {
		a = kanji_tkleft(s, a);
		pa = char_getctype(s[a]);
	}
	if(iskanji(s[a])) {
		 pa = kanji_getctype(&s[a]);
		 do	{
			a = kanji_tkleft(s, a);
		 } while(a > 0 && iskanji(s[a]) && (pa & kanji_getctype(&s[a])) != 0);
		if(a > 0) {
			a += kanji_countbuf(&s[a]);
			pb = char_getctype(s[a]);
		}
	} else {
		do	{
			a = kanji_tkleft(s, a);
			pb = char_getctype(s[a]);
		} while(a > 0 && pa == pb && !iskanji(s[a]));
		if(iskanji(s[a])) {
			a += kanji_countbuf(&s[a]);
			pb = char_getctype(s[a]);
		}
	}
	if(a < 0) {
		a = 0;
	}
	if(a == 0 || iskanji(s[a]) || (pb != CT_skip && pb != CT_other) || !f) {
		return a;
	}
	a++;
	while(pb == char_getctype(s[a]) && !iskanji(s[a])) {
		a++;
	}
	return a;
}

#define	ESC	0x1b
#define	CR	0x0d
#define	LF	0x0a
#define	SI	0x0f
#define	SO	0x0e

static const char *source_name[] = {
	"EUC-JP",
	"ISO-2022-JP-2",
	"CP932",
	"UTF-8",
	"UTF-8",
};

const char *kanji_from_utf8(char *dst, const char *src, int kc)
{
	if(kc >= KC_utf8) {
		return src;
	}
	iconv_t ic;

	if((ic = iconv_open(source_name[kc], "UTF-8")) != (iconv_t)-1) {
		char *src_pt, *dst_pt;
		size_t src_length = strlen(src);
		size_t dst_length = src_length * 4;

		src_pt = (char *)src;
		dst_pt = (char *)dst;
		iconv(ic, &src_pt, &src_length, &dst_pt, &dst_length);
		*dst_pt = '\0';

		iconv_close(ic);
	}
	return dst;
}

static unsigned char utf8_bom[] = { 0xef, 0xbb, 0xbf, 0x00 };

void write_utf8_bom(FILE *fp)
{
	fputs((char *)utf8_bom, fp);
}

int file_kanji_check(FILE *fp)
{
	int c, h, ch;
	int f_sjis, f_euc, f_utf8;
	int n_sjis, n_sjis_i, n_euc, n_euc_i;
	int n_utf8, n_utf8_i;
	int bom = 0;

	n_sjis = 0;
	n_sjis_i = 0;
	n_euc = 0;
	n_euc_i = 0;
	n_utf8 = 0;
	n_utf8_i = 0;

	f_sjis = FALSE;
	f_euc = 0;
	f_utf8 = 0;

	for(;;) {
		c = fgetc(fp);
		if(bom != -1) {
			if(c == utf8_bom[bom]) {
				bom++;
				if(bom == 3) {
					return KC_utf8bom;
				}
			} else {
				bom = -1;
			}
		}
		if(c == EOF || n_euc > 20 || n_sjis > 20) {
			if(n_sjis > n_euc && n_sjis > n_utf8 && n_sjis_i == 0 && n_utf8_i > 0) {
				return KC_sjis;
			} else if(n_euc > n_sjis && n_euc > n_utf8 && n_euc_i == 0) {
				return KC_euc;
			}
			break;
		}
		if(c == ESC) {
			c = fgetc(fp);
			if(c == 'K') {
				return KC_jis;
			} else if(c == '$') {
				c = fgetc(fp);
				if(c == 'B' || c == '@') {
					return KC_jis;
				}
			}
		}
		if(f_euc) {
			if(iseuc(c) || (f_euc == 0x8e && c >= 0xa0 && c <= 0xdf)) {
				++n_euc;
			} else {
				++n_euc_i;
			}
			f_euc = 0;
		} else {
			if(iseuc(c) || c == 0x8e) {
				f_euc = c;
			}
		}
		if(f_sjis) {
			if(issjis2(c)) {
				++n_sjis;
			} else {
				++n_sjis_i;
			}
			f_sjis = FALSE;
		} else {
			if(issjis1(c)) {
				f_sjis = TRUE;
			}
		}

		ch = c & 0xf0;
		if(f_utf8 > 0) {
			if(ch >= 0x80 && ch <= 0xb0) {
				f_utf8--;
				if(f_utf8 == 0) {
					++n_utf8;
				}
			} else {
				f_utf8 = 0;
				++n_utf8_i;
			}
		} else {
			f_utf8 = get_utf8_width(c) - 1;
			if(f_utf8 == 0 && (c & 0x80)) {
				++n_utf8_i;
			}
		}
	}
	return KC_utf8;
}

int file_gets(char *s, size_t bytes, FILE *fp, int *n_cr, int *n_lf)
{
	int c;
	bool f_cr;

	f_cr = FALSE;
	for( ; bytes > 0 ; ) {
		c = fgetc(fp);
		if(c == LF || f_cr) {
			if(c == LF) {
				++*n_lf;
			} else {
				ungetc(c, fp);
			}
			c = 0;
			break;
		}
		if(c == EOF) {
			c = -1;
			break;
		}
		if(c == CR) {
			f_cr = TRUE;
			++*n_cr;
			continue;
		}

		*s++ = c;
		--bytes;
	}
	*s = '\0';
	return c;
}

void kanji_to_utf8(char *dst, const char *src, int kc)
{
	if(kc >= KC_utf8) {
		strcpy(dst, src);
		return;
	}
	iconv_t ic;

	if((ic = iconv_open("UTF-8", source_name[kc])) != (iconv_t)-1) {
		char *src_pt, *dst_pt;
		size_t src_length = strlen(src);
		size_t dst_length = src_length * 4;

		src_pt = (char *)src;
		dst_pt = (char *)dst;
		iconv(ic, &src_pt, &src_length, &dst_pt, &dst_length);
		*dst_pt = '\0';

		iconv_close(ic);
	}
}

int kanji_poscanon(int offset, const char *buf)
{
	int n, m;

	offset = min(strlen(buf), offset);
	n = m = 0;
	for(;;) {
		if(n == offset) {
			return offset;
		} else if(n > offset) {
			return m;
		} else if(buf[n] == '\0') {
		 	return n;
		}
		m = n;
		n += kanji_countbuf(&buf[n]);
	}
}

int kanji_poscandsp(int offset, const char *buf)
{
	int n, m, a, ln;

	ln = strlen(buf);
	n = m = a = 0;
	for(;;) {
		if(n == offset) {
			return offset;
		} else if(n > offset) {
			return m;
		} else if(a > ln || buf[a] == '\0') {
			return n;
		}
		m = n;
		n += kanji_countdsp(&buf[a], n);
		a += kanji_countbuf(&buf[a]);
	}
}

int kanji_posnext(int offset, const char *buf)
{
	int i, n;

	n = kanji_countbuf(&buf[offset]);

	for(i = 0 ; i < n ; ++i) {
		if(buf[offset + i] == '\0') {
			break;
		}
	}
	return offset + i;
}

int kanji_posprev(int offset, const char *buf)
{
	int n, m;

	n = m = 0;
	for(;;) {
		if(n >= offset || buf[n] == '\0') {
			return m;
		}
		m = n;
		n += kanji_countbuf(&buf[n]);
	}
}

int kanji_posdsp(int offset, const char *buf)
{
	int n, m;

	n = 0;
	m = 0;
	for(;;) {
		if(n >= offset || buf[n] == '\0') {
			return m;
		}
		m += kanji_countdsp(&buf[n], m);
		n += kanji_countbuf(&buf[n]);
	}
}

int kanji_posbuf(int offset, const char *buf)
{
	int n, m;

	n = 0;
	m = 0;
	for(;;) {
		if(m >= offset || buf[n] == '\0') {
			return n;
		}
		m += kanji_countdsp(&buf[n], m);
		n += kanji_countbuf(&buf[n]);
	}
}

int strjfcpy(char *s, const char *t, size_t bytes, size_t len, bool space)
{
	int n, m;
	int width = 0;

	for( ; *t != 0 ; ) {
		n = kanji_countbuf(t);
		m = kanji_countdsp(t, -1);
		if(bytes < n || len < m) {
			break;
		}
		memcpy(s, t, n);
		s += n;
		t += n;
		bytes -= n;
		len -= m;
		width += m;
	}
	if(space) {
		for( ; len > 0 && bytes > 0 ; --len, --bytes) {
			*s++ = ' ';
			width++;
		}
	}
	*s = '\0';
	return width;
}

int check_frame_ambiguous2()
{
	return sysinfo.framechar >= frameCharFrame && sysinfo.ambiguous != AM_FIX1;
}

int is_zen_space(const char *p)
{
	if(sysinfo.zenspacef) {
		const unsigned char *pu = (const unsigned char *)p;
		if(*pu == 0xe3 && *(pu + 1) == 0x80 && *(pu + 2) == 0x80) {
			return TRUE;
		}
	}
	return FALSE;
}

int is_half_kana(const char *p)
{
	const unsigned char *pu = (const unsigned char *)p;
	if(*pu == 0xef) {
		if((*(pu + 1) == 0xbd && *(pu + 2) >= 0xa1) || (*(pu + 1) == 0xbe && *(pu + 2) <= 0x9f)) {
			return TRUE;
		}
	}
	return FALSE;
}

int is_combining_char(const char *p)
{
	if(sysinfo.nfdf) {
		const unsigned char *pu = (const unsigned char *)p;
		// 濁点・半濁点合成文字
		if(*pu == 0xe3 && *(pu + 1) == 0x82 && (*(pu + 2) == 0x99 || *(pu + 2) == 0x9a)) {
			return TRUE;
		}
	}
	return FALSE;
}

int kanji_countbuf(const char *p)
{
	unsigned char ch = (unsigned char)*p & 0xf0;

	if(*p == 0) {
		return 0;
	} else if(ch == 0xc0 || ch == 0xd0) {
		return 2;
	} else if(ch == 0xe0) {
		if(is_combining_char(p)) {
			return 0;
		} else if(is_combining_char(p + 3)) {
			return 6;
		}
		return 3;
	} else if(ch == 0xf0) {
		return 4;
	}
	return 1;
}

int kanji_countdsp(const char *p, int n)
{
	unsigned char ch = (unsigned char)*p & 0xf0;

	if(*p == 0) {
		return 0;
	} else if(*p == '\t' && n != -1) {
		return (n / edbuf[CurrentFileNo].tabstop + 1) * edbuf[CurrentFileNo].tabstop - n;
	} else if(ch == 0xc0 || ch == 0xd0) {
		if(term_utf8_half_char(p)) {
			return 1;
		}
		return 2;
	} else if(ch == 0xe0 || ch == 0xf0) {
		int len = 2;
		if(is_half_kana(p) || term_utf8_half_char(p)) {
			return 1;
		} else if(is_combining_char(p)) {
			return 0;
		}
		return 2;
	} else if(iscntrl(*p)) {
		return 2;
	}
	return 1;
}

int get_delete_count(const char *str)
{
	int count = 0;
	int width;

	while(*str != '\0') {
		width = kanji_countbuf(str);
		str += width;
		count++;
	}
	return count;
}

int get_display_length(const char *str)
{
	int length = 0;
	unsigned char ch;

	while(*str != '\0') {
		ch = (unsigned char)*str & 0xf0;
		if(ch >= 0xc0) {
			if(is_combining_char(str)) {
				// 濁点・半濁点合成文字
				length--;
				str += 2;
			} else if(is_half_kana(str)) {
				// 半角カナ
				str += 2;
			} else {
				if(!term_utf8_half_char(str)) {
					length++;
				}
				str++;
				if(((unsigned char)*str & 0x80) && ch >= 0xe0) {
					str++;
					if(((unsigned char)*str & 0x80) && ch == 0xf0) {
						str++;
					}
				}
			}
		}
		if(*str == '\0') {
			break;
		}
		str++;
		length++;
	}
	return length;
}

