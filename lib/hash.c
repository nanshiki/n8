/*
 *    hash module.
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
#include	"config.h"

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"generic.h"
#include	"hash.h"

void strjncpy(char *s, const char *t, size_t ln);

/* ☆hash */
hash_t *hash_init(hash_t *gvp, int m)
{
	if(gvp == NULL) {
		gvp = (hash_t *)mem_alloc(sizeof(hash_t));
	}
	gvp->item = (hash_item_t *)mem_alloc(sizeof(hash_item_t) * m);
	gvp->num = 0;
	gvp->max = m;

	return gvp;
}

void hash_fin(hash_t *gvp)
{
	free(gvp->item);
	gvp->num = 0;
	gvp->max = 0;
}

char *hash_get(hash_t *gvp, const char *ks)
{
	int i;

	for(i = 0 ; i < gvp->num ; ++i) {
		if(strcasecmp(gvp->item[i].key, ks) == 0) {
			return gvp->item[i].val;
		}
	}
	return NULL;
}

void hash_set(hash_t *gvp, const char *ks, const char *vs)
{
	if(ks != NULL && vs != NULL) {
		char *p;

		p = hash_get(gvp, ks);
		if(p == NULL) {
			if(gvp->num >= gvp->max) {
				gvp->max *= 2;
				gvp->item = (hash_item_t *)mem_realloc(gvp->item, sizeof(hash_item_t) * gvp->max);
			}
			strjncpy(gvp->item[gvp->num].key, ks, LN_key);
			p = gvp->item[gvp->num].val;
			++gvp->num;
		}
		strjncpy(p, vs, LN_val);
	}
}

void hash_defset(hash_t *gvp, const char *ks, const char *vs)
{
	char *p;

	p = hash_get(gvp, ks);
	if(p != NULL) {
		return;
	}
	if(gvp->num >= gvp->max) {
		gvp->max *= 2;
		gvp->item = (hash_item_t *)mem_realloc(gvp->item, sizeof(hash_item_t) * gvp->max);
	}
	strjncpy(gvp->item[gvp->num].key, ks, LN_key);
	strjncpy(gvp->item[gvp->num].val, vs, LN_val);
	++gvp->num;
}

bool hash_istrue(hash_t *gvp, const char *ks)
{
	const char *p;

	p = hash_get(gvp, ks);
	return p != NULL && (strcasecmp(p, "true") == 0 || strcasecmp(p, "on") == 0);
}

void hash_report(hash_t *gvp)
{
	int i;

	report_puts("内部変数:\n");
	for(i = 0 ; i < gvp->num ; ++i) {
		report_printf("  %s = [%s]\n", gvp->item[i].key, gvp->item[i].val);
	}
}

int hash_sort_cmp(const void *v, const void *w)
{
	return strcmp(((hash_item_t *)v)->key, ((hash_item_t *)w)->key);
}

void hash_sort(hash_t *gvp)
{
	qsort(gvp->item, gvp->num, sizeof(hash_item_t), hash_sort_cmp);
}
