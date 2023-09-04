#ifndef MEM_H
#define MEM_H

#include "types.h"

#define MEM_TEMP_OBJECTS_MAX 8
#define MEM_HUNK_BYTES (4 * 1024 * 1024)

void *mem_bump(uint32_t size);
void *mem_mark(void);
void mem_reset(void *p);

void *mem_temp_alloc(uint32_t size);
void mem_temp_free(void *p);
void mem_temp_check(void);

#endif
