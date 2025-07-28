/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:input.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 

	n8
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#include "n8.h"
#include "input.h"
#include "crt.h"
#include "cursor.h"
#include "line.h"
#include "lineedit.h"

void LeditInput(int c, int contrl_flag)
{
	int lx;

	lx = csrle.lx;
	le_edit(&csrle, c, contrl_flag);
	csr_fix();
}

void Ledit(int contrl_flag)
{
	LeditInput(' ', contrl_flag);
}

bool check_comment()
{
	bool comment = FALSE;
	int n = 0;
	int len = (int)strlen(csrle.buf);

	while(n < len) {
		if(!comment) {
			if(csrle.buf[n] == '/' && n < len - 1) {
				if(csrle.buf[n + 1] == '/' || csrle.buf[n + 1] == '*') {
					comment = TRUE;
				}
			}
		} else {
			if(csrle.buf[n] == '*' && n < len - 1) {
				if(csrle.buf[n + 1] == '/') {
					comment = FALSE;
				}
			}
		}
		n++;
	}
	return comment;
}

void InputAndCrt(unsigned long key)
{
	int ch = key & 0xf0;
	if(sysinfo.overwritef) {
		se_delete(1 , FALSE);
	}
	if(ch >= 0xc0) {
		key <<= 8;
		key |= term_inkey();
		if(ch >= 0xe0) {
			key <<= 8;
			key |= term_inkey();
			if(ch >= 0xf0) {
				key <<= 8;
				key |= term_inkey();
			}
		}
		LeditInput(key, NONE);
		term_locate(0, GetRow());
	} else {
		if(edbuf[CurrentFileNo].cmode && key == '}' && !check_comment()) {
			if(csrle.lx > 0) {
				if(csrle.buf[csrle.lx - 1] == '\t') {
					csrle.lx--;
					csrle.buf[csrle.lx] = '\0';
				}
			}
		}
		LeditInput(key, NONE);
	}
	csr_movehook();
}

#define	INPUT_BOX_WIDTH_MIN		40
#define	INPUT_BOX_WIDTH_MAX		60

int input_box_width()
{
	int width = GetColWidth() / 2;
	if(width > INPUT_BOX_WIDTH_MAX) {
		width = INPUT_BOX_WIDTH_MAX;
	} else if(width < INPUT_BOX_WIDTH_MIN) {
		width = INPUT_BOX_WIDTH_MIN;
	}
	return width;
}

char *HisGets(char *dest, const char *message, int listID)
{
	int width = input_box_width();
	if(sysinfo.ambiguous != AM_FIX1 && (width & 1) == 0) {
		width++;
	}
	return legets_gets(message, dest, width, MAXLINESTR, listID) == ESCAPE ? NULL : dest;
}

int GetS(const char *message, char *buffer, int width)
{
	if(width == -1) {
		width = input_box_width();
	}
	if(sysinfo.ambiguous != AM_FIX1 && (width & 1) == 0) {
		width++;
	}
	return legets_gets(message, buffer, width, MAXLINESTR, -1);
}
