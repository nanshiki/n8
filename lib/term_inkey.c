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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif


#include "generic.h"
#include "term.h"

#define SENSEPERSEC	50
#define WAITKEYPAD	360		/* msec */

#define MAX_esc	1024
typedef struct
{
	char *seq;
	int scode;

	int len;
} esc_t;

static esc_t esc[MAX_esc];
static int esc_num = 0;

void term_keyreport()
{
	int i;

	report_puts("キーシーケンス:\n");
	for(i = 0 ; i < esc_num ; ++i) {
		report_printf("  [%s] -> %x\n", esc[i].seq, esc[i].scode);
	}
}

void term_escset(int n, const char *e, const char *d)
{
	if(esc_num >= MAX_esc || *e == '\0') {
		return;
	}
	esc[esc_num].scode = n;
	if(*e == '&') {
		esc[esc_num].seq = term_getent(e + 1, d);
		if(esc[esc_num].seq == NULL || *esc[esc_num].seq == '\0') {
		 	return;
		}
	} else {
		esc[esc_num].seq = mem_strdup(e);
	}
	esc[esc_num].len = strlen(esc[esc_num].seq);
	++esc_num;
}

#define MAX_keyring	64
#define MAX_skip	4

int key_ring[MAX_keyring];
int key_ring_in = 0;
int key_ring_out = 0;
int key_skip = 0;

void key_ring_pushall(long n)
{
	int ch;

	for(;;) {
		if((key_ring_in + 1) % MAX_keyring == key_ring_out || !term_kbhit(n)) {
			return;
		}
		ch = term_getch();
		if(ch != -1) {
			key_ring[key_ring_in++] = ch;
			if(key_ring_in >= MAX_keyring) {
				key_ring_in = 0;
			}
		}
	}
}

int key_ring_pop()
{
	int ch;

	while(key_ring_in == key_ring_out) {
		key_ring_pushall(1000L);
	}
	ch = key_ring[key_ring_out++];
	if(key_ring_out >= MAX_keyring) {
		key_ring_out = 0;
	}
	return ch;
}

void key_ring_push(int ch)
{
	--key_ring_out;
	if(key_ring_out < 0) {
		key_ring_out = MAX_keyring;
	}
	key_ring[key_ring_out] = ch;
}

int term_inkey()
{
	int i, j, ch, key, start, end, s, e;

	if(key_ring_in != key_ring_out && key_skip < MAX_skip) {
		++key_skip;
	} else {
		key_skip = 0;
		term_csr_flush();
	}
	key = ch = key_ring_pop();
	start = 0;
	end = esc_num;
	for(i = 0 ; ; i++) {
		s = start;
		e = end;
		start = end = -1;
		for(j = s ; j < e ; j++) {
			if(i >= esc[j].len || esc[j].seq[i] != ch) {
				if(end != -1) {
					break;
				}
			} else {
			 	if(esc[j].len == i + 1) {
			 		return (esc[j].scode);
			 	}

		 		if(start == -1) {
		 			start = j;
		 		}
				end = j + 1;
			}
		}
		if(start == -1) {
			break;
		}
		if(key_ring_in == key_ring_out && !term_kbhit(WAITKEYPAD * 1000L)) {
			break;
		}
		ch = key_ring_pop();
	}
	if(i > 0) {
		key_ring_push(ch);
		while(--i > 0) {
			key_ring_push(esc[j - 1].seq[i]);
		}
	}
	return (key);
}


#define KS_shift	0x10000
#define KS_ctrl		0x20000
#define KS_alt		0x40000

typedef struct
{
	char *name;
	int scode;
} keysdef_t;

