#ifndef GAME_H
#define GAME_H

#include "../types.h"

#include "droid.h"
#include "ship.h"
#include "camera.h"
#include "track.h"

#define NUM_AI_OPPONENTS 7
#define NUM_PILOTS_PER_TEAM 2
#define NUM_NON_BONUS_CIRCUTS 6
#define NUM_MUSIC_TRACKS 11
#define NUM_HIGHSCORES 5

#define NUM_LAPS 3
#define NUM_LIVES 3
#define QUALIFYING_RANK 3
#define SAVE_DATA_MAGIC 0x64736f77

typedef enum {
	A_UP,
	A_DOWN,
	A_LEFT,
	A_RIGHT,
	A_BRAKE_LEFT,
	A_BRAKE_RIGHT,
	A_THRUST,
	A_FIRE,
	A_CHANGE_VIEW,
	NUM_GAME_ACTIONS,

	A_MENU_UP,
	A_MENU_DOWN,
	A_MENU_LEFT,
	A_MENU_RIGHT,
	A_MENU_BACK,
	A_MENU_SELECT,
	A_MENU_START,
	A_MENU_QUIT,
} action_t;


typedef enum {
	GAME_SCENE_INTRO,
	GAME_SCENE_TITLE,
	GAME_SCENE_MAIN_MENU,
	GAME_SCENE_HIGHSCORES,
	GAME_SCENE_RACE,
	GAME_SCENE_NONE,
	NUM_GAME_SCENES
} game_scene_t;

enum race_class {
	RACE_CLASS_VENOM,
	RACE_CLASS_RAPIER,
	NUM_RACE_CLASSES
};

enum race_type {
	RACE_TYPE_CHAMPIONSHIP,
	RACE_TYPE_SINGLE,
	RACE_TYPE_NETWORK,
	RACE_TYPE_TIME_TRIAL,
	NUM_RACE_TYPES,
};

enum highscore_tab {
	HIGHSCORE_TAB_TIME_TRIAL,
	HIGHSCORE_TAB_RACE,
	NUM_HIGHSCORE_TABS
};

enum pilot {
	PILOT_JOHN_DEKKA,
	PILOT_DANIEL_CHANG,
	PILOT_ARIAL_TETSUO,
	PILOT_ANASTASIA_CHEROVOSKI,
	PILOT_KEL_SOLAAR,
	PILOT_ARIAN_TETSUO,
	PILOT_SOFIA_DE_LA_RENTE,
	PILOT_PAUL_JACKSON,
	NUM_PILOTS
};

enum team {
	TEAM_AG_SYSTEMS,
	TEAM_AURICOM,
	TEAM_QIREX,
	TEAM_FEISAR,
	NUM_TEAMS
};

enum circut {
	CIRCUT_ALTIMA_VII,
	CIRCUT_KARBONIS_V,
	CIRCUT_TERRAMAX,
	CIRCUT_KORODERA,
	CIRCUT_ARRIDOS_IV,
	CIRCUT_SILVERSTREAM,
	CIRCUT_FIRESTAR,
	NUM_CIRCUTS
};


// Game definitions

typedef struct {
	char *name;
} race_class_t;

typedef struct {
	char *name;
} race_type_t;

typedef struct {
	char *name;
	char *portrait;
	int logo_model;
	int team;
} pilot_t;

typedef struct {
	float thrust_max;
	float thrust_magnitude;
	bool fight_back;
} ai_setting_t;

typedef struct {
	float mass;
	float thrust_max;
	float resistance;
	float turn_rate;
	float turn_rate_max;
	float skid;
} team_attributes_t;

typedef struct {
	char *name;
	int logo_model;
	int pilots[NUM_PILOTS_PER_TEAM];
	team_attributes_t attributes[NUM_RACE_CLASSES];
} team_t;

typedef struct {
	char *path;
	float start_line_pos;
	float behind_speed;
	float spread_base;
	float spread_factor;
	float sky_y_offset;
} circut_settings_t;

typedef struct {
	char *name;
	bool is_bonus_circut;
	circut_settings_t settings[NUM_RACE_CLASSES];
} circut_t;

typedef struct {
	char *path;
	char *name;
} music_track_t;

typedef struct {
	race_class_t race_classes[NUM_RACE_CLASSES];
	race_type_t race_types[NUM_RACE_TYPES];
	pilot_t pilots[NUM_PILOTS];
	team_t teams[NUM_TEAMS];
	ai_setting_t ai_settings[NUM_RACE_CLASSES][NUM_AI_OPPONENTS];
	circut_t circuts[NUM_CIRCUTS];
	int ship_model_to_pilot[NUM_PILOTS];
	int race_points_for_rank[NUM_PILOTS];
	music_track_t music[NUM_MUSIC_TRACKS];
	char *credits[104];
	struct {
		char *venom[15];
		char *venom_all_circuts[19];
		char *rapier[26];
		char *rapier_all_circuts[24];
	} congratulations;
} game_def_t;



// Running game data

typedef struct {
	uint16_t pilot;
	uint16_t points;
} pilot_points_t;

typedef struct {
	float frame_time;
	float frame_rate;
	
	int race_class;
	int race_type;
	int highscore_tab;
	int team;
	unsigned short pilot;
	int circut;
	bool is_attract_mode;
	bool show_credits;

	bool is_new_lap_record;
	bool is_new_race_record;
	float best_lap;
	float race_time;
	int lives;
	int race_position;
	
	float lap_times[NUM_PILOTS][NUM_LAPS];
	pilot_points_t race_ranks[NUM_PILOTS];
	pilot_points_t championship_ranks[NUM_PILOTS];

	camera_t camera;
	droid_t droid;
	ship_t ships[NUM_PILOTS];
	track_t track;
} game_t;



// Save Data

typedef struct {
	char name[4];
	float time;
} highscores_entry_t;

typedef struct {
	highscores_entry_t entries[NUM_HIGHSCORES];
	float lap_record;
} highscores_t;

typedef struct {
	uint32_t magic;
	bool is_dirty;

	float sfx_volume;
	float music_volume;
	float internal_roll;
	uint8_t ui_scale;
	bool show_fps;
	bool fullscreen;
	int screen_res;
	int post_effect;
	float screen_shake;
	bool enable_force_feedback;
	int network_interface;

	uint32_t has_rapier_class;
	uint32_t has_bonus_circuts;
	
	uint8_t buttons[NUM_GAME_ACTIONS][2];
	float analog_response;

	char highscores_name[4];
	highscores_t highscores[NUM_RACE_CLASSES][NUM_CIRCUTS][NUM_HIGHSCORE_TABS];
} save_t;




extern const game_def_t def;
extern game_t g;
extern save_t save;

void game_init(void);
void game_set_scene(game_scene_t scene);
void game_reset_championship(void);
void game_update(void);

#endif
