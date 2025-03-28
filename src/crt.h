typedef	struct
{
	int dline;	// 画面上のライン
	long line;	// バッファの row

	block_t bm;
} crt_draw_t;

int GetMinRow();
int GetMaxRow();
int GetRowWidth();
int GetMinCol();
int GetMaxCol();
int GetColWidth();
void widthputs(const char *s, size_t len);
void crt_crmark();
void crt_draw_proc(const char *s, crt_draw_t *gp);
void CrtDrawAll();
void DeleteAndDraw();
void InsertAndDraw();
void RefreshMessage();
dspfmt_t *dspreg_guide(void *vp, int a, int sizex, int sizey);
void putDoubleKey(int key);
void delDoubleKey();
void system_guide_init();
void system_guide();
dspfmt_t *dspreg_sysmsg(void *vp, int a, int sizex, int sizey);
void system_msg(const char *buffer);