keysdef_t keysdef[]=
{
	{"Tab",							0x09},
	{"Linefeed",					0x0A},
	{"Return",						0x0D},
	{"CR",							0x0D},
	{"Escape",						0x1B},
	{"ESC",							0x1B},
	{"SPACE",						' '},


	{"BackSpace", 					0xFF08},
	{"BS",		 					0xFF08},
	{"Clear",						0xFF0B},
	{"CLR",							0xFF0B},
	{"Pause",						0xFF13},
	{"Scroll_Lock",					0xFF14},
	{"Sys_Req",						0xFF15},
	{"Delete",						0xFFFF},
	{"DEL",							0xFFFF},

	{"Multi_key",					0xFF20},
	{"Codeinput",					0xFF37},
	{"SingleCandidate",				0xFF3C},
	{"MultipleCandidate",			0xFF3D},
	{"PreviousCandidate",			0xFF3E},

	{"Kanji",						0xFF21},
	{"Muhenkan",					0xFF22},
	{"Henkan_Mode",					0xFF23},
	{"Henkan",						0xFF23},
	{"Romaji",						0xFF24},
	{"Hiragana",					0xFF25},
	{"Katakana",					0xFF26},
	{"Hiragana_Katakana",			0xFF27},
	{"Zenkaku",						0xFF28},
	{"Hankaku",						0xFF29},
	{"Zenkaku_Hankaku",				0xFF2A},
	{"Touroku",						0xFF2B},
	{"Massyo",						0xFF2C},
	{"Kana_Lock",					0xFF2D},
	{"Kana_Shift",					0xFF2E},
	{"Eisu_Shift",					0xFF2F},
	{"Eisu_toggle",					0xFF30},
	{"Kanji_Bangou",				0xFF37},
	{"Zen_Koho",					0xFF3D},
	{"Mae_Koho",					0xFF3E},

	{"Home",						0xFF50},
	{"Left",						0xFF51},
	{"Up",							0xFF52},
	{"Right",						0xFF53},
	{"Down",						0xFF54},
	{"Prior",						0xFF55},
	{"Page_Up",						0xFF55},
	{"PGUP",						0xFF55},
	{"RLDN",						0xFF55},
	{"Next",						0xFF56},
	{"Page_Down",					0xFF56},
	{"PGDN",						0xFF56},
	{"RLUP",						0xFF56},
	{"End",							0xFF57},
	{"Begin",						0xFF58},

	{"Select",						0xFF60},
	{"Print",						0xFF61},
	{"Execute",						0xFF62},
	{"Insert",						0xFF63},
	{"INS",							0xFF63},
	{"Undo",						0xFF65},
	{"Redo",						0xFF66},
	{"Menu",						0xFF67},
	{"Find",						0xFF68},
	{"Cancel",						0xFF69},
	{"Help",						0xFF6A},
	{"Break",						0xFF6B},
	{"Mode_switch",					0xFF7E},
	{"script_switch",				0xFF7E},
	{"Num_Lock",					0xFF7F},


	{"KP_Space",					0xFF80},
	{"KP_Tab",						0xFF89},
	{"KP_Enter",					0xFF8D},
	{"KP_F1",						0xFF91},
	{"KP_F2",						0xFF92},
	{"KP_F3",						0xFF93},
	{"KP_F4",						0xFF94},
	{"KP_Home",						0xFF95},
	{"KP_Left",						0xFF96},
	{"KP_Up",						0xFF97},
	{"KP_Right",					0xFF98},
	{"KP_Down",						0xFF99},
	{"KP_Prior",					0xFF9A},
	{"KP_Page_Up",					0xFF9A},
	{"KP_Next",						0xFF9B},
	{"KP_Page_Down",				0xFF9B},
	{"KP_End",						0xFF9C},
	{"KP_Begin",					0xFF9D},
	{"KP_Insert",					0xFF9E},
	{"KP_Delete",					0xFF9F},
	{"KP_Equal",					0xFFBD},
	{"KP_Multiply",					0xFFAA},
	{"KP_Add",						0xFFAB},
	{"KP_Separator",				0xFFAC},
	{"KP_Subtract",					0xFFAD},
	{"KP_Decimal",					0xFFAE},
	{"KP_Divide",					0xFFAF},

	{"KP_0",						0xFFB0},
	{"KP_1",						0xFFB1},
	{"KP_2",						0xFFB2},
	{"KP_3",						0xFFB3},
	{"KP_4",						0xFFB4},
	{"KP_5",						0xFFB5},
	{"KP_6",						0xFFB6},
	{"KP_7",						0xFFB7},
	{"KP_8",						0xFFB8},
	{"KP_9",						0xFFB9},

	{"F1",							0xFFBE},
	{"F01",							0xFFBE},
	{"F2",							0xFFBF},
	{"F02",							0xFFBF},
	{"F3",							0xFFC0},
	{"F03",							0xFFC0},
	{"F4",							0xFFC1},
	{"F04",							0xFFC1},
	{"F5",							0xFFC2},
	{"F05",							0xFFC2},
	{"F6",							0xFFC3},
	{"F06",							0xFFC3},
	{"F7",							0xFFC4},
	{"F07",							0xFFC4},
	{"F8",							0xFFC5},
	{"F08",							0xFFC5},
	{"F9",							0xFFC6},
	{"F09",							0xFFC6},
	{"F10",							0xFFC7},
	{"F11",							0xFFC8},
	{"L1",							0xFFC8},
	{"F12",							0xFFC9},
	{"L2",							0xFFC9},
	{"F13",							0xFFCA},
	{"L3",							0xFFCA},
	{"F14",							0xFFCB},
	{"L4",							0xFFCB},
	{"F15",							0xFFCC},
	{"L5",							0xFFCC},
	{"F16",							0xFFCD},
	{"L6",							0xFFCD},
	{"F17",							0xFFCE},
	{"L7",							0xFFCE},
	{"F18",							0xFFCF},
	{"L8",							0xFFCF},
	{"F19",							0xFFD0},
	{"L9",							0xFFD0},
	{"F20",							0xFFD1},
	{"L10",							0xFFD1},
	{"F21",							0xFFD2},
	{"R1",							0xFFD2},
	{"F22",							0xFFD3},
	{"R2",							0xFFD3},
	{"F23",							0xFFD4},
	{"R3",							0xFFD4},
	{"F24",							0xFFD5},
	{"R4",							0xFFD5},
	{"F25",							0xFFD6},
	{"R5",							0xFFD6},
	{"F26",							0xFFD7},
	{"R6",							0xFFD7},
	{"F27",							0xFFD8},
	{"R7",							0xFFD8},
	{"F28",							0xFFD9},
	{"R8",							0xFFD9},
	{"F29",							0xFFDA},
	{"R9",							0xFFDA},
	{"F30",							0xFFDB},
	{"R10",							0xFFDB},
	{"F31",							0xFFDC},
	{"R11",							0xFFDC},
	{"F32",							0xFFDD},
	{"R12",							0xFFDD},
	{"F33",							0xFFDE},
	{"R13",							0xFFDE},
	{"F34",							0xFFDF},
	{"R14",							0xFFDF},
	{"F35",							0xFFE0},
	{"R15",							0xFFE0},

	{"Shift_L",						0xFFE1},
	{"Shift_R",						0xFFE2},
	{"Control_L",					0xFFE3},
	{"Control_R",					0xFFE4},
	{"Caps_Lock",					0xFFE5},
	{"Shift_Lock",					0xFFE6},

	{"Meta_L",						0xFFE7},
	{"Meta_R",						0xFFE8},
	{"Alt_L",						0xFFE9},
	{"Alt_R",						0xFFEA},
	{"Super_L",						0xFFEB},
	{"Super_R",						0xFFEC},
	{"Hyper_L",						0xFFED},
	{"Hyper_R",						0xFFEE},


	{"ISO_Lock",					0xFE01},
	{"ISO_Level2_Latch",			0xFE02},
	{"ISO_Level3_Shift",			0xFE03},
	{"ISO_Level3_Latch",			0xFE04},
	{"ISO_Level3_Lock",				0xFE05},
	{"ISO_Group_Shift",				0xFF7E},
	{"ISO_Group_Latch",				0xFE06},
	{"ISO_Group_Lock",				0xFE07},
	{"ISO_Next_Group",				0xFE08},
	{"ISO_Next_Group_Lock",			0xFE09},
	{"ISO_Prev_Group",				0xFE0A},
	{"ISO_Prev_Group_Lock",			0xFE0B},
	{"ISO_First_Group",				0xFE0C},
	{"ISO_First_Group_Lock",		0xFE0D},
	{"ISO_Last_Group",				0xFE0E},
	{"ISO_Last_Group_Lock",			0xFE0F},

	{"ISO_Left_Tab",				0xFE20},
	{"ISO_Move_Line_Up",			0xFE21},
	{"ISO_Move_Line_Down",			0xFE22},
	{"ISO_Partial_Line_Up",			0xFE23},
	{"ISO_Partial_Line_Down",		0xFE24},
	{"ISO_Partial_Space_Left",		0xFE25},
	{"ISO_Partial_Space_Right",		0xFE26},
	{"ISO_Set_Margin_Left",			0xFE27},
	{"ISO_Set_Margin_Right",		0xFE28},
	{"ISO_Release_Margin_Left",		0xFE29},
	{"ISO_Release_Margin_Right",	0xFE2A},
	{"ISO_Release_Both_Margins",	0xFE2B},
	{"ISO_Fast_Cursor_Left",		0xFE2C},
	{"ISO_Fast_Cursor_Right",		0xFE2D},
	{"ISO_Fast_Cursor_Up",			0xFE2E},
	{"ISO_Fast_Cursor_Down",		0xFE2F},
	{"ISO_Continuous_Underline",	0xFE30},
	{"ISO_Discontinuous_Underline",	0xFE31},
	{"ISO_Emphasize",				0xFE32},
	{"ISO_Center_Object",			0xFE33},
	{"ISO_Enter",					0xFE34},

	{"dead_grave",					0xFE50},
	{"dead_acute",					0xFE51},
	{"dead_circumflex",				0xFE52},
	{"dead_tilde",					0xFE53},
	{"dead_macron",					0xFE54},
	{"dead_breve",					0xFE55},
	{"dead_abovedot",				0xFE56},
	{"dead_diaeresis",				0xFE57},
	{"dead_abovering",				0xFE58},
	{"dead_doubleacute",			0xFE59},
	{"dead_caron",					0xFE5A},
	{"dead_cedilla",				0xFE5B},
	{"dead_ogonek",					0xFE5C},
	{"dead_iota",					0xFE5D},
	{"dead_voiced_sound",			0xFE5E},
	{"dead_semivoiced_sound",		0xFE5F},
	{"dead_belowdot",				0xFE60},

	{"First_Virtual_Screen",		0xFED0},
	{"Prev_Virtual_Screen",			0xFED1},
	{"Next_Virtual_Screen",			0xFED2},
	{"Last_Virtual_Screen",			0xFED4},
	{"Terminate_Server",			0xFED5},

	{"AccessX_Enable",				0xFE70},
	{"AccessX_Feedback_Enable",		0xFE71},
	{"RepeatKeys_Enable",			0xFE72},
	{"SlowKeys_Enable",				0xFE73},
	{"BounceKeys_Enable",			0xFE74},
	{"StickyKeys_Enable",			0xFE75},
	{"MouseKeys_Enable",			0xFE76},
	{"MouseKeys_Accel_Enable",		0xFE77},
	{"Overlay1_Enable",				0xFE78},
	{"Overlay2_Enable",				0xFE79},
	{"AudibleBell_Enable",			0xFE7A},

	{"Pointer_Left",				0xFEE0},
	{"Pointer_Right",				0xFEE1},
	{"Pointer_Up",					0xFEE2},
	{"Pointer_Down",				0xFEE3},
	{"Pointer_UpLeft",				0xFEE4},
	{"Pointer_UpRight",				0xFEE5},
	{"Pointer_DownLeft",			0xFEE6},
	{"Pointer_DownRight",			0xFEE7},
	{"Pointer_Button_Dflt",			0xFEE8},
	{"Pointer_Button1",				0xFEE9},
	{"Pointer_Button2",				0xFEEA},
	{"Pointer_Button3",				0xFEEB},
	{"Pointer_Button4",				0xFEEC},
	{"Pointer_Button5",				0xFEED},
	{"Pointer_DblClick_Dflt",		0xFEEE},
	{"Pointer_DblClick1",			0xFEEF},
	{"Pointer_DblClick2",			0xFEF0},
	{"Pointer_DblClick3",			0xFEF1},
	{"Pointer_DblClick4",			0xFEF2},
	{"Pointer_DblClick5",			0xFEF3},
	{"Pointer_Drag_Dflt",			0xFEF4},
	{"Pointer_Drag1",				0xFEF5},
	{"Pointer_Drag2",				0xFEF6},
	{"Pointer_Drag3",				0xFEF7},
	{"Pointer_Drag4",				0xFEF8},
	{"Pointer_Drag5",				0xFEFD},

	{"Pointer_EnableKeys",			0xFEF9},
	{"Pointer_Accelerate",			0xFEFA},
	{"Pointer_DfltBtnNext",			0xFEFB},
	{"Pointer_DfltBtnPrev",			0xFEFC},

/*
	{"nobreakspace",				0x0a0},
	{"exclamdown",					0x0a1},
	{"cent",						0x0a2},
	{"sterling",					0x0a3},
	{"currency",					0x0a4},
	{"yen",							0x0a5},
	{"brokenbar",					0x0a6},
	{"section",						0x0a7},
	{"diaeresis",					0x0a8},
	{"copyright",					0x0a9},
	{"ordfeminine",					0x0aa},
	{"guillemotleft",				0x0ab},
	{"notsign",						0x0ac},
	{"hyphen",						0x0ad},
	{"registered",					0x0ae},
	{"macron",						0x0af},
	{"degree",						0x0b0},
	{"plusminus",					0x0b1},
	{"twosuperior",					0x0b2},
	{"threesuperior",				0x0b3},
	{"acute",						0x0b4},
	{"mu",							0x0b5},
	{"paragraph",					0x0b6},
	{"periodcentered",				0x0b7},
	{"cedilla",						0x0b8},
	{"onesuperior",					0x0b9},
	{"masculine",					0x0ba},
	{"guillemotright",				0x0bb},
	{"onequarter",					0x0bc},
	{"onehalf",						0x0bd},
	{"threequarters",				0x0be},
	{"questiondown",				0x0bf},
	{"Agrave",						0x0c0},
	{"Aacute",						0x0c1},
	{"Acircumflex",					0x0c2},
	{"Atilde",						0x0c3},
	{"Adiaeresis",					0x0c4},
	{"Aring",						0x0c5},
	{"AE",							0x0c6},
	{"Ccedilla",					0x0c7},
	{"Egrave",						0x0c8},
	{"Eacute",						0x0c9},
	{"Ecircumflex",					0x0ca},
	{"Ediaeresis",					0x0cb},
	{"Igrave",						0x0cc},
	{"Iacute",						0x0cd},
	{"Icircumflex",					0x0ce},
	{"Idiaeresis",					0x0cf},
	{"ETH",							0x0d0},
	{"Eth",							0x0d0},
	{"Ntilde",						0x0d1},
	{"Ograve",						0x0d2},
	{"Oacute",						0x0d3},
	{"Ocircumflex",					0x0d4},
	{"Otilde",						0x0d5},
	{"Odiaeresis",					0x0d6},
	{"multiply",					0x0d7},
	{"Ooblique",					0x0d8},
	{"Ugrave",						0x0d9},
	{"Uacute",						0x0da},
	{"Ucircumflex",					0x0db},
	{"Udiaeresis",					0x0dc},
	{"Yacute",						0x0dd},
	{"THORN",						0x0de},
	{"Thorn",						0x0de},
	{"ssharp",						0x0df},
	{"agrave",						0x0e0},
	{"aacute",						0x0e1},
	{"acircumflex",					0x0e2},
	{"atilde",						0x0e3},
	{"adiaeresis",					0x0e4},
	{"aring",						0x0e5},
	{"ae",							0x0e6},
	{"ccedilla",					0x0e7},
	{"egrave",						0x0e8},
	{"eacute",						0x0e9},
	{"ecircumflex",					0x0ea},
	{"ediaeresis",					0x0eb},
	{"igrave",						0x0ec},
	{"iacute",						0x0ed},
	{"icircumflex",					0x0ee},
	{"idiaeresis",					0x0ef},
	{"eth",							0x0f0},
	{"ntilde",						0x0f1},
	{"ograve",						0x0f2},
	{"oacute",						0x0f3},
	{"ocircumflex",					0x0f4},
	{"otilde",						0x0f5},
	{"odiaeresis",					0x0f6},
	{"division",					0x0f7},
	{"oslash",						0x0f8},
	{"ugrave",						0x0f9},
	{"uacute",						0x0fa},
	{"ucircumflex",					0x0fb},
	{"udiaeresis",					0x0fc},
	{"yacute",						0x0fd},
	{"thorn",						0x0fe},
	{"ydiaeresis",					0x0ff},
*/
	{"Aogonek",						0x1a1},
	{"breve",						0x1a2},
	{"Lstroke",						0x1a3},
	{"Lcaron",						0x1a5},
	{"Sacute",						0x1a6},
	{"Scaron",						0x1a9},
	{"Scedilla",					0x1aa},
	{"Tcaron",						0x1ab},
	{"Zacute",						0x1ac},
	{"Zcaron",						0x1ae},
	{"Zabovedot",					0x1af},
	{"aogonek",						0x1b1},
	{"ogonek",						0x1b2},
	{"lstroke",						0x1b3},
	{"lcaron",						0x1b5},
	{"sacute",						0x1b6},
	{"caron",						0x1b7},
	{"scaron",						0x1b9},
	{"scedilla",					0x1ba},
	{"tcaron",						0x1bb},
	{"zacute",						0x1bc},
	{"doubleacute",					0x1bd},
	{"zcaron",						0x1be},
	{"zabovedot",					0x1bf},
	{"Racute",						0x1c0},
	{"Abreve",						0x1c3},
	{"Lacute",						0x1c5},
	{"Cacute",						0x1c6},
	{"Ccaron",						0x1c8},
	{"Eogonek",						0x1ca},
	{"Ecaron",						0x1cc},
	{"Dcaron",						0x1cf},
	{"Dstroke",						0x1d0},
	{"Nacute",						0x1d1},
	{"Ncaron",						0x1d2},
	{"Odoubleacute",				0x1d5},
	{"Rcaron",						0x1d8},
	{"Uring",						0x1d9},
	{"Udoubleacute",				0x1db},
	{"Tcedilla",					0x1de},
	{"racute",						0x1e0},
	{"abreve",						0x1e3},
	{"lacute",						0x1e5},
	{"cacute",						0x1e6},
	{"ccaron",						0x1e8},
	{"eogonek",						0x1ea},
	{"ecaron",						0x1ec},
	{"dcaron",						0x1ef},
	{"dstroke",						0x1f0},
	{"nacute",						0x1f1},
	{"ncaron",						0x1f2},
	{"odoubleacute",				0x1f5},
	{"udoubleacute",				0x1fb},
	{"rcaron",						0x1f8},
	{"uring",						0x1f9},
	{"tcedilla",					0x1fe},
	{"abovedot",					0x1ff},


	{"Hstroke",						0x2a1},
	{"Hcircumflex",					0x2a6},
	{"Iabovedot",					0x2a9},
	{"Gbreve",						0x2ab},
	{"Jcircumflex",					0x2ac},
	{"hstroke",						0x2b1},
	{"hcircumflex",					0x2b6},
	{"idotless",					0x2b9},
	{"gbreve",						0x2bb},
	{"jcircumflex",					0x2bc},
	{"Cabovedot",					0x2c5},
	{"Ccircumflex",					0x2c6},
	{"Gabovedot",					0x2d5},
	{"Gcircumflex",					0x2d8},
	{"Ubreve",						0x2dd},
	{"Scircumflex",					0x2de},
	{"cabovedot",					0x2e5},
	{"ccircumflex",					0x2e6},
	{"gabovedot",					0x2f5},
	{"gcircumflex",					0x2f8},
	{"ubreve",						0x2fd},
	{"scircumflex",					0x2fe},


	{"kra",							0x3a2},
	{"kappa",						0x3a2},
	{"Rcedilla",					0x3a3},
	{"Itilde",						0x3a5},
	{"Lcedilla",					0x3a6},
	{"Emacron",						0x3aa},
	{"Gcedilla",					0x3ab},
	{"Tslash",						0x3ac},
	{"rcedilla",					0x3b3},
	{"itilde",						0x3b5},
	{"lcedilla",					0x3b6},
	{"emacron",						0x3ba},
	{"gcedilla",					0x3bb},
	{"tslash",						0x3bc},
	{"ENG",							0x3bd},
	{"eng",							0x3bf},
	{"Amacron",						0x3c0},
	{"Iogonek",						0x3c7},
	{"Eabovedot",					0x3cc},
	{"Imacron",						0x3cf},
	{"Ncedilla",					0x3d1},
	{"Omacron",						0x3d2},
	{"Kcedilla",					0x3d3},
	{"Uogonek",						0x3d9},
	{"Utilde",						0x3dd},
	{"Umacron",						0x3de},
	{"amacron",						0x3e0},
	{"iogonek",						0x3e7},
	{"eabovedot",					0x3ec},
	{"imacron",						0x3ef},
	{"ncedilla",					0x3f1},
	{"omacron",						0x3f2},
	{"kcedilla",					0x3f3},
	{"uogonek",						0x3f9},
	{"utilde",						0x3fd},
	{"umacron",						0x3fe},

	{"OE",							0x13bc},
	{"oe",							0x13bd},
	{"Ydiaeresis",					0x13be},

	{"overline",					0x47e},
	{"kana_fullstop",				0x4a1},
	{"kana_openingbracket",			0x4a2},
	{"kana_closingbracket",			0x4a3},
	{"kana_comma",					0x4a4},
	{"kana_conjunctive",			0x4a5},
	{"kana_middledot",				0x4a5},
	{"kana_WO",						0x4a6},
	{"kana_a",						0x4a7},
	{"kana_i",						0x4a8},
	{"kana_u",						0x4a9},
	{"kana_e",						0x4aa},
	{"kana_o",						0x4ab},
	{"kana_ya",						0x4ac},
	{"kana_yu",						0x4ad},
	{"kana_yo",						0x4ae},
	{"kana_tsu",					0x4af},
	{"kana_tu",						0x4af},
	{"prolongedsound",				0x4b0},
	{"kana_A",						0x4b1},
	{"kana_I",						0x4b2},
	{"kana_U",						0x4b3},
	{"kana_E",						0x4b4},
	{"kana_O",						0x4b5},
	{"kana_KA",						0x4b6},
	{"kana_KI",						0x4b7},
	{"kana_KU",						0x4b8},
	{"kana_KE",						0x4b9},
	{"kana_KO",						0x4ba},
	{"kana_SA",						0x4bb},
	{"kana_SHI",					0x4bc},
	{"kana_SU",						0x4bd},
	{"kana_SE",						0x4be},
	{"kana_SO",						0x4bf},
	{"kana_TA",						0x4c0},
	{"kana_CHI",					0x4c1},
	{"kana_TI",						0x4c1},
	{"kana_TSU",					0x4c2},
	{"kana_TU",						0x4c2},
	{"kana_TE",						0x4c3},
	{"kana_TO",						0x4c4},
	{"kana_NA",						0x4c5},
	{"kana_NI",						0x4c6},
	{"kana_NU",						0x4c7},
	{"kana_NE",						0x4c8},
	{"kana_NO",						0x4c9},
	{"kana_HA",						0x4ca},
	{"kana_HI",						0x4cb},
	{"kana_FU",						0x4cc},
	{"kana_HU",						0x4cc},
	{"kana_HE",						0x4cd},
	{"kana_HO",						0x4ce},
	{"kana_MA",						0x4cf},
	{"kana_MI",						0x4d0},
	{"kana_MU",						0x4d1},
	{"kana_ME",						0x4d2},
	{"kana_MO",						0x4d3},
	{"kana_YA",						0x4d4},
	{"kana_YU",						0x4d5},
	{"kana_YO",						0x4d6},
	{"kana_RA",						0x4d7},
	{"kana_RI",						0x4d8},
	{"kana_RU",						0x4d9},
	{"kana_RE",						0x4da},
	{"kana_RO",						0x4db},
	{"kana_WA",						0x4dc},
	{"kana_N",						0x4dd},
	{"voicedsound",					0x4de},
	{"semivoicedsound",				0x4df},
	{"kana_switch",					0xFF7E},

	{"Arabic_comma",				0x5ac},
	{"Arabic_semicolon",			0x5bb},
	{"Arabic_question_mark",		0x5bf},
	{"Arabic_hamza",				0x5c1},
	{"Arabic_maddaonalef",			0x5c2},
	{"Arabic_hamzaonalef",			0x5c3},
	{"Arabic_hamzaonwaw",			0x5c4},
	{"Arabic_hamzaunderalef",		0x5c5},
	{"Arabic_hamzaonyeh",			0x5c6},
	{"Arabic_alef",					0x5c7},
	{"Arabic_beh",					0x5c8},
	{"Arabic_tehmarbuta",			0x5c9},
	{"Arabic_teh",					0x5ca},
	{"Arabic_theh",					0x5cb},
	{"Arabic_jeem",					0x5cc},
	{"Arabic_hah",					0x5cd},
	{"Arabic_khah",					0x5ce},
	{"Arabic_dal",					0x5cf},
	{"Arabic_thal",					0x5d0},
	{"Arabic_ra",					0x5d1},
	{"Arabic_zain",					0x5d2},
	{"Arabic_seen",					0x5d3},
	{"Arabic_sheen",				0x5d4},
	{"Arabic_sad",					0x5d5},
	{"Arabic_dad",					0x5d6},
	{"Arabic_tah",					0x5d7},
	{"Arabic_zah",					0x5d8},
	{"Arabic_ain",					0x5d9},
	{"Arabic_ghain",				0x5da},
	{"Arabic_tatweel",				0x5e0},
	{"Arabic_feh",					0x5e1},
	{"Arabic_qaf",					0x5e2},
	{"Arabic_kaf",					0x5e3},
	{"Arabic_lam",					0x5e4},
	{"Arabic_meem",					0x5e5},
	{"Arabic_noon",					0x5e6},
	{"Arabic_ha",					0x5e7},
	{"Arabic_heh",					0x5e7},
	{"Arabic_waw",					0x5e8},
	{"Arabic_alefmaksura",			0x5e9},
	{"Arabic_yeh",					0x5ea},
	{"Arabic_fathatan",				0x5eb},
	{"Arabic_dammatan",				0x5ec},
	{"Arabic_kasratan",				0x5ed},
	{"Arabic_fatha",				0x5ee},
	{"Arabic_damma",				0x5ef},
	{"Arabic_kasra",				0x5f0},
	{"Arabic_shadda",				0x5f1},
	{"Arabic_sukun",				0x5f2},
	{"Arabic_switch",				0xFF7E},


	{"Serbian_dje",					0x6a1},
	{"Macedonia_gje",				0x6a2},
	{"Cyrillic_io",					0x6a3},
	{"Ukrainian_ie",				0x6a4},
	{"Ukranian_je",					0x6a4},
	{"Macedonia_dse",				0x6a5},
	{"Ukrainian_i",					0x6a6},
	{"Ukranian_i",					0x6a6},
	{"Ukrainian_yi",				0x6a7},
	{"Ukranian_yi",					0x6a7},
	{"Cyrillic_je",					0x6a8},
	{"Serbian_je",					0x6a8},
	{"Cyrillic_lje",				0x6a9},
	{"Serbian_lje",					0x6a9},
	{"Cyrillic_nje",				0x6aa},
	{"Serbian_nje",					0x6aa},
	{"Serbian_tshe",				0x6ab},
	{"Macedonia_kje",				0x6ac},
	{"Byelorussian_shortu",			0x6ae},
	{"Cyrillic_dzhe",				0x6af},
	{"Serbian_dze",					0x6af},
	{"numerosign",					0x6b0},
	{"Serbian_DJE",					0x6b1},
	{"Macedonia_GJE",				0x6b2},
	{"Cyrillic_IO",					0x6b3},
	{"Ukrainian_IE",				0x6b4},
	{"Ukranian_JE",					0x6b4},
	{"Macedonia_DSE",				0x6b5},
	{"Ukrainian_I",					0x6b6},
	{"Ukranian_I",					0x6b6},
	{"Ukrainian_YI",				0x6b7},
	{"Ukranian_YI",					0x6b7},
	{"Cyrillic_JE",					0x6b8},
	{"Serbian_JE",					0x6b8},
	{"Cyrillic_LJE",				0x6b9},
	{"Serbian_LJE",					0x6b9},
	{"Cyrillic_NJE",				0x6ba},
	{"Serbian_NJE",					0x6ba},
	{"Serbian_TSHE",				0x6bb},
	{"Macedonia_KJE",				0x6bc},
	{"Byelorussian_SHORTU",			0x6be},
	{"Cyrillic_DZHE",				0x6bf},
	{"Serbian_DZE",					0x6bf},
	{"Cyrillic_yu",					0x6c0},
	{"Cyrillic_a",					0x6c1},
	{"Cyrillic_be",					0x6c2},
	{"Cyrillic_tse",				0x6c3},
	{"Cyrillic_de",					0x6c4},
	{"Cyrillic_ie",					0x6c5},
	{"Cyrillic_ef",					0x6c6},
	{"Cyrillic_ghe",				0x6c7},
	{"Cyrillic_ha",					0x6c8},
	{"Cyrillic_i",					0x6c9},
	{"Cyrillic_shorti",				0x6ca},
	{"Cyrillic_ka",					0x6cb},
	{"Cyrillic_el",					0x6cc},
	{"Cyrillic_em",					0x6cd},
	{"Cyrillic_en",					0x6ce},
	{"Cyrillic_o",					0x6cf},
	{"Cyrillic_pe",					0x6d0},
	{"Cyrillic_ya",					0x6d1},
	{"Cyrillic_er",					0x6d2},
	{"Cyrillic_es",					0x6d3},
	{"Cyrillic_te",					0x6d4},
	{"Cyrillic_u",					0x6d5},
	{"Cyrillic_zhe",				0x6d6},
	{"Cyrillic_ve",					0x6d7},
	{"Cyrillic_softsign",			0x6d8},
	{"Cyrillic_yeru",				0x6d9},
	{"Cyrillic_ze",					0x6da},
	{"Cyrillic_sha",				0x6db},
	{"Cyrillic_e",					0x6dc},
	{"Cyrillic_shcha",				0x6dd},
	{"Cyrillic_che",				0x6de},
	{"Cyrillic_hardsign",			0x6df},
	{"Cyrillic_YU",					0x6e0},
	{"Cyrillic_A",					0x6e1},
	{"Cyrillic_BE",					0x6e2},
	{"Cyrillic_TSE",				0x6e3},
	{"Cyrillic_DE",					0x6e4},
	{"Cyrillic_IE",					0x6e5},
	{"Cyrillic_EF",					0x6e6},
	{"Cyrillic_GHE",				0x6e7},
	{"Cyrillic_HA",					0x6e8},
	{"Cyrillic_I",					0x6e9},
	{"Cyrillic_SHORTI",				0x6ea},
	{"Cyrillic_KA",					0x6eb},
	{"Cyrillic_EL",					0x6ec},
	{"Cyrillic_EM",					0x6ed},
	{"Cyrillic_EN",					0x6ee},
	{"Cyrillic_O",					0x6ef},
	{"Cyrillic_PE",					0x6f0},
	{"Cyrillic_YA",					0x6f1},
	{"Cyrillic_ER",					0x6f2},
	{"Cyrillic_ES",					0x6f3},
	{"Cyrillic_TE",					0x6f4},
	{"Cyrillic_U",					0x6f5},
	{"Cyrillic_ZHE",				0x6f6},
	{"Cyrillic_VE",					0x6f7},
	{"Cyrillic_SOFTSIGN",			0x6f8},
	{"Cyrillic_YERU",				0x6f9},
	{"Cyrillic_ZE",					0x6fa},
	{"Cyrillic_SHA",				0x6fb},
	{"Cyrillic_E",					0x6fc},
	{"Cyrillic_SHCHA",				0x6fd},
	{"Cyrillic_CHE",				0x6fe},
	{"Cyrillic_HARDSIGN",			0x6ff},

	{"Greek_ALPHAaccent",			0x7a1},
	{"Greek_EPSILONaccent",			0x7a2},
	{"Greek_ETAaccent",				0x7a3},
	{"Greek_IOTAaccent",			0x7a4},
	{"Greek_IOTAdiaeresis",			0x7a5},
	{"Greek_OMICRONaccent",			0x7a7},
	{"Greek_UPSILONaccent",			0x7a8},
	{"Greek_UPSILONdieresis",		0x7a9},
	{"Greek_OMEGAaccent",			0x7ab},
	{"Greek_accentdieresis",		0x7ae},
	{"Greek_horizbar",				0x7af},
	{"Greek_alphaaccent",			0x7b1},
	{"Greek_epsilonaccent",			0x7b2},
	{"Greek_etaaccent",				0x7b3},
	{"Greek_iotaaccent",			0x7b4},
	{"Greek_iotadieresis",			0x7b5},
	{"Greek_iotaaccentdieresis",	0x7b6},
	{"Greek_omicronaccent",			0x7b7},
	{"Greek_upsilonaccent",			0x7b8},
	{"Greek_upsilondieresis",		0x7b9},
	{"Greek_upsilonaccentdieresis",	0x7ba},
	{"Greek_omegaaccent",			0x7bb},
	{"Greek_ALPHA",					0x7c1},
	{"Greek_BETA",					0x7c2},
	{"Greek_GAMMA",					0x7c3},
	{"Greek_DELTA",					0x7c4},
	{"Greek_EPSILON",				0x7c5},
	{"Greek_ZETA",					0x7c6},
	{"Greek_ETA",					0x7c7},
	{"Greek_THETA",					0x7c8},
	{"Greek_IOTA",					0x7c9},
	{"Greek_KAPPA",					0x7ca},
	{"Greek_LAMDA",					0x7cb},
	{"Greek_LAMBDA",				0x7cb},
	{"Greek_MU",					0x7cc},
	{"Greek_NU",					0x7cd},
	{"Greek_XI",					0x7ce},
	{"Greek_OMICRON",				0x7cf},
	{"Greek_PI",					0x7d0},
	{"Greek_RHO",					0x7d1},
	{"Greek_SIGMA",					0x7d2},
	{"Greek_TAU",					0x7d4},
	{"Greek_UPSILON",				0x7d5},
	{"Greek_PHI",					0x7d6},
	{"Greek_CHI",					0x7d7},
	{"Greek_PSI",					0x7d8},
	{"Greek_OMEGA",					0x7d9},
	{"Greek_alpha",					0x7e1},
	{"Greek_beta",					0x7e2},
	{"Greek_gamma",					0x7e3},
	{"Greek_delta",					0x7e4},
	{"Greek_epsilon",				0x7e5},
	{"Greek_zeta",					0x7e6},
	{"Greek_eta",					0x7e7},
	{"Greek_theta",					0x7e8},
	{"Greek_iota",					0x7e9},
	{"Greek_kappa",					0x7ea},
	{"Greek_lamda",					0x7eb},
	{"Greek_lambda",				0x7eb},
	{"Greek_mu",					0x7ec},
	{"Greek_nu",					0x7ed},
	{"Greek_xi",					0x7ee},
	{"Greek_omicron",				0x7ef},
	{"Greek_pi",					0x7f0},
	{"Greek_rho",					0x7f1},
	{"Greek_sigma",					0x7f2},
	{"Greek_finalsmallsigma",		0x7f3},
	{"Greek_tau",					0x7f4},
	{"Greek_upsilon",				0x7f5},
	{"Greek_phi",					0x7f6},
	{"Greek_chi",					0x7f7},
	{"Greek_psi",					0x7f8},
	{"Greek_omega",					0x7f9},
	{"Greek_switch",				0xFF7E},

	{"leftradical",					0x8a1},
	{"topleftradical",				0x8a2},
	{"horizconnector",				0x8a3},
	{"topintegral",					0x8a4},
	{"botintegral",					0x8a5},
	{"vertconnector",				0x8a6},
	{"topleftsqbracket",			0x8a7},
	{"botleftsqbracket",			0x8a8},
	{"toprightsqbracket",			0x8a9},
	{"botrightsqbracket",			0x8aa},
	{"topleftparens",				0x8ab},
	{"botleftparens",				0x8ac},
	{"toprightparens",				0x8ad},
	{"botrightparens",				0x8ae},
	{"leftmiddlecurlybrace",		0x8af},
	{"rightmiddlecurlybrace",		0x8b0},
	{"topleftsummation",			0x8b1},
	{"botleftsummation",			0x8b2},
	{"topvertsummationconnector",	0x8b3},
	{"botvertsummationconnector",	0x8b4},
	{"toprightsummation",			0x8b5},
	{"botrightsummation",			0x8b6},
	{"rightmiddlesummation",		0x8b7},
	{"lessthanequal",				0x8bc},
	{"notequal",					0x8bd},
	{"greaterthanequal",			0x8be},
	{"integral",					0x8bf},
	{"therefore",					0x8c0},
	{"variation",					0x8c1},
	{"infinity",					0x8c2},
	{"nabla",						0x8c5},
	{"approximate",					0x8c8},
	{"similarequal",				0x8c9},
	{"ifonlyif",					0x8cd},
	{"implies",						0x8ce},
	{"identical",					0x8cf},
	{"radical",						0x8d6},
	{"includedin",					0x8da},
	{"includes",					0x8db},
	{"intersection",				0x8dc},
	{"union",						0x8dd},
	{"logicaland",					0x8de},
	{"logicalor",					0x8df},
	{"partialderivative",			0x8ef},
	{"function",					0x8f6},
	{"leftarrow",					0x8fb},
	{"uparrow",						0x8fc},
	{"rightarrow",					0x8fd},
	{"downarrow",					0x8fe},


	{"blank",						0x9df},
	{"soliddiamond",				0x9e0},
	{"checkerboard",				0x9e1},
	{"ht",							0x9e2},
	{"ff",							0x9e3},
/*	{"cr",							0x9e4},*/
	{"lf",							0x9e5},
	{"nl",							0x9e8},
	{"vt",							0x9e9},
	{"lowrightcorner",				0x9ea},
	{"uprightcorner",				0x9eb},
	{"upleftcorner",				0x9ec},
	{"lowleftcorner",				0x9ed},
	{"crossinglines",				0x9ee},
	{"horizlinescan1",				0x9ef},
	{"horizlinescan3",				0x9f0},
	{"horizlinescan5",				0x9f1},
	{"horizlinescan7",				0x9f2},
	{"horizlinescan9",				0x9f3},
	{"leftt",						0x9f4},
	{"rightt",						0x9f5},
	{"bott",						0x9f6},
	{"topt",						0x9f7},
	{"vertbar",						0x9f8},

	{"emspace",						0xaa1},
	{"enspace",						0xaa2},
	{"em3space",					0xaa3},
	{"em4space",					0xaa4},
	{"digitspace",					0xaa5},
	{"punctspace",					0xaa6},
	{"thinspace",					0xaa7},
	{"hairspace",					0xaa8},
	{"emdash",						0xaa9},
	{"endash",						0xaaa},
	{"signifblank",					0xaac},
	{"ellipsis",					0xaae},
	{"doubbaselinedot",				0xaaf},
	{"onethird",					0xab0},
	{"twothirds",					0xab1},
	{"onefifth",					0xab2},
	{"twofifths",					0xab3},
	{"threefifths",					0xab4},
	{"fourfifths",					0xab5},
	{"onesixth",					0xab6},
	{"fivesixths",					0xab7},
	{"careof",						0xab8},
	{"figdash",						0xabb},
	{"leftanglebracket",			0xabc},
	{"decimalpoint",				0xabd},
	{"rightanglebracket",			0xabe},
	{"marker",						0xabf},
	{"oneeighth",					0xac3},
	{"threeeighths",				0xac4},
	{"fiveeighths",					0xac5},
	{"seveneighths",				0xac6},
	{"trademark",					0xac9},
	{"signaturemark",				0xaca},
	{"trademarkincircle",			0xacb},
	{"leftopentriangle",			0xacc},
	{"rightopentriangle",			0xacd},
	{"emopencircle",				0xace},
	{"emopenrectangle",				0xacf},
	{"leftsinglequotemark",			0xad0},
	{"rightsinglequotemark",		0xad1},
	{"leftdoublequotemark",			0xad2},
	{"rightdoublequotemark",		0xad3},
	{"prescription",				0xad4},
	{"minutes",						0xad6},
	{"seconds",						0xad7},
	{"latincross",					0xad9},
	{"hexagram",					0xada},
	{"filledrectbullet",			0xadb},
	{"filledlefttribullet",			0xadc},
	{"filledrighttribullet",		0xadd},
	{"emfilledcircle",				0xade},
	{"emfilledrect",				0xadf},
	{"enopencircbullet",			0xae0},
	{"enopensquarebullet",			0xae1},
	{"openrectbullet",				0xae2},
	{"opentribulletup",				0xae3},
	{"opentribulletdown",			0xae4},
	{"openstar",					0xae5},
	{"enfilledcircbullet",			0xae6},
	{"enfilledsqbullet",			0xae7},
	{"filledtribulletup",			0xae8},
	{"filledtribulletdown",			0xae9},
	{"leftpointer",					0xaea},
	{"rightpointer",				0xaeb},
	{"club",						0xaec},
	{"diamond",						0xaed},
	{"heart",						0xaee},
	{"maltesecross",				0xaf0},
	{"dagger",						0xaf1},
	{"doubledagger",				0xaf2},
	{"checkmark",					0xaf3},
	{"ballotcross",					0xaf4},
	{"musicalsharp",				0xaf5},
	{"musicalflat",					0xaf6},
	{"malesymbol",					0xaf7},
	{"femalesymbol",				0xaf8},
	{"telephone",					0xaf9},
	{"telephonerecorder",			0xafa},
	{"phonographcopyright",			0xafb},
	{"caret",						0xafc},
	{"singlelowquotemark",			0xafd},
	{"doublelowquotemark",			0xafe},
	{"cursor",						0xaff},


	{"leftcaret",					0xba3},
	{"rightcaret",					0xba6},
	{"downcaret",					0xba8},
	{"upcaret",						0xba9},
	{"overbar",						0xbc0},
	{"downtack",					0xbc2},
	{"upshoe",						0xbc3},
	{"downstile",					0xbc4},
	{"underbar",					0xbc6},
	{"jot",							0xbca},
	{"quad",						0xbcc},
	{"uptack",						0xbce},
	{"circle",						0xbcf},
	{"upstile",						0xbd3},
	{"downshoe",					0xbd6},
	{"rightshoe",					0xbd8},
	{"leftshoe",					0xbda},
	{"lefttack",					0xbdc},
	{"righttack",					0xbfc},

	{"hebrew_doublelowline",		0xcdf},
	{"hebrew_aleph",				0xce0},
	{"hebrew_bet",					0xce1},
	{"hebrew_beth",					0xce1},
	{"hebrew_gimel",				0xce2},
	{"hebrew_gimmel",				0xce2},
	{"hebrew_dalet",				0xce3},
	{"hebrew_daleth",				0xce3},
	{"hebrew_he",					0xce4},
	{"hebrew_waw",					0xce5},
	{"hebrew_zain",					0xce6},
	{"hebrew_zayin",				0xce6},
	{"hebrew_chet",					0xce7},
	{"hebrew_het",					0xce7},
	{"hebrew_tet",					0xce8},
	{"hebrew_teth",					0xce8},
	{"hebrew_yod",					0xce9},
	{"hebrew_kaph",					0xceb},
	{"hebrew_lamed",				0xcec},
	{"hebrew_finalkaph",			0xcea},
	{"hebrew_finalmem",				0xced},
	{"hebrew_mem",					0xcee},
	{"hebrew_finalnun",				0xcef},
	{"hebrew_nun",					0xcf0},
	{"hebrew_samech",				0xcf1},
	{"hebrew_samekh",				0xcf1},
	{"hebrew_ayin",					0xcf2},
	{"hebrew_finalpe",				0xcf3},
	{"hebrew_pe",					0xcf4},
	{"hebrew_finalzade",			0xcf5},
	{"hebrew_finalzadi",			0xcf5},
	{"hebrew_zade",					0xcf6},
	{"hebrew_zadi",					0xcf6},
	{"hebrew_qoph",					0xcf7},
	{"hebrew_kuf",					0xcf7},
	{"hebrew_resh",					0xcf8},
	{"hebrew_shin",					0xcf9},
	{"hebrew_taw",					0xcfa},
	{"hebrew_taf",					0xcfa},
	{"Hebrew_switch",				0xFF7E},

	{"Thai_kokai",					0xda1},
	{"Thai_khokhai",				0xda2},
	{"Thai_khokhuat",				0xda3},
	{"Thai_khokhwai",				0xda4},
	{"Thai_khokhon",				0xda5},
	{"Thai_khorakhang",				0xda6},
	{"Thai_ngongu",					0xda7},
	{"Thai_chochan",				0xda8},
	{"Thai_choching",				0xda9},
	{"Thai_chochang",				0xdaa},
	{"Thai_soso",					0xdab},
	{"Thai_chochoe",				0xdac},
	{"Thai_yoying",					0xdad},
	{"Thai_dochada",				0xdae},
	{"Thai_topatak",				0xdaf},
	{"Thai_thothan",				0xdb0},
	{"Thai_thonangmontho",			0xdb1},
	{"Thai_thophuthao",				0xdb2},
	{"Thai_nonen",					0xdb3},
	{"Thai_dodek",					0xdb4},
	{"Thai_totao",					0xdb5},
	{"Thai_thothung",				0xdb6},
	{"Thai_thothahan",				0xdb7},
	{"Thai_thothong",				0xdb8},
	{"Thai_nonu",					0xdb9},
	{"Thai_bobaimai",				0xdba},
	{"Thai_popla",					0xdbb},
	{"Thai_phophung",				0xdbc},
	{"Thai_fofa",					0xdbd},
	{"Thai_phophan",				0xdbe},
	{"Thai_fofan",					0xdbf},
	{"Thai_phosamphao",				0xdc0},
	{"Thai_moma",					0xdc1},
	{"Thai_yoyak",					0xdc2},
	{"Thai_rorua",					0xdc3},
	{"Thai_ru",						0xdc4},
	{"Thai_loling",					0xdc5},
	{"Thai_lu",						0xdc6},
	{"Thai_wowaen",					0xdc7},
	{"Thai_sosala",					0xdc8},
	{"Thai_sorusi",					0xdc9},
	{"Thai_sosua",					0xdca},
	{"Thai_hohip",					0xdcb},
	{"Thai_lochula",				0xdcc},
	{"Thai_oang",					0xdcd},
	{"Thai_honokhuk",				0xdce},
	{"Thai_paiyannoi",				0xdcf},
	{"Thai_saraa",					0xdd0},
	{"Thai_maihanakat",				0xdd1},
	{"Thai_saraaa",					0xdd2},
	{"Thai_saraam",					0xdd3},
	{"Thai_sarai",					0xdd4},
	{"Thai_saraii",					0xdd5},
	{"Thai_saraue",					0xdd6},
	{"Thai_sarauee",				0xdd7},
	{"Thai_sarau",					0xdd8},
	{"Thai_sarauu",					0xdd9},
	{"Thai_phinthu",				0xdda},
	{"Thai_maihanakat_maitho",		0xdde},
	{"Thai_baht",					0xddf},
	{"Thai_sarae",					0xde0},
	{"Thai_saraae",					0xde1},
	{"Thai_sarao",					0xde2},
	{"Thai_saraaimaimuan",			0xde3},
	{"Thai_saraaimaimalai",			0xde4},
	{"Thai_lakkhangyao",			0xde5},
	{"Thai_maiyamok",				0xde6},
	{"Thai_maitaikhu",				0xde7},
	{"Thai_maiek",					0xde8},
	{"Thai_maitho",					0xde9},
	{"Thai_maitri",					0xdea},
	{"Thai_maichattawa",			0xdeb},
	{"Thai_thanthakhat",			0xdec},
	{"Thai_nikhahit",				0xded},
	{"Thai_leksun",					0xdf0},
	{"Thai_leknung",				0xdf1},
	{"Thai_leksong",				0xdf2},
	{"Thai_leksam",					0xdf3},
	{"Thai_leksi",					0xdf4},
	{"Thai_lekha",					0xdf5},
	{"Thai_lekhok",					0xdf6},
	{"Thai_lekchet",				0xdf7},
	{"Thai_lekpaet",				0xdf8},
	{"Thai_lekkao",					0xdf9},

	{"Hangul",						0xff31},
	{"Hangul_Start",				0xff32},
	{"Hangul_End",					0xff33},
	{"Hangul_Hanja",				0xff34},
	{"Hangul_Jamo",					0xff35},
	{"Hangul_Romaja",				0xff36},
	{"Hangul_Codeinput",			0xff37},
	{"Hangul_Jeonja",				0xff38},
	{"Hangul_Banja",				0xff39},
	{"Hangul_PreHanja",				0xff3a},
	{"Hangul_PostHanja",			0xff3b},
	{"Hangul_SingleCandidate",		0xff3c},
	{"Hangul_MultipleCandidate",	0xff3d},
	{"Hangul_PreviousCandidate",	0xff3e},
	{"Hangul_Special",				0xff3f},
	{"Hangul_switch",				0xFF7E},

	{"Hangul_Kiyeog",				0xea1},
	{"Hangul_SsangKiyeog",			0xea2},
	{"Hangul_KiyeogSios",			0xea3},
	{"Hangul_Nieun",				0xea4},
	{"Hangul_NieunJieuj",			0xea5},
	{"Hangul_NieunHieuh",			0xea6},
	{"Hangul_Dikeud",				0xea7},
	{"Hangul_SsangDikeud",			0xea8},
	{"Hangul_Rieul",				0xea9},
	{"Hangul_RieulKiyeog",			0xeaa},
	{"Hangul_RieulMieum",			0xeab},
	{"Hangul_RieulPieub",			0xeac},
	{"Hangul_RieulSios",			0xead},
	{"Hangul_RieulTieut",			0xeae},
	{"Hangul_RieulPhieuf",			0xeaf},
	{"Hangul_RieulHieuh",			0xeb0},
	{"Hangul_Mieum",				0xeb1},
	{"Hangul_Pieub",				0xeb2},
	{"Hangul_SsangPieub",			0xeb3},
	{"Hangul_PieubSios",			0xeb4},
	{"Hangul_Sios",					0xeb5},
	{"Hangul_SsangSios",			0xeb6},
	{"Hangul_Ieung",				0xeb7},
	{"Hangul_Jieuj",				0xeb8},
	{"Hangul_SsangJieuj",			0xeb9},
	{"Hangul_Cieuc",				0xeba},
	{"Hangul_Khieuq",				0xebb},
	{"Hangul_Tieut",				0xebc},
	{"Hangul_Phieuf",				0xebd},
	{"Hangul_Hieuh",				0xebe},

	{"Hangul_A",					0xebf},
	{"Hangul_AE",					0xec0},
	{"Hangul_YA",					0xec1},
	{"Hangul_YAE",					0xec2},
	{"Hangul_EO",					0xec3},
	{"Hangul_E",					0xec4},
	{"Hangul_YEO",					0xec5},
	{"Hangul_YE",					0xec6},
	{"Hangul_O",					0xec7},
	{"Hangul_WA",					0xec8},
	{"Hangul_WAE",					0xec9},
	{"Hangul_OE",					0xeca},
	{"Hangul_YO",					0xecb},
	{"Hangul_U",					0xecc},
	{"Hangul_WEO",					0xecd},
	{"Hangul_WE",					0xece},
	{"Hangul_WI",					0xecf},
	{"Hangul_YU",					0xed0},
	{"Hangul_EU",					0xed1},
	{"Hangul_YI",					0xed2},
	{"Hangul_I",					0xed3},

	{"Hangul_J_Kiyeog",				0xed4},
	{"Hangul_J_SsangKiyeog",		0xed5},
	{"Hangul_J_KiyeogSios",			0xed6},
	{"Hangul_J_Nieun",				0xed7},
	{"Hangul_J_NieunJieuj",			0xed8},
	{"Hangul_J_NieunHieuh",			0xed9},
	{"Hangul_J_Dikeud",				0xeda},
	{"Hangul_J_Rieul",				0xedb},
	{"Hangul_J_RieulKiyeog",		0xedc},
	{"Hangul_J_RieulMieum",			0xedd},
	{"Hangul_J_RieulPieub",			0xede},
	{"Hangul_J_RieulSios",			0xedf},
	{"Hangul_J_RieulTieut",			0xee0},
	{"Hangul_J_RieulPhieuf",		0xee1},
	{"Hangul_J_RieulHieuh",			0xee2},
	{"Hangul_J_Mieum",				0xee3},
	{"Hangul_J_Pieub",				0xee4},
	{"Hangul_J_PieubSios",			0xee5},
	{"Hangul_J_Sios",				0xee6},
	{"Hangul_J_SsangSios",			0xee7},
	{"Hangul_J_Ieung",				0xee8},
	{"Hangul_J_Jieuj",				0xee9},
	{"Hangul_J_Cieuc",				0xeea},
	{"Hangul_J_Khieuq",				0xeeb},
	{"Hangul_J_Tieut",				0xeec},
	{"Hangul_J_Phieuf",				0xeed},
	{"Hangul_J_Hieuh",				0xeee},

	{"Hangul_RieulYeorinHieuh",		0xeef},
	{"Hangul_SunkyeongeumMieum",	0xef0},
	{"Hangul_SunkyeongeumPieub",	0xef1},
	{"Hangul_PanSios",				0xef2},
	{"Hangul_KkogjiDalrinIeung",	0xef3},
	{"Hangul_SunkyeongeumPhieuf",	0xef4},
	{"Hangul_YeorinHieuh",			0xef5},

	{"Hangul_AraeA",				0xef6},
	{"Hangul_AraeAE",				0xef7},

	{"Hangul_J_PanSios",			0xef8},
	{"Hangul_J_KkogjiDalrinIeung",	0xef9},
	{"Hangul_J_YeorinHieuh",		0xefa},

	{"Korean_Won",					0xeff},


	{"EcuSign",						0x20a0},
	{"ColonSign",					0x20a1},
	{"CruzeiroSign",				0x20a2},
	{"FFrancSign",					0x20a3},
	{"LiraSign",					0x20a4},
	{"MillSign",					0x20a5},
	{"NairaSign",					0x20a6},
	{"PesetaSign",					0x20a7},
	{"RupeeSign",					0x20a8},
	{"WonSign",						0x20a9},
	{"NewSheqelSign",				0x20aa},
	{"DongSign",					0x20ab},
	{"EuroSign",					0x20ac},

	{NULL,							0}
};

