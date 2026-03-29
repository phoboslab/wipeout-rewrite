#include "platform.h"
#include "system.h"
#include "utils.h"
#include "mem.h"

static char *path_assets = "";		// optionally set by -DPATH_ASSETS
static char *path_userdata = "";	// optionally set by -DPATH_USERDATA
static char *temp_path = NULL;		// buffer alloc'd in main()

void platform_exit(void) {}
vec2i_t platform_screen_size(void) {
	return vec2i(0, 0);
}
double platform_now(void) {
	return 0.0;
}
bool platform_get_fullscreen(void) {
	return false;
}
void platform_set_fullscreen(bool fullscreen) {
	(void) fullscreen;
}
void platform_set_audio_mix_cb(void (*cb)(float *buffer, uint32_t len)) {
	(void) cb;
}

FILE *platform_open_asset(const char *name, const char *mode) {
	char *path = strcat(strcpy(temp_path, path_assets), name);
	return fopen(path, mode);
}
uint8_t *platform_load_asset(const char *name, uint32_t *bytes_read) {
	char *path = strcat(strcpy(temp_path, path_assets), name);
	return file_load(path, bytes_read);
}
uint8_t *platform_load_userdata(const char *name, uint32_t *bytes_read) {
	char *path = strcat(strcpy(temp_path, path_userdata), name);
	if (!file_exists(path)) {
		*bytes_read = 0;
		return NULL;
	}
	return file_load(path, bytes_read);
}
uint32_t platform_store_userdata(const char *name, void *bytes, int32_t len) {
	char *path = strcat(strcpy(temp_path, path_userdata), name);
	return file_store(path, bytes, len);
}

int main(int argc, char *argv[]) {
	(void) argc; (void) argv;
	// Figure out the absolute asset and userdata paths. These may either be
	// supplied at build time through -DPATH_ASSETS=.. and -DPATH_USERDATA=..
	// We fall back to the current directory (i.e. just "") in this case.

	#ifdef PATH_ASSETS
		path_assets = TOSTRING(PATH_ASSETS);
	#else
		path_assets = "";
	#endif

	#ifdef PATH_USERDATA
		path_userdata = TOSTRING(PATH_USERDATA);
	#else
		path_userdata = "";
	#endif

	// Reserve some space for concatenating the asset and userdata paths with
	// local filenames.
	temp_path = mem_bump(max(strlen(path_assets), strlen(path_userdata)) + 64);

	// This is the bare minimum, which will still go through the motions of
	// loading assets, e.g.:
	//
	// load cmp wipeout/textures/drfonts.cmp
	// load: wipeout/textures/speedo.tim
	// load: wipeout/textures/target2.tim
	// load cmp wipeout/common/wicons.cmp
	// load cmp wipeout/common/allsh.cmp
	// load: wipeout/common/allsh.prm
	// load cmp wipeout/common/alcol.cmp
	// load: wipeout/common/alcol.prm
	// load: wipeout/textures/shad1.tim
	// load: wipeout/textures/shad2.tim
	// load: wipeout/textures/shad3.tim
	// load: wipeout/textures/shad4.tim
	// load cmp wipeout/common/rescu.cmp
	// load: wipeout/common/rescu.prm
	// load cmp wipeout/common/effects.cmp
	// load: wipeout/textures/target2.tim
	// load cmp wipeout/common/mine.cmp
	// load: wipeout/common/rock.prm
	// load: wipeout/common/mine.prm
	// load: wipeout/common/miss.prm
	// load: wipeout/common/shld.prm
	// load: wipeout/common/shld.prm
	// load: wipeout/common/ebolt.prm
	// open music track 1
	system_init();
	system_update();
	system_cleanup();

	return 0;
}
