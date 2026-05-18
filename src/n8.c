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
#include "lineedit.h"
#include "sh.h"
#include <ctype.h>
#include <locale.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <sys/wait.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <direct.h>
#else
#ifdef HAVE_SYS_UTSNAME_H
 #include <sys/utsname.h>
#endif
#endif

#include "../lib/regexp.h"
#include "../lib/misc.h"

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
}

void n8_init()
{
	char *p;

	sysinfo.vp_def = hash_init(NULL, MAX_val);
	opt_default();

#ifdef _WIN32
	get_home_dir(sysinfo.nxpath, LN_path);
#else
	p = getenv("HOME");
	if(p != NULL) {
		strcpy(sysinfo.nxpath, p);
	} else {
		getcwd(sysinfo.nxpath, LN_path);
	}
#endif
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

#ifdef _WIN32
int n8_arg(int argc, wchar_t *argv[])
#else
int n8_arg(int argc, char *argv[])
#endif
{
	int line;
	int optcount;
	char buf[LN_dspbuf + 1];
	int f;
	char *sp;
	char *rname = NULL;

	line = 0;
	f = FALSE;

	sysinfo_optset();
	sysinfo_keyword();
	for(optcount = 1 ; optcount < argc ; ++optcount) {
		if(argv[optcount][0] == '-') {
			switch(argv[optcount][1]) {
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
				if(optcount < argc - 1) {
					optcount++;
#ifdef _WIN32
					wchar_to_utf8(argv[optcount], buf, LN_dspbuf);
#else
					strjcpy(buf, argv[optcount], LN_dspbuf);
#endif
					sp = buf;
					strsep(&sp, "=");
					if(sp != NULL) {
						hash_set(sysinfo.vp_def, buf, sp);
					}
				}
				break;
			}
		} else if(argv[optcount][0] == '+') {
#ifdef _WIN32
			line = _wtoi(&argv[optcount][1]);
#else
			line = atoi(&argv[optcount][1]);
#endif
		} else {
#ifdef _WIN32
			char current_path[LN_path + 1];
			getcwd(current_path, LN_path);
			if(argv[optcount][0] == '.' && (argv[optcount][1] == '/' || argv[optcount][1] == '\\')) {
				wchar_to_utf8(&argv[optcount][2], buf, LN_dspbuf);
			} else {
				wchar_to_utf8(argv[optcount], buf, LN_dspbuf);
			}
			correct_path(buf, current_path);
#else
			if(argv[optcount][0] == '.' && (argv[optcount][1] == '/' || argv[optcount][1] == '\\')) {
				strjcpy(buf, &argv[optcount][2], LN_dspbuf);
			} else {
				strjcpy(buf, argv[optcount], LN_dspbuf);
			}
#endif
			if(FileOpenOp(buf, openModeNormal) == openOK) {
			 	f = TRUE;
			} else if(dir_isdir(buf)) {
				set_temp_path(buf);
			}
		}
	}
	if(rname != NULL) {
		if(FileOpenOp(rname, openModeNormal) == openOK) {
		 	f = TRUE;
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

#ifdef _WIN32
BOOL WINAPI signal_func(DWORD signal)
{
	if(signal != CTRL_C_EVENT) {
		delete_lock_file();
		return TRUE;
	}
	return FALSE;
}

#else
void signal_func(int sig)
{
	if(sig == SIGWINCH) {
		term_change_screen();
	} else {
		delete_lock_file();

		exit(1);
	}
}
#endif

#ifdef _WIN32
int wmain(int argc, wchar_t *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	int open_flag;
#ifdef _WIN32
	SetConsoleCtrlHandler(signal_func, TRUE);
	regexp_init();
#else
	signal(SIGHUP, signal_func);
	signal(SIGTERM, signal_func);
	signal(SIGBUS, signal_func);
	signal(SIGSEGV, signal_func);
	signal(SIGWINCH, signal_func);
#endif
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

	udbuf_init();
	bstack_init();
	search_init();
	eff_init(NULL, NULL);
	sort_init();
	system_guide_init();
	*sysinfo.doublekey = '\0';
	if(!open_flag) {
		if(!filer_file_open()) {
			op_file_open();
		}
	}
	if(CurrentFileNo < MAX_edbuf) {
		n8_loop(0);
	}
	history_save_file();
	delete_lock_file();
#ifdef _WIN32
	term_stop();
#endif
}

/*-------------------------------------------------------------------
	Escape Shell
-------------------------------------------------------------------*/
void CommandCom(char *sys_buff)
{
	int f;

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
	crt_reinit();
	system_guide_reinit();
	eff_reinit();
	term_cls();
}

/*-----------------------------------------------------------------------------
	fork shell and take its stdout/stderr.
*/
void op_misc_insert_output(void)
{
#ifndef _WIN32
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
				int f;
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
#endif
}