int keysdef_getcode(const char *s, int k[], int num)
{
	int i;
	int n, ln;
	char *p;

	for(n = 0 ; n < num && *s != '\0' ; ++n) {
		k[n] = 0;
		switch(*s) {
		case '^':
			 k[n] = KS_ctrl;
			 ++s;
			 break;
		case '\\':
			 k[n] = KS_shift;
			 ++s;
			 break;
		case '@':
			 k[n] = KS_alt;
			 ++s;
		}
		if(*s == '\0') {
			break;
		}
		if(*s != '[') {
			k[n] = (k[n] == KS_ctrl) ? *s & 0x1f : *s;
			++s;
			continue;
		}

		++s;
		p = strchr(s, ']');
		ln = p - s;

		for(i = 0 ; ; ++i) {
			if(keysdef[i].name == NULL) {
				return 0;
			}
			if(strncasecmp(keysdef[i].name, s, ln) == 0) {
				k[n] |= keysdef[i].scode;
				s = p + 1;
				break;
			}
		}
	}
	for(i = n ; i < num ; ++i) {
		k[i] = -1;
	}
	return n;
}

void keys_set(const char *k, const char *e, const char *d)
{
	int n;

	if(keysdef_getcode(k, &n, 1) == 0) {
		return;
	}
	term_escset(n, e, d);
}

