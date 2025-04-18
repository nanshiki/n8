long GetTopNumber();
long GetLastNumber();
EditLine *GetTop();
EditLine *GetLast();
void lists_debug();
void lists_init();
void lists_clear();
EditLine *MakeLine(const char *buffer);
void Realloc(EditLine *li, const char *s);
void AppendLast(EditLine *li);
void InsertLine(EditLine *bli, EditLine *li);
void DeleteList(EditLine *li);
EditLine *GetList(long o_number);
size_t lists_size(long n_st, long n_en);
void lists_proc(void func(const char *, void *), void *gp, long n_st, long n_en);
void lists_add(void *func(char *, void *), void *gp);
void lists_delete(int n_st, int n_ed);
long history_get_last_count(int no);
HistoryData *history_get_top(int no);
HistoryData *history_get_last(int no);
HistoryData *history_make_data(const char *buffer);
void history_append_last(int no, HistoryData *hi);
void history_insert_line(int no, HistoryData *bhi, HistoryData *hi);
void history_delete_list(int no, HistoryData *hi);
void history_set_line(char *filename, int line);
int history_add_file(char *filename);
void history_save_file();
void history_load_file();
