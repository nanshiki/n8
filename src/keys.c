#include "n8.h"
#include "keys.h"
#include "sh.h"
#include "../lib/misc.h"
#include <termcap.h>
#include <ctype.h>

int keyf_getname(const char *s, int n)
{
	int i;
	int a;

	if(n >= MAX_region) {
		return -1;
	}

	if(strncasecmp(s, "op_", 3) == 0) {
		a = 0;
	} else {
		a = 3;
	}

	for(i = 0 ; i < MAX_func ; ++i) {
		if(*keyfcode[n][i] == '\0') {
			break;
		}
		if(strcasecmp(keyfcode[n][i] + a, s) == 0) {
			return i;
		}
	}
	return -1;
}

void keyf_setarg(keydef_t *kdp, const char *s)
{
	char buf[2048 + 1], *p;
	int n;
	bool f;

	while(*s != '\0' && isspace(*s)) {
		++s;
	}

	f = FALSE;
	n = 0;
	p = buf;
	do {
		if(*s == '\"') {
			f = !f;
			++s;
			if(*s != '\0') {
				continue;
			}
		}

		if(f || (*s != '\0' && !isspace(*s))) {
			*p++ = *s++;
			if(*s != '\0') {
				continue;
			}
		}

		*p = '\0';
		kdp->args[n] = (char *)mem_alloc(strlen(buf) + 1);
		strcpy(kdp->args[n], buf);
		++n;

		while(*s != '\0' && isspace(*s)) {
			++s;
		}
		p = buf;
	} while(*s != '\0');

	kdp->args[n] = NULL;
}

void keyf_set(int region, const char *k, const char *s)
{
	keydef_t *kdp;
	int key[2];
	int n;
	char buf[LN_buf + 1], *p;


	p = strchr(s, ' ');
	if(p == NULL) {
		n = keyf_getname(s, region);
	} else {
		strjncpy(buf, s, p - s);
		n = keyf_getname(buf, region);
	}
	if(n == -1) {
		return;
	}

	keysdef_getcode(k, key, 2);
	kdp = keydef_get(region, key[0], key[1]);
	if(kdp == NULL || kdp == (keydef_t *) - 1) {
		kdp = keydef_set(region, KDM_func, n, key[0], key[1]);
	}

	// ?? 上のif分の中にいれておいてよいのではなかったか?
	if(p != NULL && kdp != NULL && kdp != (keydef_t *)-1) {
		keyf_setarg(kdp, p);
	} else {
		kdp->args[0] = NULL;
	}
}