int term_escdefault_cmp(const void *v, const void *w)
{
	esc_t *kp_v, *kp_w;

	return strcmp(((esc_t *)v)->seq, ((esc_t *)w)->seq);
}

void config_read(char *path);

void term_escdefault()
{
	if(esc_num == 0) {
		char *p, buf[1024 + 1];

		keys_set("[UP]",        "&ku", "\033OA");
		keys_set("[DOWN]",      "&kd", "\033OB");
		keys_set("[RIGHT]",     "&kr", "\033OC");
		keys_set("[LEFT]",      "&kl", "\033OD");
		keys_set("[HOME]",      "&kh", "\033[4~");
		keys_set("[BS]",        "&kb", "\010");
		keys_set("[DEL]",       "&kD", "\177");
		keys_set("[INS]",       "&kI", "\033[2~");
		keys_set("[Page_Up]",   "&kP", "\033[5~");
		keys_set("[Page_Down]", "&kN", "\033[6~");
		keys_set("[END]",       "&@7", "\033[1~");
		//keys_set("[CR]",        "&@8","\033[9~");

		keys_set("[HOME]",      "\033[4~", "");
		keys_set("[END]",       "\033[1~", "");

		keys_set("[F01]",       "\033[11~", "");
		keys_set("[F02]",       "\033[12~", "");
		keys_set("[F03]",       "\033[13~", "");
		keys_set("[F04]",       "\033[14~", "");
		keys_set("[F11]",       "\033[23~", "");
		keys_set("[F12]",       "\033[24~", "");

		keys_set("[F01]",       "&k1", "\033[11~");
		keys_set("[F02]",       "&k2", "\033[12~");
		keys_set("[F03]",       "&k3", "\033[13~");
		keys_set("[F04]",       "&k4", "\033[14~");
		keys_set("[F05]",       "&k5", "\033[15~");
		keys_set("[F06]",       "&k6", "\033[17~");
		keys_set("[F07]",       "&k7", "\033[18~");
		keys_set("[F08]",       "&k8", "\033[19~");
		keys_set("[F09]",       "&k9", "\033[20~");
		keys_set("[F10]",       "&k0", "\033[21~");

		keys_set("[F01]",       "&l1", "\033OP");
		keys_set("[F02]",       "&l2", "\033OQ");
		keys_set("[F03]",       "&l3", "\033OR");
		keys_set("[F04]",       "&l4", "\033OS");
		keys_set("[F05]",       "&l5", "\033OT");
		keys_set("[F06]",       "&l6", "\033OU");
		keys_set("[F07]",       "&l7", "\033OV");
		keys_set("[F08]",       "&l8", "\033OW");
		keys_set("[F09]",       "&l9", "\033OX");
		keys_set("[F10]",       "&l0", "\033OY");

		keys_set("\\[F01]",     "\033O2P", "");
		keys_set("\\[F02]",     "\033O2Q", "");
		keys_set("\\[F03]",     "\033O2R", "");
		keys_set("\\[F04]",     "\033O2S", "");

		keys_set("\\[F01]",     "\033[1;2P", "");
		keys_set("\\[F02]",     "\033[1;2Q", "");
		keys_set("\\[F03]",     "\033[1;2R", "");
		keys_set("\\[F04]",     "\033[1;2S", "");
		keys_set("\\[F05]",     "\033[15;2~", "");
		keys_set("\\[F06]",     "\033[17;2~", "");
		keys_set("\\[F07]",     "\033[18;2~", "");
		keys_set("\\[F08]",     "\033[19;2~", "");
		keys_set("\\[F09]",     "\033[20;2~", "");
		keys_set("\\[F10]",     "\033[21;2~", "");

		keys_set("^[F01]",      "\033O5P", "");
		keys_set("^[F02]",      "\033O5Q", "");
		keys_set("^[F03]",      "\033O5R", "");
		keys_set("^[F04]",      "\033O5S", "");

		keys_set("^[F01]",      "\033[1;5P", "");
		keys_set("^[F02]",      "\033[1;5Q", "");
		keys_set("^[F03]",      "\033[1;5R", "");
		keys_set("^[F04]",      "\033[1;5S", "");
		keys_set("^[F05]",      "\033[15;5~", "");
		keys_set("^[F06]",      "\033[17;5~", "");
		keys_set("^[F07]",      "\033[18;5~", "");
		keys_set("^[F08]",      "\033[19;5~", "");
		keys_set("^[F09]",      "\033[20;5~", "");
		keys_set("^[F10]",      "\033[21;5~", "");

		keys_set("@[F01]",      "\033O3P", "");
		keys_set("@[F02]",      "\033O3Q", "");
		keys_set("@[F03]",      "\033O3R", "");
		keys_set("@[F04]",      "\033O3S", "");

		keys_set("@[F01]",      "\033[1;3P", "");
		keys_set("@[F02]",      "\033[1;3Q", "");
		keys_set("@[F03]",      "\033[1;3R", "");
		keys_set("@[F04]",      "\033[1;3S", "");
		keys_set("@[F05]",      "\033[15;3~", "");
		keys_set("@[F06]",      "\033[17;3~", "");
		keys_set("@[F07]",      "\033[18;3~", "");
		keys_set("@[F08]",      "\033[19;3~", "");
		keys_set("@[F09]",      "\033[20;3~", "");
		keys_set("@[F10]",      "\033[21;3~", "");

		keys_set("*",           "\033Oj", "");
		keys_set("+",           "\033Ok", "");
		keys_set(",",           "\033Ol", "");
		keys_set("-",           "\033Om", "");
		keys_set(".",           "\033On", "");
		keys_set("/",           "\033Oo", "");
		keys_set("0",           "\033Op", "");
		keys_set("1",           "\033Oq", "");
		keys_set("2",           "\033Or", "");
		keys_set("3",           "\033Os", "");
		keys_set("4",           "\033Ot", "");
		keys_set("5",           "\033Ou", "");
		keys_set("6",           "\033Ov", "");
		keys_set("7",           "\033Ow", "");
		keys_set("8",           "\033Ox", "");
		keys_set("9",           "\033Oy", "");
		//keys_set("[CR]",        "\033OM", "");

		keys_set("\\[UP]",      "\033[1;2A", "");
		keys_set("\\[DOWN]",    "\033[1;2B", "");
		keys_set("\\[RIGHT]",   "\033[1;2C", "");
		keys_set("\\[LEFT]",    "\033[1;2D", "");
		keys_set("^[UP]",       "\033[1;5A", "");
		keys_set("^[DOWN]",     "\033[1;5B", "");
		keys_set("^[RIGHT]",    "\033[1;5C", "");
		keys_set("^[LEFT]",     "\033[1;5D", "");
		keys_set("@[UP]",       "\033[1;3A", "");
		keys_set("@[DOWN]",     "\033[1;3B", "");
		keys_set("@[RIGHT]",    "\033[1;3C", "");
		keys_set("@[LEFT]",     "\033[1;3D", "");

		keys_set("\\[Page_Up]",  "\033[5;2~", "");
		keys_set("\\[Page_Down]","\033[6;2~", "");
		keys_set("^[Page_Up]",   "\033[5;5~", "");
		keys_set("^[Page_Down]", "\033[6;5~", "");
		keys_set("@[Page_Up]",   "\033[5;3~", "");
		keys_set("@[Page_Down]", "\033[6;3~", "");

		keys_set("\\[HOME]",    "\033[1;2H", "");
		keys_set("\\[END]",     "\033[1;2F", "");
		keys_set("\\[INS]",     "\033[2;2~", "");
		keys_set("\\[DEL]",     "\033[3;2~", "");
		keys_set("^[HOME]",     "\033[1;5H", "");
		keys_set("^[END]",      "\033[1;5F", "");
		keys_set("^[INS]",      "\033[2;5~", "");
		keys_set("^[DEL]",      "\033[3;5~", "");
		keys_set("@[HOME]",     "\033[1;3H", "");
		keys_set("@[END]",      "\033[1;3F", "");
		keys_set("@[INS]",      "\033[2;3~", "");
		keys_set("@[DEL]",      "\033[3;3~", "");

		p = getenv("N8_TERM");
		if(p != NULL) {
			sprintf(buf, "term.%s", p);
		} else {
			 p = getenv("TERM");
			 if(p != NULL) {
			 	sprintf(buf, "term.%s", p);
			}
		}
		if(p != NULL) {
			config_read(buf);
		}
	}
	qsort(esc, esc_num, sizeof(esc_t), term_escdefault_cmp);
}

