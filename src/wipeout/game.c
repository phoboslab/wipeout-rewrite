#include <string.h>

#include "../mem.h"
#include "../utils.h"
#include "../system.h"
#include "../platform.h"
#include "../input.h"

#include "game.h"
#include "ship.h"
#include "weapon.h"
#include "droid.h"
#include "object.h"
#include "hud.h"
#include "game.h"
#include "sfx.h"
#include "ui.h"
#include "particle.h"
#include "race.h"
#include "main_menu.h"
#include "title.h"
#include "intro.h"

#define TURN_ACCEL(V) NTSC_ACCELERATION(ANGLE_NORM_TO_RADIAN(FIXED_TO_FLOAT(YAW_VELOCITY(V))))
#define TURN_VEL(V)   NTSC_VELOCITY(ANGLE_NORM_TO_RADIAN(FIXED_TO_FLOAT(YAW_VELOCITY(V))))

const game_def_t def = {
	.race_classes = {
		[RACE_CLASS_VENOM] =  {.name = "VENOM CLASS"},
		[RACE_CLASS_RAPIER] = {.name = "RAPIER CLASS"},
	},

	.race_types = {
		[RACE_TYPE_CHAMPIONSHIP] = {.name = "CHAMPIONSHIP RACE"},
		[RACE_TYPE_SINGLE]       = {.name = "SINGLE RACE"},
		[RACE_TYPE_TIME_TRIAL]   = {.name = "TIME TRIAL"},
	},

	.pilots = {
		[PILOT_JOHN_DEKKA]           = {.name = "JOHN DEKKA",           .portrait = "wipeout/textures/dekka.cmp", .team = 0, .logo_model = 0},
		[PILOT_DANIEL_CHANG]         = {.name = "DANIEL CHANG",         .portrait = "wipeout/textures/chang.cmp", .team = 0, .logo_model = 4},
		[PILOT_ARIAL_TETSUO]         = {.name = "ARIAL TETSUO",         .portrait = "wipeout/textures/arial.cmp", .team = 1, .logo_model = 6},
		[PILOT_ANASTASIA_CHEROVOSKI] = {.name = "ANASTASIA CHEROVOSKI", .portrait = "wipeout/textures/anast.cmp", .team = 1, .logo_model = 7},
		[PILOT_KEL_SOLAAR]           = {.name = "KEL SOLAAR",           .portrait = "wipeout/textures/solar.cmp", .team = 2, .logo_model = 2},
		[PILOT_ARIAN_TETSUO]         = {.name = "ARIAN TETSUO",         .portrait = "wipeout/textures/arian.cmp", .team = 2, .logo_model = 5},
		[PILOT_SOFIA_DE_LA_RENTE]    = {.name = "SOFIA DE LA RENTE",    .portrait = "wipeout/textures/sophi.cmp", .team = 3, .logo_model = 1},
		[PILOT_PAUL_JACKSON]         = {.name = "PAUL JACKSON",         .portrait = "wipeout/textures/paul.cmp",  .team = 3, .logo_model = 3},
	},

	.ship_model_to_pilot = {6, 4, 7, 1, 5, 2, 3, 0},
	.race_points_for_rank = {9, 7, 5, 3, 2, 1, 0, 0},

	// SHIP ATTRIBUTES
	//               TEAM 1   TEAM 2   TEAM 3   TEAM 4
	// Acceleration:    ***    *****       **     ****
	//    Top Speed:   ****       **     ****      ***
	//       Armour:  *****      ***     ****       **
	//    Turn Rate:     **     ****      ***    *****

	.teams = {
		[TEAM_AG_SYSTEMS] = {
			.name = "AG SYSTEMS",
			.logo_model = 2,
			.pilots = {0, 1},
			.attributes = {
				[RACE_CLASS_VENOM]  = {.mass = 150, .thrust_max =  790, .resistance = 140, .turn_rate = TURN_ACCEL(160), .turn_rate_max = TURN_VEL(2560), .skid = 12},
				[RACE_CLASS_RAPIER] = {.mass = 150, .thrust_max = 1200, .resistance = 140, .turn_rate = TURN_ACCEL(160), .turn_rate_max = TURN_VEL(2560), .skid = 10},
			},
		},
		[TEAM_AURICOM] = {
			.name = "AURICOM",
			.logo_model = 3,
			.pilots = {2, 3},
			.attributes = {
				[RACE_CLASS_VENOM]  = {.mass = 150, .thrust_max =  850, .resistance = 134, .turn_rate = TURN_ACCEL(140), .turn_rate_max = TURN_VEL(1920), .skid = 20},
				[RACE_CLASS_RAPIER] = {.mass = 150, .thrust_max = 1400, .resistance = 140, .turn_rate = TURN_ACCEL(120), .turn_rate_max = TURN_VEL(1920), .skid = 14},
			},
		},
		[TEAM_QIREX] = {
			.name = "QIREX",
			.logo_model = 1,
			.pilots = {4, 5},
			.attributes = {
				[RACE_CLASS_VENOM]  = {.mass = 150, .thrust_max =  850, .resistance = 140, .turn_rate = TURN_ACCEL(120), .turn_rate_max = TURN_VEL(1920), .skid = 24},
				[RACE_CLASS_RAPIER] = {.mass = 150, .thrust_max = 1400, .resistance = 130, .turn_rate = TURN_ACCEL(140), .turn_rate_max = TURN_VEL(1920), .skid = 16},
			},
		},
		[TEAM_FEISAR] = {
			.name = "FEISAR",
			.logo_model = 0,
			.pilots = {6, 7},
			.attributes = {
				[RACE_CLASS_VENOM]  = {.mass = 150, .thrust_max =  790, .resistance = 134, .turn_rate = TURN_ACCEL(180), .turn_rate_max = TURN_VEL(2560), .skid = 12},
				[RACE_CLASS_RAPIER] = {.mass = 150, .thrust_max = 1200, .resistance = 130, .turn_rate = TURN_ACCEL(180), .turn_rate_max = TURN_VEL(2560), .skid =  8},
			},
		},
	},

	.ai_settings = {
		[RACE_CLASS_VENOM] = {
			{.thrust_max = 2550, .thrust_magnitude = 44, .fight_back = 1},
			{.thrust_max = 2600, .thrust_magnitude = 45, .fight_back = 1},
			{.thrust_max = 2630, .thrust_magnitude = 45, .fight_back = 1},
			{.thrust_max = 2660, .thrust_magnitude = 46, .fight_back = 1},
			{.thrust_max = 2700, .thrust_magnitude = 47, .fight_back = 1},
			{.thrust_max = 2720, .thrust_magnitude = 48, .fight_back = 1},
			{.thrust_max = 2750, .thrust_magnitude = 49, .fight_back = 1},
		},
		[RACE_CLASS_RAPIER] = {
			{.thrust_max = 3750, .thrust_magnitude = 50, .fight_back = 1},
			{.thrust_max = 3780, .thrust_magnitude = 53, .fight_back = 1},
			{.thrust_max = 3800, .thrust_magnitude = 55, .fight_back = 1},
			{.thrust_max = 3850, .thrust_magnitude = 57, .fight_back = 1},
			{.thrust_max = 3900, .thrust_magnitude = 60, .fight_back = 1},
			{.thrust_max = 3950, .thrust_magnitude = 62, .fight_back = 1},
			{.thrust_max = 4000, .thrust_magnitude = 65, .fight_back = 1},
		},
	},

	.circuts = {
		[CIRCUT_ALTIMA_VII] = {
			.name = "ALTIMA VII",
			.is_bonus_circut = false,
			.settings = {
				[RACE_CLASS_VENOM]  = {.path = "wipeout/track02/", .start_line_pos = 27, .behind_speed = 300, .spread_base = 80, .spread_factor = 20, .sky_y_offset = -2520},
				[RACE_CLASS_RAPIER] = {.path = "wipeout/track03/", .start_line_pos = 27, .behind_speed = 500, .spread_base = 80, .spread_factor = 11, .sky_y_offset = -1930},
			}
		},
		[CIRCUT_KARBONIS_V] = {
			.name = "KARBONIS V",
			.is_bonus_circut = false,
			.settings = {
				[RACE_CLASS_VENOM]  = {.path = "wipeout/track04/", .start_line_pos = 16, .behind_speed = 200, .spread_base = 10, .spread_factor =  8, .sky_y_offset = -5000},
				[RACE_CLASS_RAPIER] = {.path = "wipeout/track05/", .start_line_pos = 16, .behind_speed = 500, .spread_base = 10, .spread_factor =  8, .sky_y_offset = -5000},
			}
		},
		[CIRCUT_TERRAMAX] = {
			.name = "TERRAMAX",
			.is_bonus_circut = false,
			.settings = {
				[RACE_CLASS_VENOM]  = {.path = "wipeout/track01/", .start_line_pos = 27, .behind_speed = 350, .spread_base = 60, .spread_factor = 11, .sky_y_offset =  -820},
				[RACE_CLASS_RAPIER] = {.path = "wipeout/track06/", .start_line_pos = 27, .behind_speed = 500, .spread_base = 10, .spread_factor =  8, .sky_y_offset =     0},
			}
		},
		[CIRCUT_KORODERA] = {
			.name = "KORODERA",
			.is_bonus_circut = false,
			.settings = {
				[RACE_CLASS_VENOM]  = {.path = "wipeout/track12/", .start_line_pos = 16, .behind_speed = 450, .spread_base = 40, .spread_factor = 11, .sky_y_offset = -2120},
				[RACE_CLASS_RAPIER] = {.path = "wipeout/track07/", .start_line_pos = 16, .behind_speed = 500, .spread_base = 30, .spread_factor = 11, .sky_y_offset = -2260},
			}
		},
		[CIRCUT_ARRIDOS_IV] = {
			.name = "ARRIDOS IV",
			.is_bonus_circut = false,
			.settings = {
				[RACE_CLASS_VENOM]  = {.path = "wipeout/track08/", .start_line_pos = 16, .behind_speed = 350, .spread_base = 80, .spread_factor = 15, .sky_y_offset =   -40},
				[RACE_CLASS_RAPIER] = {.path = "wipeout/track11/", .start_line_pos = 16, .behind_speed = 450, .spread_base = 30, .spread_factor = 11, .sky_y_offset =  -240},
			}
		},
		[CIRCUT_SILVERSTREAM] = {
			.name = "SILVERSTREAM",
			.is_bonus_circut = false,
			.settings = {
				[RACE_CLASS_VENOM]  = {.path = "wipeout/track09/", .start_line_pos = 16, .behind_speed = 150, .spread_base = 10, .spread_factor =  8, .sky_y_offset = -2700},
				[RACE_CLASS_RAPIER] = {.path = "wipeout/track13/", .start_line_pos = 16, .behind_speed = 150, .spread_base = 10, .spread_factor =  8, .sky_y_offset = -2700},
			}
		},
		[CIRCUT_FIRESTAR] = {
			.name = "FIRESTAR",
			.is_bonus_circut = true,
			.settings = {
				[RACE_CLASS_VENOM]  = {.path = "wipeout/track10/", .start_line_pos = 27, .behind_speed = 200, .spread_base = 40, .spread_factor = 11, .sky_y_offset =     0},
				[RACE_CLASS_RAPIER] = {.path = "wipeout/track14/", .start_line_pos = 27, .behind_speed = 500, .spread_base = 40, .spread_factor = 11, .sky_y_offset =     0},
			}
		},
	},
	.music = {
		{.path = "wipeout/music/track01.qoa", .name = "CAIRODROME"},
		{.path = "wipeout/music/track02.qoa", .name = "CARDINAL DANCER"},
		{.path = "wipeout/music/track03.qoa", .name = "COLD COMFORT"},
		{.path = "wipeout/music/track04.qoa", .name = "DOH T"},
		{.path = "wipeout/music/track05.qoa", .name = "MESSIJ"},
		{.path = "wipeout/music/track06.qoa", .name = "OPERATIQUE"},
		{.path = "wipeout/music/track07.qoa", .name = "TENTATIVE"},
		{.path = "wipeout/music/track08.qoa", .name = "TRANCEVAAL"},
		{.path = "wipeout/music/track09.qoa", .name = "AFRO RIDE"},
		{.path = "wipeout/music/track10.qoa", .name = "CHEMICAL BEATS"},
		{.path = "wipeout/music/track11.qoa", .name = "WIPEOUT"},
	},
	.credits = {
		"#MANAGING DIRECTORS",
			"IAN HETHERINGTON",
			"JONATHAN ELLIS",
		"#DIRECTOR OF DEVELOPMENT",
			"JOHN WHITE",
		"#PRODUCERS",
			"DOMINIC MALLINSON",
			"ANDY YELLAND",
		"#PRODUCT MANAGER",
			"SUE CAMPBELL",
		"#GAME DESIGNER",
			"NICK BURCOMBE",
			"",
			"",
		"#PLAYSTATION VERSION",
		"#PROGRAMMERS",
			"DAVE ROSE",
			"ROB SMITH",
			"JASON DENTON",
			"STEWART SOCKETT",
		"#ORIGINAL ARTISTS",
			"NICKY CARUS WESTCOTT",
			"LAURA GRIEVE",
			"LOUISE SMITH",
			"DARREN DOUGLAS",
			"POL SIGERSON",
		"#INTRO SEQUENCE",
			"LEE CARUS WESTCOTT",
		"#CONCEPTUAL ARTIST",
			"JIM BOWERS",
		"#ADDITIONAL GRAPHIC DESIGN",
			"THE DESIGNERS REPUBLIC",
		"#MUSIC",
			"ORBITAL",
			"CHEMICAL BROTHERS",
			"LEFTFIELD",
			"COLD STORAGE",
		"#SOUND EFFECTS",
			"TIM WRIGHT",
		"#MANUAL WRITTEN BY",
			"DAMON FAIRCLOUGH",
			"NICK BURCOMBE",
		"#PACKAGING DESIGN",
			"THE DESIGNERS REPUBLIC",
			"KEITH HOPWOOD",
			"",
			"",
		"#PC VERSION",
		"#PROGRAMMERS",
			"ANDY YELLAND",
			"ANDY SATTERTHWAITE",
			"DAVE SMITH",
			"MARK KELLY",
			"JED ADAMS",
			"STEVE WARD",
			"CHRIS EDEN",
			"SALIM SIWANI",
		"#SOUND PROGRAMMING",
			"ANDY CROWLEY",
		"#MOVIE PROGRAMMING",
			"MIKE ANTHONY",
		"#CONVERSION ARTISTS",
			"JOHN DWYER",
			"GARY BURLEY",
			"",
			"",
		"#ATI 3D RAGE VERSION",
		"#PRODUCER",
			"BILL ALLEN",
		"#DEVELOPED BY",
		"#BROADSWORD INTERACTIVE LTD",
			"STEPHEN ROSE",
			"JOHN JONES STEELE",
			"",
			"",
		"#2023 REWRITE",
			"PHOBOSLAB",
			"DOMINIC SZABLEWSKI",
			"",
			"",
		"#DEVELOPMENT SECRETARY",
			"JENNIFER REES",
			"",
			"",
		"#QUALITY ASSURANCE",
			"STUART ALLEN",
			"CHRIS GRAHAM",
			"THOMAS REES",
			"BRIAN WALSH",
			"CARL BERRY",
			"MARK INMAN",
			"PAUL TWEEDLE",
			"ANTHONY CROSS",
			"EDWARD HAY",
			"ROB WOLFE",
			"",
			"",
		"#SPECIAL THANKS TO",
			"THE HACKERS TEAM MGM",
			"SOFTIMAGE",
			"SGI",
			"GLEN OCONNELL",
			"JOANNE GALVIN",
			"ALL AT PSYGNOSIS",
	},
	.congratulations = {
		.venom = {
			"#WELL DONE",
			"",
			"VENOM CLASS",
			"",
			"COMPETENCE ACHIEVED",
			"",
			"YOU HAVE NOW QUALIFIED",
			"",
			"FOR THE ULTRA FAST",
			"",
			"RAPIER CLASS",
			"",
			"WE RECOMMEND YOU",
			"",
			"SAVE YOUR CURRENT GAME",
		},
		.venom_all_circuts = {
			"#AMAZING",
			"",
			"YOU HAVE COMPLETED THE FULL",
			"",
			"VENOM CLASS CHAMPIONSHIP",
			"",
			"",
			"WELL DONE",
			"",
			"YOU ARE A GREAT PILOT",
			"",
			"",
			"",
			"NOW TAKE ON THE FULL",
			"",
			"RAPIER CLASS CHAMPIONSHIP",
			"",
			"",
			"#KEEP GOING",
		},
		.rapier = {
			"#CONGRATULATIONS",
			"",
			"RAPIER CLASS",
			"",
			"COMPETENCE ACHIEVED",
			"",
			"YOU NOW HAVE ACCESS TO THE",
			"",
			"FULL VENOM AND RAPIER",
			"",
			"CHAMPIONSHIPS WITH THE ",
			"",
			"NEWLY CONSTRUCTED CIRCUIT",
			"",
			"FIRESTAR",
			"",
			"",
			"",
			"WE RECOMMEND YOU",
			"",
			"SAVE",
			"",
			"YOUR CURRENT GAME",
			"",
			"",
			"#GOOD LUCK",
		},
		.rapier_all_circuts = {
			"#AWESOME",
			"",
			"YOU HAVE BEATEN",
			"#WIPEOUT",
			"",
			"YOU ARE A TRULY",
			"",
			"AMAZING PILOT",
			"",
			"",
			"",
			"#CONGRATULATIONS",
			"",
			"",
			"",
			"",
			"#A BIG THANKS",
			"",
			"FROM ALL OF US ON THE TEAM",
			"",
			"LOOK OUT FOR",
			"#WIPEOUT II",
			"",
			"COMING SOON",
		},
	}
};

