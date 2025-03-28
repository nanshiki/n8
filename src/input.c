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
		LeditInput(key, NONE);
	}
	csr_movehook();
}

char *HisGets(char *dest, const char *message, int listID)
{
	return legets_gets(message, dest, GetColWidth(), MAXLINESTR, listID) == ESCAPE ? NULL : dest;
}

int GetS(const char *message, char *buffer)
{
	return legets_gets(message, buffer, GetColWidth(), MAXLINESTR, -1);
}
