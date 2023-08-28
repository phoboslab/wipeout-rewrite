#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include "types.h"


#if !defined(offsetof)
	#define offsetof(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
#endif
#define member_size(type, member) sizeof(((type *)0)->member)

#define max(a,b) ({ \
		__typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a > _b ? _a : _b; \
	})

#define min(a,b) ({ \
		__typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		_a < _b ? _a : _b; \
	})

#define swap(a, b) ({ \
		__typeof__(a) tmp = a; a = b; b = tmp; \
	})

#define clamp(v, min, max) ({ \
		__typeof__(v) _v = v, _min = min, _max = max; \
		_v > _max ? _max : _v < _min ? _min : _v; \
	})
#define scale(v, in_min, in_max, out_min, out_max) ({ \
		__typeof__(v) _in_min = in_min, _out_min = out_min; \
		_out_min + ((out_max) - _out_min) * (((v) - _in_min) / ((in_max) - _in_min)); \
	})
#define lerp(a, b, t) ({ \
		__typeof__(a) _a = a; \
		_a + ((b) - _a) * (t); \
	})

#define len(A) (sizeof(A) / sizeof(A[0]))
#define clear(A) memset(A, 0, sizeof(A))


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define die(...) \
	printf("Abort at " TOSTRING(__FILE__) " line " TOSTRING(__LINE__) ": " __VA_ARGS__); \
	printf("\n"); \
	exit(1)

#define error_if(TEST, ...) \
	if (TEST) { \
		die(__VA_ARGS__); \
	}


#define flags_add(FLAGS, F)  (FLAGS |= (F))
#define flags_rm(FLAGS, F)   (FLAGS &= ~(F))
#define flags_is(FLAGS, F)   ((FLAGS & (F)) == (F))
#define flags_any(FLAGS, F)  (FLAGS & (F))
#define flags_not(FLAGS, F)  ((FLAGS & (F)) != (F))
#define flags_none(FLAGS, F) ((FLAGS & (F)) == 0)
#define flags_set(FLAGS, F)  (FLAGS = (F))

	

char *get_path(const char *dir, const char *file);
bool str_starts_with(const char *haystack, const char *needle);
float rand_float(float min, float max);
int32_t rand_int(int32_t min, int32_t max); 

bool file_exists(const char *path);
uint8_t *file_load(const char *path, uint32_t *bytes_read);
uint32_t file_store(const char *path, void *bytes, int32_t len);


#define sort(LIST, LEN, COMPARE_FUNC) \
	for (uint32_t sort_i = 1, sort_j; sort_i < (LEN); sort_i++) { \
		sort_j = sort_i; \
		__typeof__((LIST)[0]) sort_temp = (LIST)[sort_j]; \
		while (sort_j > 0 && COMPARE_FUNC(&(LIST)[sort_j-1], &sort_temp)) { \
			(LIST)[sort_j] = (LIST)[sort_j-1]; \
			sort_j--; \
		} \
		(LIST)[sort_j] = sort_temp; \
	}

#define shuffle(LIST, LEN) \
	for (int i = (LEN) - 1; i > 0; i--) { \
		int j = rand_int(0, i+1); \
		swap((LIST)[i], (LIST)[j]); \
	}


static inline uint8_t get_u8(uint8_t *bytes, uint32_t *p) {
	return bytes[(*p)++];
}

static inline uint16_t get_u16(uint8_t *bytes, uint32_t *p) {
	uint16_t v = 0;
	v |= bytes[(*p)++] << 8;
	v |= bytes[(*p)++] << 0;
	return v;
}

static inline uint32_t get_u32(uint8_t *bytes, uint32_t *p) {
	uint32_t v = 0;
	v |= bytes[(*p)++] << 24;
	v |= bytes[(*p)++] << 16;
	v |= bytes[(*p)++] <<  8;
	v |= bytes[(*p)++] <<  0;
	return v;
}

static inline uint16_t get_u16_le(uint8_t *bytes, uint32_t *p) {
	uint16_t v = 0;
	v |= bytes[(*p)++] << 0;
	v |= bytes[(*p)++] << 8;
	return v;
}

static inline uint32_t get_u32_le(uint8_t *bytes, uint32_t *p) {
	uint32_t v = 0;
	v |= bytes[(*p)++] <<  0;
	v |= bytes[(*p)++] <<  8;
	v |= bytes[(*p)++] << 16;
	v |= bytes[(*p)++] << 24;
	return v;
}

#define get_i8(BYTES, P) ((int8_t)get_u8(BYTES, P))
#define get_i16(BYTES, P) ((int16_t)get_u16(BYTES, P))
#define get_i16_le(BYTES, P) ((int16_t)get_u16_le(BYTES, P))
#define get_i32(BYTES, P) ((int32_t)get_u32(BYTES, P))
#define get_i32_le(BYTES, P) ((int32_t)get_u32_le(BYTES, P))

#endif