save_t save = {
	.magic = SAVE_DATA_MAGIC,
	.is_dirty = true,

	.sfx_volume = 0.6,
	.music_volume = 0.5,
	.ui_scale = 0,
	.show_fps = false,
	.fullscreen = false,
	.screen_res = 0,
	.post_effect = 0,

	.has_rapier_class = true,  // for testing; should be false in prod
	.has_bonus_circuts = true, // for testing; should be false in prod

	.buttons = {
		[A_UP] = {INPUT_KEY_UP, INPUT_GAMEPAD_DPAD_UP},
		[A_DOWN] = {INPUT_KEY_DOWN, INPUT_GAMEPAD_DPAD_DOWN},
		[A_LEFT] = {INPUT_KEY_LEFT, INPUT_GAMEPAD_DPAD_LEFT},
		[A_RIGHT] = {INPUT_KEY_RIGHT, INPUT_GAMEPAD_DPAD_RIGHT},
		[A_BRAKE_LEFT] = {INPUT_KEY_C, INPUT_GAMEPAD_L_SHOULDER},
		[A_BRAKE_RIGHT] = {INPUT_KEY_V, INPUT_GAMEPAD_R_SHOULDER},
		[A_THRUST] = {INPUT_KEY_X, INPUT_GAMEPAD_A},
		[A_FIRE] = {INPUT_KEY_Z, INPUT_GAMEPAD_X},
		[A_CHANGE_VIEW] = {INPUT_KEY_A, INPUT_GAMEPAD_Y},
	},

	.highscores_name = {0,0,0,0},
	.highscores = {
		[RACE_CLASS_VENOM] = {
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 85.83, .entries = {{"WIP", 254.50},{"EOU", 271.17},{"TPC", 289.50},{"NOT", 294.50},{"PSX", 314.50}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 85.83, .entries = {{"MVE", 254.50},{"ALM", 271.17},{"POL", 289.50},{"NIK", 294.50},{"DAR", 314.50}}},
			},
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 55.33, .entries = {{"AJY", 159.33},{"AJS", 172.67},{"DLS", 191.00},{"MAK", 207.67},{"JED", 219.33}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 55.33, .entries = {{"DAR", 159.33},{"STU", 172.67},{"MOC", 191.00},{"DOM", 207.67},{"NIK", 219.33}}},
			},
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 57.5, .entries = {{ "JD", 171.00},{"AJC", 189.33},{"MSA", 202.67},{ "SD", 219.33},{"TIM", 232.67}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 57.5, .entries = {{"PHO", 171.00},{"ENI", 189.33},{ "XR", 202.67},{"ISI", 219.33},{ "NG", 232.67}}},
			},
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 85.17, .entries = {{"POL", 251.33},{"DAR", 263.00},{"JAS", 283.00},{"ROB", 294.67},{"DJR", 314.82}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 85.17, .entries = {{"DOM", 251.33},{"DJR", 263.00},{"MPI", 283.00},{"GOC", 294.67},{"SUE", 314.82}}},
			},
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 80.17, .entries = {{"NIK", 236.17},{"SAL", 253.17},{"DOM", 262.33},{ "LG", 282.67},{"LNK", 298.17}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 80.17, .entries = {{"NIK", 236.17},{"ROB", 253.17},{ "AM", 262.33},{"JAS", 282.67},{"DAR", 298.17}}},
			},
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 61.67, .entries = {{"HAN", 182.33},{"PER", 196.33},{"FEC", 214.83},{"TPI", 228.83},{"ZZA", 244.33}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 61.67, .entries = {{ "FC", 182.33},{"SUE", 196.33},{"ROB", 214.83},{"JEN", 228.83},{ "NT", 244.33}}},
			},
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 63.83, .entries = {{"CAN", 195.40},{"WEH", 209.23},{"AVE", 227.90},{"ABO", 239.90},{"NUS", 240.73}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 63.83, .entries = {{"DJR", 195.40},{"NIK", 209.23},{"JAS", 227.90},{"NCW", 239.90},{"LOU", 240.73}}},
			},
		},
		[RACE_CLASS_RAPIER] = {
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 69.50, .entries = {{"AJY", 200.67},{"DLS", 213.50},{"AJS", 228.67},{"MAK", 247.67},{"JED", 263.00}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 69.50, .entries = {{"NCW", 200.67},{"LEE", 213.50},{"STU", 228.67},{"JAS", 247.67},{"ROB", 263.00}}},
			},
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 47.33, .entries = {{"BOR", 134.58},{"ING", 147.00},{"HIS", 162.25},{"COR", 183.08},{ "ES", 198.25}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 47.33, .entries = {{"NIK", 134.58},{"POL", 147.00},{"DAR", 162.25},{"STU", 183.08},{"ROB", 198.25}}},
			},
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 47.83, .entries = {{"AJS", 142.08},{"DLS", 159.42},{"MAK", 178.08},{"JED", 190.25},{"AJY", 206.58}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 47.83, .entries = {{"POL", 142.08},{"JIM", 159.42},{"TIM", 178.08},{"MOC", 190.25},{ "PC", 206.58}}},
			},
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 76.75, .entries = {{"DLS", 224.17},{"DJR", 237.00},{"LEE", 257.50},{"MOC", 272.83},{"MPI", 285.17}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 76.75, .entries = {{"TIM", 224.17},{"JIM", 237.00},{"NIK", 257.50},{"JAS", 272.83},{ "LG", 285.17}}},
			},
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 65.75, .entries = {{"MAK", 191.00},{"STU", 203.67},{"JAS", 221.83},{"ROB", 239.00},{"DOM", 254.50}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 65.75, .entries = {{ "LG", 191.00},{"LOU", 203.67},{"JIM", 221.83},{"HAN", 239.00},{ "NT", 254.50}}},
			},
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 59.23, .entries = {{"JED", 156.67},{"NCW", 170.33},{"LOU", 188.83},{"DAR", 201.00},{"POL", 221.50}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 59.23, .entries = {{"STU", 156.67},{"DAV", 170.33},{"DOM", 188.83},{"MOR", 201.00},{"GAN", 221.50}}},
			},
			{
				[HIGHSCORE_TAB_RACE]       = {.lap_record = 55.00, .entries = {{ "PC", 162.42},{"POL", 179.58},{"DAR", 194.75},{"DAR", 208.92},{"MSC", 224.58}}},
				[HIGHSCORE_TAB_TIME_TRIAL] = {.lap_record = 55.00, .entries = {{"THA", 162.42},{"NKS", 179.58},{"FOR", 194.75},{"PLA", 208.92},{"YIN", 224.58}}},
			}
		}
	}
};

