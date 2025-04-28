#include "menu.h"

void key_set();
void dir_init();
void sort_init();
void sysinfo_optset();
void opt_set(const char *s, const char *t);
void opt_default();
void option_set_proc(int a, mitem_t *mip, void *vp);
void SeeOption();
void config_read(char *path);
bool check_use_ext(const char *name, const char *path);
void clear_string_item(int no);
void set_ext_item(int no, const char *p);
int get_string_item_count(int no);
char *get_string_item(int no, int pos);
