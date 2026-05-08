#ifndef DEFINE_FILE_H
#define DEFINE_FILE_H

typedef	struct
{
	FILE *fp;
	int kc;
	int jm;			/* jis処理中 */

	int n_cr, n_lf;
} kinfo_t;

enum {
	openOK,
	openNG,
	openCancel,
};

enum {
	openModeNormal,
	openModeNew,
	openModeReadonly,
};

void FileStartInit(int f);
void edbuf_init();
int edbuf_lock(int func(FILE *, const char*), const char *fn);
int edbuf_rm_func(FILE *fp, const char *fn);
void edbuf_rm(int n);
int edbuf_add_func(FILE *fp, const char *fn);
int edbuf_add(const char *fn, int flag);
int edbuf_mv(int n, const char *fn);
int CheckFileAccess(const char *fn);
int file_change(int n);
void SetFileChangeFlag();
void ResetFileChangeFlag();
int CheckFileOpened(const char *fn);
int FindOutNextFile(int no);
int GetBackFile(int n);
void filesave_proc(const char *s, FILE *fp);
int filesave(char *filename, int f);
void *file_open_proc(char *s, kinfo_t *kip);
void *file_new_proc(char *s, void *vp);
int file_insert(char *filename);
int RenameFile(int current_no, const char *s);
int FileOpenOp(const char *path, int mode);
int fileclose(int n);
int SelectFileMenu();
void sysinfo_path(char *s, const char *t);
void set_temp_path(char *path);
int filer_file_open();

#endif
