void strjncpy(char *s, const char *t, size_t ln);
int kanji_chk(const char *s, const char *t);
int touchfile(const char *path, time_t atime, time_t mtime);
int mole_dir(const char *s);
#ifdef _WIN32
#define	W_OK	2
#define	R_OK	4
#define	F_OK	0
void realpath(const char *cp, char *s);
FILE *fopen_win(const char *filename, char *mode);
int access_win(const char *filename, int mode);
int chdir_win(const char *path);
int unlink_win(const char *path);
int mkdir_win(const char *path, int mode);
int rmdir_win(const char *path);
void change_dir_char(char *dir);
int wchar_to_utf8(const wchar_t *src, char *dst, int len);
int utf8_to_wchar(const char *src, wchar_t *dst, int len);
void getcwd_win(char *path, int len);
void get_home_dir(char *path, int len);
void get_exe_dir(char *path, int len);
char *strsep(char **stringp, const char *delim);
void set_current_dir(int drive, char *path);
void get_current_dir(int drive, char *path);
int check_unc_root(const char *path);

#define	fopen		fopen_win
#define	access		access_win
#define	chdir		chdir_win
#define	unlink		unlink_win
#define	mkdir		mkdir_win
#define	rmdir		rmdir_win
#define	getcwd		getcwd_win
#define	ftruncate	_chsize_s
#define read		_read
#define	fileno		_fileno
#define strdup		_strdup
#define strcasecmp	_stricmp
#define strncasecmp	_strnicmp

void Trace(const char *form , ...);
#endif
