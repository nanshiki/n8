/*--------------------------------------------------------------------
	nxeditor
			FILE NAME:nxedit.c
			Programed by : I.Neva
			R & D  ADVANCED SYSTEMS. IMAGING PRODUCTS.
			1992.06.01

    Copyright (c) 1998,1999,2000 SASAKI Shunsuke.
    All rights reserved. 

	n8
	Copyright (c) 2025 takapyu
--------------------------------------------------------------------*/
#define	VAL_impl

#include "n8.h"
#include "crt.h"
#include "list.h"
#include "line.h"
#include "block.h"
#include "search.h"
#include "filer.h"
#include "cursor.h"
#include "input.h"
#include "file.h"
#include "keyf.h"
#include "list.h"
#include "setopt.h"
#include "sh.h"
#include <ctype.h>
#include <locale.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#ifdef HAVE_SYS_UTSNAME_H
 #include <sys/utsname.h>
#endif

/*-------------------------------------------------------------------
	Main Command Loop
-------------------------------------------------------------------*/

void n8_loop(int region)
{
 	int key;

	for(;;) {
		if(region == 0) {
			CrtDrawAll();
		}
		dsp_allview();
		term_csrn();
		key = get_keyf(region);
		RefreshMessage();
		if(region == 0 && csrse.gf) {
			csrse.gf = FALSE;
			term_locate(GetRow(), GetCol() + NumWidth);
			crt_crmark(FALSE);
		}
		if(key == -1) {
			continue;
		}
		if((key & KF_normalcode) == 0) {
			funclist[0][key]();
			if(CurrentFileNo == -1 && BackFileNo == -1) {
				break;
			}
		} else {
			if(region == 0) {
				if(edbuf[CurrentFileNo].readonly) {
					system_msg(NOCHANGE_MSG);
				} else {
					InputAndCrt(key & ~KF_normalcode);
				}
			}
		}
	}
	history_save_file();
}

void n8_init()
{
	char *p;

	sysinfo.vp_def = hash_init(NULL, MAX_val);
	opt_default();

	p = getenv("HOME");
	if(p != NULL) {
		strcpy(sysinfo.nxpath, p);
	} else {
		getcwd(sysinfo.nxpath, LN_path);
	}
	strcat(sysinfo.nxpath, "/.n8");
	mkdir(sysinfo.nxpath, 0777);

	sysinfo.shell = getenv("SHELL");
	if(sysinfo.shell == NULL) {
		sysinfo.shell = "/bin/sh";
	}
	config_read("n8rc");

	p = hash_get(sysinfo.vp_def, "Locale");
	if(p != NULL && isalpha(*p)) {
		setlocale(LC_CTYPE, p);
	}
}

void delete_lock_file()
{
	char buf[LN_path + 1];

	sysinfo_path(buf, N8_LOCK_FILE);
	unlink(buf);
}

bool n8_arg(int argc, char *argv[])
{
	int line;
	int optcount;
	char buf[LN_dspbuf + 1];
	bool f;
	int c;
	char *sp, *p;
	char *rname = NULL;

	line = 0;
	f = FALSE;

	for(optcount = 1 ; optcount < argc ; ++optcount) {
		c = getopt(argc, argv, "jecrD:");
		if(c == EOF) {
			break;
		}
		switch(c) {
		case 'j':
			hash_set(sysinfo.vp_def, "japanese", "true");
			break;
		case 'e':
			hash_set(sysinfo.vp_def, "japanese", "false");
			break;
		case 'c':
			delete_lock_file();
			break;
		case 'r':
			{
				HistoryData *hi = history_get_last(historyOpen);
				if(hi != NULL && hi->buffer != NULL) {
					rname = hi->buffer;
				}
			}
			break;
		case 'D':
			strcpy(buf, optarg);
			optcount++;
			sp = buf;
			strsep(&sp, "=");
			if(sp != NULL) {
				hash_set(sysinfo.vp_def, buf, sp);
			}
			break;
		}
	}
	sysinfo_optset();
	if(rname != NULL) {
		if(FileOpenOp(rname, openModeNormal) == openOK) {
		 	f = TRUE;
		}
	}
	for( ; optcount < argc ; ++optcount) {
		if(*argv[optcount] == '+') {
			line = atoi(argv[optcount] + 1);
		} else {
			if(FileOpenOp(argv[optcount], openModeNormal) == openOK) {
			 	f = TRUE;
			}
		}
	}
	if(f && line > 0) {
		int a;
		a = min(line, GetRowWidth() / 2 + 1);
		csr_setly(1);
		csr_setly(line - a + 1);
		csr_setdy(a);
	}
	return f;
}

void signal_func()
{
	delete_lock_file();

	exit(1);
}