game_t g = {0};



struct {
	void (*init)(void);
	void (*update)(void);
} game_scenes[] = {
	[GAME_SCENE_INTRO] = {intro_init, intro_update},
	[GAME_SCENE_TITLE] = {title_init, title_update},
	[GAME_SCENE_MAIN_MENU] = {main_menu_init, main_menu_update},
	[GAME_SCENE_RACE] = {race_init, race_update},
};

static game_scene_t scene_current = GAME_SCENE_NONE;
static game_scene_t scene_next = GAME_SCENE_NONE;
static int global_textures_len = 0;
static void *global_mem_mark = 0;

void game_init(void) {
	uint32_t size;
	save_t *save_file = (save_t *)platform_load_userdata("save.dat", &size);
	if (save_file) {
		if (size == sizeof(save_t) && save_file->magic == SAVE_DATA_MAGIC) {
			printf("load save data success\n");
			memcpy(&save, save_file, sizeof(save_t));
		}
		else {
			printf("unexpected size/magic for save data");
		}
		mem_temp_free(save_file);
	}

	platform_set_fullscreen(save.fullscreen);
	render_set_resolution(save.screen_res);
	render_set_post_effect(save.post_effect);

	srand((int)(platform_now() * 100));
	
	ui_load();
	sfx_load();
	hud_load();
	ships_load();
	droid_load();
	particles_load();
	weapons_load();

	global_textures_len = render_textures_len();
	global_mem_mark = mem_mark();

	sfx_music_mode(SFX_MUSIC_PAUSED);
	sfx_music_play(rand_int(0, len(def.music)));


	// System binds; always fixed
	// Keyboard
	input_bind(INPUT_LAYER_SYSTEM, INPUT_KEY_UP, A_MENU_UP);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_KEY_DOWN, A_MENU_DOWN);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_KEY_LEFT, A_MENU_LEFT);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_KEY_RIGHT, A_MENU_RIGHT);

	input_bind(INPUT_LAYER_SYSTEM, INPUT_KEY_BACKSPACE, A_MENU_BACK);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_KEY_C, A_MENU_BACK);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_KEY_V, A_MENU_BACK);

	input_bind(INPUT_LAYER_SYSTEM, INPUT_KEY_X, A_MENU_SELECT);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_KEY_RETURN, A_MENU_START);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_KEY_ESCAPE, A_MENU_QUIT);

	// Gamepad
	input_bind(INPUT_LAYER_SYSTEM, INPUT_GAMEPAD_DPAD_UP, A_MENU_UP);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_GAMEPAD_DPAD_DOWN, A_MENU_DOWN);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_GAMEPAD_DPAD_LEFT, A_MENU_LEFT);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_GAMEPAD_DPAD_RIGHT, A_MENU_RIGHT);

	input_bind(INPUT_LAYER_SYSTEM, INPUT_GAMEPAD_L_STICK_UP, A_MENU_UP);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_GAMEPAD_L_STICK_DOWN, A_MENU_DOWN);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_GAMEPAD_L_STICK_LEFT, A_MENU_LEFT);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_GAMEPAD_L_STICK_RIGHT, A_MENU_RIGHT);

	input_bind(INPUT_LAYER_SYSTEM, INPUT_GAMEPAD_X, A_MENU_BACK);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_GAMEPAD_B, A_MENU_BACK);

	input_bind(INPUT_LAYER_SYSTEM, INPUT_GAMEPAD_A, A_MENU_SELECT);
	input_bind(INPUT_LAYER_SYSTEM, INPUT_GAMEPAD_START, A_MENU_START);
	

	// User defined, loaded from the save struct
	for (int action = 0; action < len(save.buttons); action++) {
		if (save.buttons[action][0] != INPUT_INVALID) {
			input_bind(INPUT_LAYER_USER, save.buttons[action][0], action);
		}
		if (save.buttons[action][1] != INPUT_INVALID) {
			input_bind(INPUT_LAYER_USER, save.buttons[action][1], action);
		}
	}


	game_set_scene(GAME_SCENE_INTRO);
}

