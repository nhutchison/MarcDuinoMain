/***************************************
 * 	random functions similar to the ones from Arduino
 * 	int32_t correspond to long in the c library
 * 	int16_t correspond to int in the c library
 * 	int8_t correspond to char in the c library
 */

#include "wmath.h"
#include "stdlib.h"

void randomSeed(uint32_t seed)
{
  if (seed != 0) {
    srandom(seed);
  }
}

int32_t random_howbig(int32_t howbig)
{
  if (howbig == 0) {
    return 0;
  }
  return random() % howbig;
}

int32_t random_howsmall_howbig(int32_t howsmall, int32_t howbig)
{
  if (howsmall >= howbig) {
    return howsmall;
  }
  long diff = howbig - howsmall;
  return random_howbig(diff) + howsmall;
}

int32_t map32(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// same with smaller size integers
void randSeed(uint16_t seed)
{
  if (seed != 0) {
    srand(seed);
  }
}

int16_t rand_howbig(int16_t howbig)
{
  if (howbig == 0) {
    return 0;
  }
  return rand() % howbig;
}

int16_t rand_howsmall_howbig(int16_t howsmall, int16_t howbig)
{
  if (howsmall >= howbig) {
    return howsmall;
  }
  long diff = howbig - howsmall;
  return rand_howbig(diff) + howsmall;
}

int16_t map(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