int main(int argc, char *argv[])
{
	bool open_flag;

	signal(SIGHUP, signal_func);
	signal(SIGTERM, signal_func);
	signal(SIGBUS, signal_func);
	signal(SIGSEGV, signal_func);

	term_init();
	term_start();
	term_cls();
	dsp_allinit();

	lists_init();
	edbuf_init();

	keydef_init();
	n8_init();
	history_load_file();
	open_flag = n8_arg(argc, argv);
	key_set();
	dir_init();
	sysinfo_optset();

	udbuf_init();
	bstack_init();
	search_init();
	eff_init(NULL, NULL);
	sort_init();
	system_guide_init();
	*sysinfo.doublekey = '\0';
	if(!open_flag) {
		op_file_open();
	}
	if(CurrentFileNo < MAX_edbuf) {
		n8_loop(0);
	}
}

/*-------------------------------------------------------------------
	Escape Shell
-------------------------------------------------------------------*/
void CommandCom(char *sys_buff)
{
	bool f;

	term_stop();
	if(*sys_buff == '\0') {
		puts(TYPE_EXIT_MSG);
		system(sysinfo.shell);
		f = FALSE;
	} else {
		puts(sys_buff);
		system(sys_buff);
		f = TRUE;
	}
	term_start();
	if(f) {
		fputs(HIT_ANY_KEY_MSG, stdout);
		fflush(stdout);
		term_kflush();
		term_inkey();
	}
	term_cls();
}

void op_misc_exec()
{
	char buf[MAXEDITLINE + 1];

	*buf = '\0';
	if(HisGets(buf, GETS_SHELL_MSG, historyShell) != NULL) {
		CommandCom(buf);
	}
}

void op_opt_linenum()
{
	opt_set("number", NULL);
}

void op_misc_redraw()
{
	term_reinit();
	dsp_reinit();
	system_guide_reinit();
	eff_reinit();
	term_cls();
}

/*-----------------------------------------------------------------------------
	fork shell and take its stdout/stderr.
*/
void op_misc_insert_output(void)
{
	pid_t pid_child;
	int pipefds[2];
	char buf[MAXEDITLINE + 1] = "";

	if(HisGets(buf, GETS_SHELL_MSG, historyShell) == NULL) {
		return;
		/* NOTREACHED */
	}
	/*	Normally, this may not be failed, but... */
	if(pipe(pipefds) != 0) {
		system_msg(strerror(errno));
		term_inkey();
		return;
		/* NOTREACHED */
	}
	pid_child = fork();
	switch(pid_child) {
		/*	now we are child. */
		case 0:
			/*	pipe plumbing.
				child process's stdout and stderr are dup'ed from same fd.
				whatever written into these will end up to our pipefds[ 0 ].
			*/
			close(1);
			dup(pipefds[1]);	/* duplicate to stdout */
			close(2); 
			dup(pipefds[1]) ;	/* duplicate to stderr */
			/*	these fds are no longer useful. */
			close(pipefds[0]);
			close(pipefds[1]);
			execl(sysinfo.shell, sysinfo.shell, "-c", buf, NULL);	/* should not be failed. */
			_exit(1);
			/* NOTREACHED */
		/*	cannot be forked off. */
		case -1:
			system_msg(strerror(errno));
			term_inkey();
			system_msg("");
			close(pipefds[0]);
			close(pipefds[1]);
			break;
			/* NOTREACHED */
		/*	now we are parent. */
		default :
		{
			int status ;
			FILE *fp_pipe;

			close(pipefds[1]);
			fp_pipe = fdopen(pipefds[0], "r");
			if(fp_pipe == NULL) {
				system_msg(strerror(errno));
				term_inkey();
				system_msg("");
			} else {
				EditLine *ed, *ed_new;
				bool f;
				int n;

				csr_leupdate();
				ed_new = ed = GetList(GetLineOffset() - 1);
				f = FALSE;

				term_stop();
				while(fgets(buf, sizeof(buf) - 1, fp_pipe)) {
					/* echo to stdout for user friendliness. :) */
					fputs(buf, stdout);
					/* remove last '\n' */
					n = strlen(buf) - 1;
					if(buf[n] == '\n') {
						buf[n] = '\0' ;
					}
					/*	create new linebuffer and insert into list. */
					InsertLine(ed, MakeLine(buf));
					ed = ed->next;
					f = TRUE;
				}
				fclose(fp_pipe);
				close(pipefds[0]);
				term_start();
				if(f) {
					SetFileChangeFlag() ;	/* now, file is dirty. */
					csrse.ed = ed_new->next;
					csr_lenew();
					OffsetSetByColumn();
				}
			}
			wait(&status);
			term_cls();
		}
	}
}

