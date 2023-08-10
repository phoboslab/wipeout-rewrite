#ifndef SCENE_H
#define SCENE_H

#include "image.h"
#include "camera.h"

void scene_load(const char *path, float sky_y_offset);
void scene_draw(camera_t *camera);
void scene_init();
void scene_set_start_booms(int num_lights);
void scene_init_aurora_borealis();
void scene_update();
void scene_draw();

#endif
