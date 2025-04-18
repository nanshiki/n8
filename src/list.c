/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:list.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 

	n8
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#include "n8.h"
#include "list.h"
#include "file.h"
#include "cursor.h"
#include <ctype.h>

/*Base Pointer*/
static EditLine BaseLine[MAX_edbuf];
static EditLine *LastLine[MAX_edbuf];
static long LastOffset[MAX_edbuf];
static HistoryData BaseHistory[HISTORY_MAX];
static HistoryData *LastHistory[HISTORY_MAX];
static long LastCount[HISTORY_MAX];

long GetTopNumber()
{
	return 1;
}

long GetLastNumber()
{
	return LastOffset[CurrentFileNo];
}

EditLine *GetTop()
{
	return &BaseLine[CurrentFileNo];
}

EditLine *GetLast()
{
	return LastLine[CurrentFileNo];
}

void lists_debug()
{
	EditLine *ed;

	ed = GetTop();

	fprintf(stderr, "byte/size buffer\n");
	while(ed != NULL) {
		fprintf(stderr, "%4ld/%4ld %p[%s]\n", ed->bytes, ed->size, ed->buffer, ed->buffer);
		ed = ed->next;
	}
	fprintf(stderr, "***\n");
}

void lists_init()
{
	int i;

	for(i = 0 ; i < MAX_edbuf ; ++i) {
		BaseLine[i].prev = NULL;
		BaseLine[i].next = NULL;
		LastLine[i] = &BaseLine[i];
		LastOffset[i] = 0;
	}
	for(i = 0 ; i < HISTORY_MAX ; ++i) {
		BaseHistory[i].prev = NULL;
		BaseHistory[i].next = NULL;
		BaseHistory[i].buffer = NULL;
		LastHistory[i] = &BaseHistory[i];
		LastCount[i] = 0;
	}
}

void lists_clear()
{
	EditLine *p, *q;

	p = BaseLine[CurrentFileNo].next;
	while(p != NULL) {
		q = p;
		p = p->next;

 		q->next = NULL;
		q->prev = NULL;
		free(q->buffer);
		free(q);
	}

	BaseLine[CurrentFileNo].prev = NULL;
	BaseLine[CurrentFileNo].next = NULL;
	LastLine[CurrentFileNo] = &BaseLine[CurrentFileNo];
	LastOffset[CurrentFileNo] = 0;

	csrse.bytes = 0;
}

EditLine *MakeLine(const char *buffer)
{
	EditLine *pli;
	int n;

	pli = (EditLine *)mem_alloc(sizeof(EditLine));

	n = strlen(buffer);
	pli->buffer = (char *)mem_alloc(sizeof(char)*(n + 1));
	strcpy(pli->buffer, buffer);
	pli->size = n;
	pli->bytes = n;
	return pli;
}

void Realloc(EditLine *li, const char *s)
{
	char *p;
	size_t n;

	n = strlen(s);
	if(n > li->size) {
		li->buffer = (char *)mem_realloc(li->buffer, n + 1);
		li->size = n;
	}
	strcpy(li->buffer, s);
	csrse.bytes = csrse.bytes + n - li->bytes;
	li->bytes = strlen(s);
}

void AppendLast(EditLine *li)
{
	EditLine *last;

	last = GetLast();
	last->next = li;
	li->prev = last;
	li->next = NULL;
	++LastOffset[CurrentFileNo];
	LastLine[CurrentFileNo] = li;

	csrse.bytes += li->bytes;
}

void InsertLine(EditLine *bli, EditLine *li)
{
	if(bli->next == NULL) {
		AppendLast(li);
		return;
	}

	bli->next->prev = li;
	li->prev = bli;
	li->next = bli->next;
	bli->next = li;

	LastOffset[CurrentFileNo]++;
	csrse.bytes += li->bytes;
}

