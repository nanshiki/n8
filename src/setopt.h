#include "menu.h"

void key_set();
void sysinfo_optset();
void opt_set(const char *s, const char *t);
void opt_default();
void option_set_proc(int a, mitem_t *mip, void *vp);
void SeeOption();
void config_read(char *path);
bool check_use_ext(char *name);
