#include <string.h>

#include "../input.h"
#include "../system.h"
#include "../utils.h"
#include "../mem.h"

#include "menu.h"
#include "ingame_menus.h"
#include "game.h"
#include "image.h"
#include "ui.h"
#include "race.h"

#if defined(__EMSCRIPTEN__)
	#include <emscripten/emscripten.h>
#endif

static void page_race_points_init(menu_t * menu);
static void page_championship_points_init(menu_t * menu);
static void page_hall_of_fame_init(menu_t * menu);

static texture_list_t pilot_portraits;
static menu_t *ingame_menu;

void ingame_menus_load(void) {
	pilot_portraits = image_get_compressed_textures(def.pilots[g.pilot].portrait);
	ingame_menu = mem_bump(sizeof(menu_t));
}

// -----------------------------------------------------------------------------
// Pause Menu

static void button_continue(menu_t *menu, int data) {
	race_unpause();
}

static void button_restart_confirm(menu_t *menu, int data) {
	if (data) {
		race_restart();
	}
	else {
		menu_pop(menu);
	}
}

static void button_restart_or_quit(menu_t *menu, int data) {
	if (data) {
		race_restart();
	}
	else {
		game_set_scene(GAME_SCENE_MAIN_MENU);
	}
}

static void button_restart(menu_t *menu, int data) {
	menu_confirm(menu, "ARE YOU SURE YOU", "WANT TO RESTART", "YES", "NO", button_restart_confirm);
}

static void button_quit_confirm(menu_t *menu, int data) {
	if (data) {
		game_set_scene(GAME_SCENE_MAIN_MENU);
	}
	else {
		menu_pop(menu);
	}
}

static void button_quit(menu_t *menu, int data) {
	menu_confirm(menu, "ARE YOU SURE YOU", "WANT TO QUIT", "YES", "NO", button_quit_confirm);
}


static void button_music_track(menu_t *menu, int data) {
	sfx_music_play(data);
	sfx_music_mode(SFX_MUSIC_LOOP);
}

static void button_music_random(menu_t *menu, int data) {
	sfx_music_play(rand_int(0, len(def.music)));
	sfx_music_mode(SFX_MUSIC_RANDOM);
}

static void button_music(menu_t *menu, int data) {
	menu_page_t *page = menu_push(menu, "MUSIC", NULL);

	for (int i = 0; i < len(def.music); i++) {
		menu_page_add_button(page, i, def.music[i].name, button_music_track);
	}
	menu_page_add_button(page, 0, "RANDOM", button_music_random);
}

menu_t *pause_menu_init(void) {
	sfx_play(SFX_MENU_SELECT);
	menu_reset(ingame_menu);

	menu_page_t *page = menu_push(ingame_menu, "PAUSED", NULL);
	menu_page_add_button(page, 0, "CONTINUE", button_continue);
	menu_page_add_button(page, 0, "RESTART", button_restart);
	menu_page_add_button(page, 0, "QUIT", button_quit);
	menu_page_add_button(page, 0, "MUSIC", button_music);

	#if defined(__EMSCRIPTEN__)
		ui_update_pause_button(false);
	#endif

	return ingame_menu;
}



// -----------------------------------------------------------------------------
// Game Over

menu_t *game_over_menu_init(void) {
	sfx_play(SFX_MENU_SELECT);
	menu_reset(ingame_menu);

	menu_page_t *page = menu_push(ingame_menu, "GAME OVER", NULL);
	menu_page_add_button(page, 1, "", button_quit_confirm);
	return ingame_menu;
}


// -----------------------------------------------------------------------------
// Race Stats

static void button_qualify_confirm(menu_t *menu, int data) {
	if (data) {
		race_restart();
	}
	else {
		game_set_scene(GAME_SCENE_MAIN_MENU);
	}
}

