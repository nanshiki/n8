#define	JM_ank		0
#define	JM_kana		1
#define	JM_kanji	2
#define	JM_so		3

#define	KC_euc		0
#define	KC_jis		1
#define	KC_sjis		2
#define	KC_utf8		3
#define	KC_utf8bom	4

#define	RM_lf	0
#define	RM_crlf	1
#define	RM_cr	2

bool iscnt(unsigned char c);
bool iskanji(int c);
int utf8_disp_copy(char *dst, const char *src, int count);
const char *get_utf8_code(const char *str, unsigned long *code);
int get_utf8_width(int c);
int get_utf8(const unsigned char *pt);
int IsKanjiPosition();
int char_getctype(int c);
int kanji_getctype(const char *pt);
int kanji_tknext(const char *s, int a, bool f);
int kanji_tkprev(const char *s, int a, bool f);
const char *kanji_from_utf8(char *dst, const char *src, int kc);
void write_utf8_bom(FILE *fp);
int file_kanji_check(FILE *fp);
int file_gets(char *s, size_t bytes, FILE *fp, int *n_cr, int *n_lf);
void kanji_to_utf8(char *dst, const char *src, int kc);
int kanji_poscanon(int offset, const char *buf);
int kanji_poscandsp(int offset, const char *buf);
int kanji_posnext(int offset, const char *buf);
int kanji_posprev(int offset, const char *buf);
int kanji_posdsp(int offset, const char *buf);
int kanji_posbuf(int offset, const char *buf);
int strjfcpy(char *s, const char *t, size_t bytes, size_t len);
int is_half_kana(const char *p);
int kanji_countbuf(const char *p);
int kanji_countdsp(const char *p, int n);
int get_delete_count(const char *str);
int get_display_length(const char *str);
int is_combining_char(const char *p);