void game_set_scene(game_scene_t scene) {
	sfx_reset();
	scene_next = scene;
}

void game_reset_championship(void) {
	for (int i = 0; i < len(g.championship_ranks); i++) {
		g.championship_ranks[i].points = 0;
		g.championship_ranks[i].pilot = i;
	}
	g.lives = NUM_LIVES;
}

void game_update(void) {
	double frame_start_time = platform_now();

	int sh = render_size().y;
	int scale = max(1, sh >=  720 ? sh / 360 : sh / 240);
	if (save.ui_scale && save.ui_scale < scale) {
		scale = save.ui_scale;
	}
	ui_set_scale(scale);


	if (scene_next != GAME_SCENE_NONE) {
		scene_current = scene_next;
		scene_next = GAME_SCENE_NONE;
		render_textures_reset(global_textures_len);
		mem_reset(global_mem_mark);
		system_reset_cycle_time();

		if (scene_current != GAME_SCENE_NONE) {
			game_scenes[scene_current].init();
		}
	}

	if (scene_current != GAME_SCENE_NONE) {
		game_scenes[scene_current].update();
	}

	// Fullscreen might have been toggled through alt+enter
	bool fullscreen = platform_get_fullscreen();
	if (fullscreen != save.fullscreen) {
		save.fullscreen = fullscreen;
		save.is_dirty = true;
	}

	if (save.is_dirty) {
		// FIXME: use a text based format?
		// FIXME: this should probably run async somewhere
		save.is_dirty = false;
		platform_store_userdata("save.dat", &save, sizeof(save_t));
		printf("wrote save.dat\n");
	}

	double now = platform_now();
	g.frame_time = now - frame_start_time;
	if (g.frame_time > 0) {
		g.frame_rate = ((double)g.frame_rate * 0.95) + (1.0/g.frame_time) * 0.05;
	}
}

