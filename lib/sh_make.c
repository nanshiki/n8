/*
 *    SHELL function generate utility.
 *
 * Copyright (C) 1996, 1998, 1999, 2000 SASAKI Shunsuke.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define MAX_region	16


int max(int x, int y)
{
	return x > y ? x : y;
}

void error(char *s)
{
	fputs(s, stderr);
	fputs("\n", stderr);
	exit(1);
}

typedef struct
{
	char name[256 + 1];
	int args;
} func_t;

func_t func[MAX_region][256];
int func_n[MAX_region];

int main(int argc, char *argv[])
{
	FILE *fpr, *fpw;
	int i;
	char buf[1024], *p;

	char fname[32][64 + 1];
	int f, file_n;
	int region;

	int func_max, region_max;

	p = argc < 2 ? "sh_shells.c" : argv[1];
	fpw = fopen(p, "w");
	if(fpw == NULL) {
		error("error:write modeでファイルオープンできません。");
	}
	strcpy(fname[0], "sh.h");
	file_n = 1;
	for(i = 0 ; i < MAX_region ; ++i) {
		func_n[i] = 0;
	}
	region = 0;
	func_max = 0;
	region_max = 0;
	for(f = 0 ; f < file_n ; ++f) {
		fpr = fopen(fname[f], "r");
		if(fpr == NULL) {
			continue; /* ファイルがopenできない場合、無視 */
		}
		while(fgets(buf, sizeof(buf), fpr)) {
			p = strtok(buf," \t");
			while(isspace(*p)) {
				++p;
			}
			if(strcmp(buf,"#include") == 0) {
				p = strtok(NULL, "");
				p = strchr(p, '"');
				if(p == NULL) {
					continue;
				}
				p = strtok(p, "\"");
				if(p == NULL) {
					error("Wrong #include");
				}
				strcpy(fname[file_n++], p);
				continue;
			}
			if(strcmp(p, "//") == 0) {
				p = strtok(NULL, " \t");
				if(p == NULL) {
					continue;
				}
				if(strcasecmp(p, "shell") == 0) {
					p = strtok(NULL, " \t\r\n");
					if(p == NULL) {
						continue;
					}
					while(isspace(*p)) {
						++p;
					}
					strcpy(func[region][func_n[region]].name, p);
					func[region][func_n[region]].args = 0;
					++func_n[region];
					func_max = max(func_max, func_n[region]);
					continue;
				}
				if(strcasecmp(p, "region") == 0) {
					p = strtok(NULL, " \t\r\n");
					if(p == NULL) {
						continue;
					}
					while(isspace(*p)) {
						++p;
					}
					func_max = max(func_max, func_n[region]);
					region = atoi(p);
					if(region < 0 || region >= MAX_region) {
						error("Wrong region");
					}
					region_max = max(region_max, region);
				}
				continue;
			}
			if(strcmp(p, "SHELL") != 0) {
				continue;
			}
			p = strtok(NULL," \t");	/* 型 */
			p = strtok(NULL,"(");	/* 関数 */
			if(p == NULL) {
				error("Wrong function define");
			}
			while(isspace(*p)) {
				++p;
			}
			i = strlen(p);
			while(isspace(p[i]) && i > 0) {
				--i;
			}
			if(i == 0) {
				error("Wrong function define");
			}
			p[i + 1] = '\0';
			strcpy(func[region][func_n[region]].name, p);
			func[region][func_n[region]].args = 1;	/* 引数の数は確かめていない。 */
			++func_n[region];
			func_max = max(func_max, func_n[region]);
		}
		fclose(fpr);
	}
	++region_max;
	puts("#ifndef\t__SH_DEF_H_");
	puts("#define\t__SH_DEF_H_\n");
	printf("#define\tMAX_region\t%d\n", region_max);
	printf("#define\tMAX_func\t%d\n", func_max);
	for(f = 0 ; f < region_max ; ++f) {
		if(func_n[f] == 0) {
			continue;
		}
		puts(	"\nenum {");
		for(i = 0 ; i < func_n[f] ; ++i) {
			p = func[f][i].name;
			if(strncasecmp(p,"op_",3) == 0) {
				p += 3;
			}
			printf("\t KF_%s", p);
			if(i + 1 < func_n[f]) {
				puts(",");
			} else {
				puts("");
			}
		}
		puts(	"\t};");
	}
	puts("#endif");
	fprintf(fpw,"/* %d x %d */\n", region_max, func_max);
	fprintf(fpw,"#include\t\"%s\"\n\n"
				"static\tvoid\tdummy()\n"
				"{;}\n\n"
			, fname[0]);
	if(func_max  > 0) {
		fputs("char\t*keyfcode[MAX_region][MAX_func]=\n"
			  "\t{\n"
			  "\t\t{\n"
			, fpw);

		for(f = 0 ; f < region_max ; ++f) {
			for(i = 0 ; i < func_max ; ++i) {
				if(func_n[f] > i) {
				 	fprintf(fpw,"\t\t \"%s\"", func[f][i].name);
				} else {
				 	fputs("\t\t \"\"", fpw);
				}
				if(i + 1 < func_max) {
					fputs(",\n", fpw);
				} else {
					fputs("\n", fpw);
				}
			}
			fputs("\t\t}", fpw);
			if(f + 1 >= region_max) {
				fputs("\n", fpw);
			} else {
				fputs(",\n"
				      "\t\t{\n", fpw);
			}
		}
		fputs("\t};\n", fpw);
		fputs(	"void\t(*funclist[MAX_region][MAX_func])()=\n"
				"\t{\n"
				"\t\t{\n", fpw);

		for(f = 0 ; f < region_max ; ++f) {
			for(i = 0 ; i < func_max ; ++i) {
				if(func_n[f] > i && func[f][i].args > 0) {
					fprintf(fpw,"\t\t %s", func[f][i].name);
				} else {
					fputs("\t\t dummy", fpw);
				}
				if(i + 1 < func_max) {
					fputs(",\n", fpw);
				} else {
					fputs("\n", fpw);
				}
			}
			fputs("\t\t}", fpw);
			if(f + 1 >= region_max) {
				fputs("\n", fpw);
			} else {
				fputs(",\n"
				      "\t\t{\n", fpw);
			}
		}
		fputs("\t};\n", fpw);
	}
	fclose(fpw);
	exit(EXIT_SUCCESS);
}