static void button_race_stats_continue(menu_t *menu, int data) {
	if (g.race_type == RACE_TYPE_CHAMPIONSHIP) {
		if (g.race_position <= QUALIFYING_RANK) {
			page_race_points_init(menu);
		}
		else {
			menu_page_t *page = menu_confirm(menu, "CONTINUE QUALIFYING OR QUIT", "", "QUALIFY", "QUIT", button_qualify_confirm);
			page->index = 0;
		}
	}
	else {
		if (g.is_new_race_record) {
			page_hall_of_fame_init(menu);
		}
		else {
			menu_confirm(menu, "", "RESTART RACE", "RESTART", "QUIT", button_restart_or_quit);
		}
	}
}

static void page_race_stats_draw(menu_t *menu, int data) {
	menu_page_t *page = &menu->pages[menu->index];
	vec2i_t pos = page->title_pos;
	pos.x -= 140;
	pos.y += 32;
	ui_pos_t anchor = UI_POS_MIDDLE | UI_POS_CENTER;

	// Pilot portrait and race position - only for championship or single race
	if (g.race_type != RACE_TYPE_TIME_TRIAL) {
		vec2i_t image_pos = ui_scaled_pos(anchor, vec2i(pos.x + 180, pos.y));
		uint16_t image = texture_from_list(pilot_portraits, g.race_position <= QUALIFYING_RANK ? 1 : 0);
		render_push_2d(image_pos, ui_scaled(render_texture_size(image)), rgba(0, 0, 0, 128), RENDER_NO_TEXTURE);
		ui_draw_image(image_pos, image);

		ui_draw_text("RACE POSITION", ui_scaled_pos(anchor, pos), UI_SIZE_8, UI_COLOR_ACCENT);
		ui_draw_number(g.race_position, ui_scaled_pos(anchor, vec2i(pos.x + ui_text_width("RACE POSITION", UI_SIZE_8)+8, pos.y)), UI_SIZE_8, UI_COLOR_DEFAULT);
	}

	pos.y += 32;

	ui_draw_text("RACE STATISTICS", ui_scaled_pos(anchor, pos), UI_SIZE_8, UI_COLOR_ACCENT);
	pos.y += 16;

	for (int i = 0; i < NUM_LAPS; i++) {
		ui_draw_text("LAP", ui_scaled_pos(anchor, vec2i(pos.x + 8, pos.y)), UI_SIZE_8, UI_COLOR_ACCENT);
		ui_draw_number(i+1, ui_scaled_pos(anchor, vec2i(pos.x + 50, pos.y)), UI_SIZE_8, UI_COLOR_ACCENT);
		ui_draw_time(g.lap_times[g.pilot][i], ui_scaled_pos(anchor, vec2i(pos.x + 72, pos.y)), UI_SIZE_8, UI_COLOR_DEFAULT);
		pos.y+= 12;
	}
	pos.y += 32;

	ui_draw_text("RACE TIME", ui_scaled_pos(anchor, pos), UI_SIZE_8, UI_COLOR_ACCENT);
	pos.y += 12;
	ui_draw_time(g.race_time, ui_scaled_pos(anchor, vec2i(pos.x + 8, pos.y)), UI_SIZE_8, UI_COLOR_DEFAULT);
	pos.y += 12;

	ui_draw_text("BEST LAP", ui_scaled_pos(anchor, pos), UI_SIZE_8, UI_COLOR_ACCENT);
	pos.y += 12;
	ui_draw_time(g.best_lap, ui_scaled_pos(anchor, vec2i(pos.x + 8, pos.y)), UI_SIZE_8, UI_COLOR_DEFAULT);
	pos.y += 12;
}

