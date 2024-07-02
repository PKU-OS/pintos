#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H
/** Implemented from summary @:
 * https://pkuflyingpig.gitbook.io/pintos/appendix/4.4bsd-scheduler#fixed-point-real-arithmetic */

typedef int fixed_t;

/** Number of bits to shift, i.e. q in fixed point format p.q . */
#define BITS 14

/** Equivalent with 2 to the BITS power. */
#define FACTOR (1 << BITS)

/** Convert int to p.q fixed point integer.  */
#define INT_TO_FIXED(N) (N * (FACTOR))

/** Convert p.q fixed point integer to int, rounded down. */
#define FIXED_TO_INT_ROUND_DOWN(X) (X / (FACTOR))

/** Convert p.q fixed point integer to int rounded to nearest int. */
#define FIXED_TO_NEAREST_INT(X) (X >= 0 ? \
                                ((X + ((FACTOR) / 2)) / (FACTOR)) : \
                                ((X - ((FACTOR) / 2)) / (FACTOR)))

/** Add two fixed point integers. */
#define FIXED_ADD(X, Y) (X + Y)

/** Add int to fixed point integer. 
 *  X: fixed point, N: int. */
#define FIXED_ADD_INT(X, N) (X + (N * (FACTOR)))

/** Subtract two fixed point integers. */
#define FIXED_SUBTRACT(X, Y) (X - Y)

/** Subtract int from fixed point integer. 
 *  X: fixed point, N: int. */
#define FIXED_SUBTRACT_INT(X, N) (X - (N * FACTOR))

/** Multiply two fixed point integers. */
#define FIXED_MULTIPLY(X, Y) (((int64_t) X) * Y / FACTOR)

/** Multiply fixed point integer by int.
 *  X: fixed point, N: int. */
#define FIXED_MULTIPLY_INT(X, N) (X * N)

/** Divide two fixed point integers. */
#define FIXED_DIVIDE(X, Y) (((int64_t) X) * FACTOR / Y)

/** Divide fixed point integer by int. 
 *  X: fixed point, N: int. */
#define FIXED_DIVIDE_INT(X, N) (X / N)
#endif /**< threads/fixed_point.h */