void DeleteList(EditLine *li)
{
	if(li->next != NULL) {
		li->next->prev = li->prev;
	} else {
		LastLine[CurrentFileNo] = li->prev;
	}

	csrse.bytes -= li->bytes;
	li->prev->next = li->next;
	free(li->buffer);
	free(li);
	LastOffset[CurrentFileNo]--;
}


EditLine *GetList(long o_number)
{
	long num, off;
	EditLine *p;

	off = o_number - GetLineOffset();

	if(o_number <= labs(off)) {
failed:
		num = 0;
		p = GetTop();

		while(p->next != NULL && num++ < o_number) {
			p = p->next;
		}
		return p;
	}

	if(GetLastNumber() - o_number <= labs(off)) {
		num = GetLastNumber();
		p = GetLast();

		if(p == NULL) {
			goto failed; // あってはならない。
		}
		while(p->prev != NULL && num-- > o_number) {
			p = p->prev;
		}
		return p;
	}

	num = o_number - off;
	p = csrse.ed;

	if(p == NULL) {
		goto failed; // おこらないはず。
	}
	if(off < 0) {
		while(p->prev != NULL && off++ < 0) {
			p = p->prev;
		}
	} else {
		while(p->next != NULL && off-- > 0) {
			p = p->next;
		}
	}
	return p;
}

size_t lists_size(long n_st, long n_en)
{
	long i;
	EditLine *ed;
	long a; /* size */

	a = 1;
	ed = GetList(n_st);
	for(i = n_st ; i <= n_en ; ++i) {
		if(ed == NULL || ed->buffer == NULL) {
			break;
		}
		a += strlen(ed->buffer);
		if(ed->next != NULL) {
			++a;
		}
		ed = ed->next;
	}
	return a;
}

void lists_proc(void func(const char *, void *), void *gp, long n_st, long n_en)
{
	long i;
	EditLine *ed;
	char buf[MAXEDITLINE + 1];

	ed = GetList(n_st);
	for(i = n_st ; i <= n_en ; ++i) {
		if(ed == NULL || ed->buffer == NULL || i > GetLastNumber()) {
			func(NULL, gp);
			continue;
		}
		if(ed->next == NULL) {
			strcpy(buf, ed->buffer);
		} else {
			sprintf(buf, "%s\n", ed->buffer);
		}
		func(buf, gp);
		ed = ed->next;
	}
}

void lists_add(void *func(char *, void *), void *gp)
{
	int n;
	char buf[MAXEDITLINE * 4 + 1];
	EditLine *ed, *edb, *ed_new;

	n = 0;
	ed_new = edb = GetList(GetLineOffset() - 1);
	while(gp != NULL) {
		 gp = func(buf, gp);
		 ed = MakeLine(buf);
		 InsertLine(edb, ed);
		 edb = ed;
		 ++n;
	}
	csrse.ed = ed_new->next;
}

void lists_delete(int n_st, int n_ed)
{
	EditLine *ed, *ed_next, *ed_new;
	int n;

	ed_new = csrse.ed;
	n = n_ed - n_st + 1;
	if(n_st <= GetLineOffset()) {
		int	m = n;
		while(m-- >0 && ed_new != NULL) {
			ed_new = ed_new->next;
		}
	}

	ed = GetList(n_st);
	while(n-- > 0) {
		if(n_st > GetLastNumber()) { // GetLastNumber は移動する。
			Realloc(ed, "");
			return;
		}
		ed_next = ed->next;
		DeleteList(ed);
		ed = ed_next;
	}
	csrse.ed = ed_new;
}

long history_get_last_count(int no)
{
	return LastCount[no];
}

HistoryData *history_get_top(int no)
{
	return &BaseHistory[no];
}

HistoryData *history_get_last(int no)
{
	return LastHistory[no];
}

void history_append_last(int no, HistoryData *hi)
{
	HistoryData *last;

	last = history_get_last(no);
	last->next = hi;
	hi->prev = last;
	hi->next = NULL;
	LastCount[no]++;
	LastHistory[no] = hi;
}