menu_t *race_stats_menu_init(void) {
	sfx_play(SFX_MENU_SELECT);
	menu_reset(ingame_menu);
	
	char *title;
	if (g.race_type == RACE_TYPE_TIME_TRIAL) {
		title = "";
	}
	else if (g.race_position <= QUALIFYING_RANK) {
		title = "CONGRATULATIONS";
	}
	else {
		title = "FAILED TO QUALIFY";
	}
	menu_page_t *page = menu_push(ingame_menu, title, page_race_stats_draw);
	flags_add(page->layout_flags, MENU_FIXED);
	page->title_anchor = UI_POS_MIDDLE | UI_POS_CENTER;
	page->title_pos = vec2i(0, -100);
	menu_page_add_button(page, 1, "", button_race_stats_continue);
	return ingame_menu;
}


// -----------------------------------------------------------------------------
// Race Table

static void button_race_points_continue(menu_t *menu, int data) {
	if (g.race_type == RACE_TYPE_CHAMPIONSHIP) {
		page_championship_points_init(menu);
	}
	else if (g.is_new_race_record) {
		page_hall_of_fame_init(menu);
	}
	else {
		menu_confirm(menu, "", "RESTART RACE", "RESTART", "QUIT", button_restart_or_quit);
	}
}

static void page_race_points_draw(menu_t *menu, int data) {
	menu_page_t *page = &menu->pages[menu->index];
	vec2i_t pos = page->title_pos;
	pos.x -= 140;
	pos.y += 32;
	ui_pos_t anchor = UI_POS_MIDDLE | UI_POS_CENTER;

	ui_draw_text("PILOT NAME", ui_scaled_pos(anchor, pos), UI_SIZE_8, UI_COLOR_ACCENT);
	ui_draw_text("POINTS", ui_scaled_pos(anchor, vec2i(pos.x + 222, pos.y)), UI_SIZE_8, UI_COLOR_ACCENT);

	pos.y += 24;

	for (int i = 0; i < len(g.race_ranks); i++) {
		rgba_t color = g.race_ranks[i].pilot == g.pilot ? UI_COLOR_ACCENT : UI_COLOR_DEFAULT;
		ui_draw_text(def.pilots[g.race_ranks[i].pilot].name, ui_scaled_pos(anchor, pos), UI_SIZE_8, color);
		int w = ui_number_width(g.race_ranks[i].points, UI_SIZE_8);
		ui_draw_number(g.race_ranks[i].points, ui_scaled_pos(anchor, vec2i(pos.x + 280 - w, pos.y)), UI_SIZE_8, color);
		pos.y += 12;
	}
}

static void page_race_points_init(menu_t *menu) {
	menu_page_t *page = menu_push(menu, "RACE POINTS", page_race_points_draw);
	flags_add(page->layout_flags, MENU_FIXED);
	page->title_anchor = UI_POS_MIDDLE | UI_POS_CENTER;
	page->title_pos = vec2i(0, -100);
	menu_page_add_button(page, 1, "", button_race_points_continue);
}


// -----------------------------------------------------------------------------
// Championship Table

static void button_championship_points_continue(menu_t *menu, int data) {
	if (g.is_new_race_record) {
		page_hall_of_fame_init(menu);
	}
	else if (g.race_type == RACE_TYPE_CHAMPIONSHIP) {
		race_next();
	}
	else {
		menu_confirm(menu, "", "RESTART RACE", "RESTART", "QUIT", button_quit_confirm);
	}
}

static void page_championship_points_draw(menu_t *menu, int data) {
	menu_page_t *page = &menu->pages[menu->index];
	vec2i_t pos = page->title_pos;
	pos.x -= 140;
	pos.y += 32;
	ui_pos_t anchor = UI_POS_MIDDLE | UI_POS_CENTER;

	ui_draw_text("PILOT NAME", ui_scaled_pos(anchor, pos), UI_SIZE_8, UI_COLOR_ACCENT);
	ui_draw_text("POINTS", ui_scaled_pos(anchor, vec2i(pos.x + 222, pos.y)), UI_SIZE_8, UI_COLOR_ACCENT);

	pos.y += 24;

	for (int i = 0; i < len(g.championship_ranks); i++) {
		rgba_t color = g.championship_ranks[i].pilot == g.pilot ? UI_COLOR_ACCENT : UI_COLOR_DEFAULT;
		ui_draw_text(def.pilots[g.championship_ranks[i].pilot].name, ui_scaled_pos(anchor, pos), UI_SIZE_8, color);
		int w = ui_number_width(g.championship_ranks[i].points, UI_SIZE_8);
		ui_draw_number(g.championship_ranks[i].points, ui_scaled_pos(anchor, vec2i(pos.x + 280 - w, pos.y)), UI_SIZE_8, color);
		pos.y += 12;
	}
}

