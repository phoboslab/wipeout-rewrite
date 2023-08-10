#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#include "menu.h"

void ingame_menus_load();

menu_t *pause_menu_init();
menu_t *game_over_menu_init();
menu_t *race_stats_menu_init();
menu_t *text_scroll_menu_init(char * const *lines, int len);

#endif
