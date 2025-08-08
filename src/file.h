#ifndef DEFINE_FILE_H
#define DEFINE_FILE_H

typedef	struct
{
	FILE *fp;
	int kc;
	int jm;			/* jisèàóùíÜ */

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

void FileStartInit(bool f);
void edbuf_init();
bool edbuf_lock(bool func(FILE *, const char*), const char *fn);
bool edbuf_rm_func(FILE *fp, const char *fn);
void edbuf_rm(int n);
bool edbuf_add_func(FILE *fp, const char *fn);
bool edbuf_add(const char *fn, bool flag);
bool edbuf_mv(int n, const char *fn);
bool CheckFileAccess(const char *fn);
bool file_change(int n);
void SetFileChangeFlag();
void ResetFileChangeFlag();
int CheckFileOpened(const char *fn);
int FindOutNextFile(int no);
int GetBackFile(int n);
void filesave_proc(const char *s, FILE *fp);
int filesave(char *filename, bool f);
void *file_open_proc(char *s, kinfo_t *kip);
void *file_new_proc(char *s, void *vp);
bool file_insert(char *filename);
bool RenameFile(int current_no, const char *s);
int FileOpenOp(const char *path, int mode);
bool fileclose(int n);
int SelectFileMenu();
void sysinfo_path(char *s, const char *t);
void set_temp_path(char *path);
bool filer_file_open();

#endif
