#include "utils.h"
#include <stdint.h>

int fp_from_int(int x)
{
  return x * THREADS_FP_FRAG;
}

int fp_add(int fx, int fy)
{
  return fx + fy;
}

int fp_mul(int x, int y)
{
  return ((int64_t)x) * y / THREADS_FP_FRAG;
}

int fp_div(int x, int y)
{
  return ((int64_t)x) * THREADS_FP_FRAG / y;
}

int fp_to_int_rounded(int x)
{
  if (x >= 0)
  {
    return (x + ((THREADS_FP_FRAG) / 2)) / (THREADS_FP_FRAG);
  }
  else
  {
    return (x - ((THREADS_FP_FRAG) / 2)) / (THREADS_FP_FRAG);
  }
}