static void page_championship_points_init(menu_t *menu) {
	menu_page_t *page = menu_push(menu, "CHAMPIONSHIP TABLE", page_championship_points_draw);
	flags_add(page->layout_flags, MENU_FIXED);
	page->title_anchor = UI_POS_MIDDLE | UI_POS_CENTER;
	page->title_pos = vec2i(0, -100);
	menu_page_add_button(page, 1, "", button_championship_points_continue);
}


// -----------------------------------------------------------------------------
// Hall of Fame

static highscores_entry_t hs_new_entry = {
	.time = 0,
	.name = ""
};
static const char *hs_charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static int hs_char_index = 0;
static bool hs_entry_complete = false;

static void hall_of_fame_draw_name_entry(menu_t *menu, ui_pos_t anchor, vec2i_t pos) {
	int entry_len = strlen(hs_new_entry.name);
	int entry_width = ui_text_width(hs_new_entry.name, UI_SIZE_16);

	vec2i_t c_pos = ui_scaled_pos(anchor, vec2i(pos.x + entry_width, pos.y));
	int c_first = 0;
	int c_last = 38;
	if (entry_len == 0) {
		c_last = 37;
	}
	else if (entry_len == 3) {
		c_first = 36;
	}

	if (input_pressed(A_MENU_UP)) {
		hs_char_index++;
	}
	else if (input_pressed(A_MENU_DOWN)) {
		hs_char_index--;
	}

	if (hs_char_index < c_first) {
		hs_char_index = c_last-1;
	}
	if (hs_char_index >= c_last) {
		hs_char_index = c_first;
	}

	// DEL
	if (hs_char_index == 36) {
		ui_draw_icon(UI_ICON_DEL, c_pos, UI_COLOR_ACCENT);
		if (input_pressed(A_MENU_SELECT)) {
			sfx_play(SFX_MENU_SELECT);
			if (entry_len > 0) {
				hs_new_entry.name[entry_len-1] = '\0';
			}
		}
	}

	// END
	else if (hs_char_index == 37) {
		ui_draw_icon(UI_ICON_END, c_pos, UI_COLOR_ACCENT);
		if (input_pressed(A_MENU_SELECT)) {
			hs_entry_complete = true;
		}
	}

	// A-Z, 0-9
	else {
		char selector[2] = {hs_charset[hs_char_index], '\0'};
		ui_draw_text(selector, c_pos, UI_SIZE_16, UI_COLOR_ACCENT);

		if (input_pressed(A_MENU_SELECT)) {
			sfx_play(SFX_MENU_SELECT);
			hs_new_entry.name[entry_len] = hs_charset[hs_char_index];
			hs_new_entry.name[entry_len+1] = '\0';
		}
	}

	ui_draw_text(hs_new_entry.name, ui_scaled_pos(anchor, pos), UI_SIZE_16, UI_COLOR_ACCENT);
}

