#ifndef wmath_h
#define wmath_h

#include <stdlib.h>			// random() is there
#include <stdint.h> 		// uint8_t and companions
//#include <math.h>


// WMath prototypes
int32_t random_howbig(int32_t);
int32_t random_howsmall_howbig(int32_t, int32_t);
void randomSeed(uint32_t);
int32_t map32(int32_t, int32_t, int32_t, int32_t, int32_t);

// same with smaller size int (int16-t) instead of longer size long (int32_t)
int16_t rand_howbig(int16_t);
int16_t rand_howsmall_howbig(int16_t, int16_t);
void randSeed(uint16_t);
int16_t map(int16_t, int16_t, int16_t, int16_t, int16_t);

#endif
