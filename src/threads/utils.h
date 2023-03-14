#ifndef THREADS_UTILS_H
#define THREADS_UTILS_H

// Magic taken from https://pkuflyingpig.gitbook.io/pintos/appendix/4.4bsd-scheduler#fixed-point-real-arithmetic
#define THREADS_FP_FRAG (1 << 14)

int fp_from_int(int x);
int fp_add(int x, int y);
int fp_mul(int x, int y);
int fp_div(int x, int y);
int fp_to_int_rounded(int x);

#endif