static void page_hall_of_fame_draw(menu_t *menu, int data) {
	// FIXME: doing this all in the draw() function leads to all kinds of
	// complications

	highscores_t *hs = &save.highscores[g.race_class][g.circut][g.highscore_tab];
	
	if (hs_entry_complete) {
		sfx_play(SFX_MENU_SELECT);
		strncpy(save.highscores_name, hs_new_entry.name, 4);
		save.is_dirty = true;
		
		// Insert new highscore entry into the save struct
		highscores_entry_t temp_entry = hs->entries[0];
		for (int i = 0; i < NUM_HIGHSCORES; i++) {
			if (hs_new_entry.time < hs->entries[i].time) {
				for (int j = NUM_HIGHSCORES - 2; j >= i; j--) {
					hs->entries[j+1] = hs->entries[j];
				}
				hs->entries[i] = hs_new_entry;
				break;
			}
		}
		save.is_dirty = true;

		if (g.race_type == RACE_TYPE_CHAMPIONSHIP) {
			race_next();
		}
		else {
			menu_reset(menu); // Can't go back!
			menu_confirm(menu, "", "RESTART RACE", "RESTART", "QUIT", button_restart_or_quit);
		}
		return;
	}

	menu_page_t *page = &menu->pages[menu->index];
	vec2i_t pos = page->title_pos;
	pos.x -= 120;
	pos.y += 48;
	ui_pos_t anchor = UI_POS_MIDDLE | UI_POS_CENTER;

	bool has_shown_new_entry = false;
	for (int i = 0, j = 0; i < NUM_HIGHSCORES; i++, j++) {
		if (!has_shown_new_entry && hs_new_entry.time < hs->entries[i].time) {
			hall_of_fame_draw_name_entry(menu, anchor, pos);
			ui_draw_time(hs_new_entry.time, ui_scaled_pos(anchor, vec2i(pos.x + 120, pos.y)), UI_SIZE_16, UI_COLOR_DEFAULT);
			has_shown_new_entry = true;
			j--;
		}
		else {
			ui_draw_text(hs->entries[j].name, ui_scaled_pos(anchor, pos), UI_SIZE_16, UI_COLOR_DEFAULT);
			ui_draw_time(hs->entries[j].time, ui_scaled_pos(anchor, vec2i(pos.x + 120, pos.y)), UI_SIZE_16, UI_COLOR_DEFAULT);
		}
		pos.y += 24;
	}
}

static void page_hall_of_fame_init(menu_t *menu) {
	menu_reset(menu); // Can't go back!
	menu_page_t *page = menu_push(menu, "HALL OF FAME", page_hall_of_fame_draw);
	flags_add(page->layout_flags, MENU_FIXED);
	page->title_anchor = UI_POS_MIDDLE | UI_POS_CENTER;
	page->title_pos = vec2i(0, -100);

	hs_new_entry.time = g.race_time;
	strncpy(hs_new_entry.name, save.highscores_name, 4);
	hs_char_index = 0;
	hs_entry_complete = false;
}



// -----------------------------------------------------------------------------
// Text scroller

static char * const *text_scroll_lines;
static int text_scroll_lines_len;
static double text_scroll_start_time;

static void text_scroll_menu_draw(menu_t *menu, int data) {
	double time = system_time() - text_scroll_start_time;
	int scale = ui_get_scale();
	int speed = 32;
	vec2i_t screen = render_size();
	vec2i_t pos = vec2i(screen.x / 2, screen.y - time * scale * speed);

	for (int i = 0; i < text_scroll_lines_len; i++) {
		const char *line = text_scroll_lines[i];

		if (line[0] == '#') {
			pos.y += 48 * scale;
			ui_draw_text_centered(line + 1, pos, UI_SIZE_16, UI_COLOR_ACCENT);
			pos.y += 32 * scale;
		}
		else {
			ui_draw_text_centered(line, pos, UI_SIZE_8, UI_COLOR_DEFAULT);	
			pos.y += 12 * scale;
		}
	}
}

menu_t *text_scroll_menu_init(char * const *lines, int len) {
	text_scroll_lines = lines;
	text_scroll_lines_len = len;
	text_scroll_start_time = system_time();

	menu_reset(ingame_menu);

	menu_page_t *page = menu_push(ingame_menu, "", text_scroll_menu_draw);
	menu_page_add_button(page, 1, "", button_quit_confirm);
	return ingame_menu;
}