void history_insert_line(int no, HistoryData *bhi, HistoryData *hi)
{
	if(bhi->next == NULL) {
		history_append_last(no, hi);
		return;
	}

	bhi->next->prev = hi;
	hi->prev = bhi;
	hi->next = bhi->next;
	bhi->next = hi;

	LastCount[no]++;
}

void history_delete_list(int no, HistoryData *hi)
{
	if(hi->next != NULL) {
		hi->next->prev = hi->prev;
	} else {
		LastHistory[no] = hi->prev;
	}
	hi->prev->next = hi->next;
	free(hi->buffer);
	free(hi);
	LastCount[no]--;
}

HistoryData *history_get_file(char *filename)
{
	HistoryData *hi;

	hi = BaseHistory[FOPEN_SYSTEM].next;
	while(hi != NULL) {
		if(!strcmp(hi->buffer, filename)) {
			return hi;
		}
		hi = hi->next;
	}
	return NULL;
}

void history_set_line(char *filename, int line)
{
	HistoryData *hi;

	if((hi = history_get_file(filename)) != NULL) {
		hi->line = line;
	}
}

HistoryData *history_make_data(const char *buffer)
{
	HistoryData *hi;
	int n;

	hi = (HistoryData *)mem_alloc(sizeof(HistoryData));

	n = strlen(buffer);
	hi->buffer = (char *)mem_alloc(sizeof(char) * (n + 1));
	strcpy(hi->buffer, buffer);
	hi->line = 1;
	hi->prev = NULL;
	hi->next = NULL;

	return hi;
}

int history_add_file(char *filename)
{
	HistoryData *hi;

	if(*filename == '\0') {
		return 1;
	}
	if((hi = history_get_file(filename)) == NULL) {
		hi = history_make_data(filename);
		LastCount[FOPEN_SYSTEM]++;
	}
	if(LastHistory[FOPEN_SYSTEM] != hi) {
		if(hi->next != NULL) {
			hi->next->prev = hi->prev;
		}
		if(hi->prev != NULL) {
			hi->prev->next = hi->next;
		}
		LastHistory[FOPEN_SYSTEM]->next = hi;
		hi->next = NULL;
		hi->prev = LastHistory[FOPEN_SYSTEM];
		LastHistory[FOPEN_SYSTEM] = hi;
	}
	return hi->line;
}

void history_save_file()
{
	char path[LN_path + 1];
	FILE *fp;

	sysinfo_path(path, N8_HISTORY_FILE);
	if((fp = fopen(path, "w")) != NULL) {
		int count = 0;
		HistoryData *hi = LastHistory[FOPEN_SYSTEM];
		while(hi != NULL && count < LastCount[FOPEN_SYSTEM] && count < sysinfo.file_history_count) {
			fprintf(fp, "%s,%d\n", hi->buffer, hi->line);
			hi = hi->prev;
			count++;
		}
		fclose(fp);
	}
}

void history_load_file()
{
	char path[LN_path + 1];
	char buff[LN_path];
	FILE *fp;

	sysinfo_path(path, N8_HISTORY_FILE);
	if((fp = fopen(path, "r")) != NULL) {
		while(fgets(buff, LN_path, fp) != NULL) {
			char *pt;
			if((pt = strtok(buff, ",")) != NULL) {
				HistoryData *hi;
				hi = history_make_data(buff);
				if((pt = strtok(NULL, ",")) != NULL) {
					if(isdigit(*pt)) {
						hi->line = atoi(pt);
						hi->next = BaseHistory[FOPEN_SYSTEM].next;
						if(hi->next != NULL) {
							hi->next->prev = hi;
						}
						hi->prev = &BaseHistory[FOPEN_SYSTEM];
						BaseHistory[FOPEN_SYSTEM].next = hi;
						if(LastCount[FOPEN_SYSTEM] == 0) {
							LastHistory[FOPEN_SYSTEM] = hi;
						}
						LastCount[FOPEN_SYSTEM]++;
					}
				}
			}
		}
		fclose(fp);
	}
}

