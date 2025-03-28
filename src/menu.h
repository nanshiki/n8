#ifndef MENU_H
#define MENU_H

typedef	struct
{
	char str[MAXLINESTR + 1];
	color_t nc; /* normal時 */
	color_t cc; /* cursorがある時 */

	bool mf;	/* mark flag */
} mitem_t;

typedef	struct
{
	mitem_t *mitem;
	size_t nums;

	char *title;
	bool df;				/* filerで利用するdisable flag */

	int sy, cy;			/* 現在の座標 */

	dspreg_t *drp;
} menu_t;

void menu_itemmake(menu_t *mnp, void func(int, mitem_t *, void *), size_t nums, void *vp);
void menu_itemmakelists(menu_t *mnp, size_t width, size_t num, char *s);
void menu_iteminit(menu_t *mnp);
void menu_itemfin(menu_t *mnp);
void menu_dview(menu_t *mnp);
void menu_itemview(menu_t *mnp, int a, bool f);
void menu_view(menu_t *mnp);
void menu_csrmove(menu_t *mnp, int ly);
int menu_csrnext(menu_t *mnp, char c);
int menu_select(menu_t *mnp);
int menu_vselect(int x, int y, size_t num, ...);
dspreg_t *menu_regset(menu_t *mnp);

#endif
