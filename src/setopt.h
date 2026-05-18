#include "menu.h"

void key_set();
void dir_init();
void sort_init();
void sysinfo_optset();
void sysinfo_keyword();
void opt_set(const char *s, const char *t);
void opt_default();
void option_set_proc(int a, mitem_t *mip, void *vp);
void SeeOption();
void config_read(char *path);
int check_use_ext(const char *name, const char *path);
void clear_string_item(sitem_t **top);
void set_ext_item(sitem_t **top, const char *p);
int get_string_item_count(int no);
char *get_string_item(int no, int pos);
void start_mask_reg();
void end_mask_reg();
void search_option(int x, int y);
int check_file_ext(const char *filename, sitem_t **top);
int check_cmode_ext(const char *filename);
void set_keyword_ext(const char *filename);
