#ifndef MENU_H
#define MENU_H

#include "../types.h"
#include "ui.h"

#define MENU_PAGES_MAX 8
#define MENU_ENTRIES_MAX 16

typedef enum {
	MENU_ENTRY_BUTTON,
	MENU_ENTRY_TOGGLE
} menu_entry_type_t;

typedef enum {
	MENU_VERTICAL     = (1<<0),
	MENU_HORIZONTAL   = (1<<1),
	MENU_FIXED        = (1<<2),
	MENU_ALIGN_CENTER = (1<<3),
	MENU_ALIGN_BLOCK  = (1<<4)
} menu_page_layout_t;

typedef struct menu_t menu_t;
typedef struct menu_page_t menu_page_t;
typedef struct menu_entry_t menu_entry_t;
typedef struct menu_entry_options_t menu_entry_options_t;

struct menu_entry_t {
	menu_entry_type_t type;
	int data;
	char *text;
	void (*select_func)(menu_t *, int);
	const char **options;
	int options_len;
};

struct menu_page_t {
	char *title, *subtitle;
	menu_page_layout_t layout_flags;
	void (*draw_func)(menu_t *, int);
	void (*init_func)();
	void (*exit_func)();
	menu_entry_t entries[MENU_ENTRIES_MAX];
	int entries_len;
	int index;
	int block_width;
	vec2i_t title_pos;
	ui_pos_t title_anchor;
	vec2i_t items_pos;
	ui_pos_t items_anchor;
};

struct menu_t {
	menu_page_t pages[MENU_PAGES_MAX];
	int index;
};


void menu_reset(menu_t *menu);
menu_page_t *menu_push(menu_t *menu, char *title, void(*draw_func)(menu_t *, int), void(*init_func)(), void(*exit_func)());
menu_page_t *menu_confirm(menu_t *menu, char *title, char *subtitle, char *yes, char *no, void(*confirm_func)(menu_t *, int));
void menu_pop(menu_t *menu);
void menu_page_add_button(menu_page_t *page, int data, const char *text, void(*select_func)(menu_t *, int));
void menu_page_add_toggle(menu_page_t *page, int data, const char *text, const char **options, int len, void(*select_func)(menu_t *, int));
void menu_update(menu_t *menu);

#endif
