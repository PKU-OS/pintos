#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H
/** Implemented from summary @:
 * https://pkuflyingpig.gitbook.io/pintos/appendix/4.4bsd-scheduler#fixed-point-real-arithmetic */

#define BITS 14
#define ONE_LSHIFT_BITS (1 << BITS)

#define FACTOR 16384 /** 1 << 14 */
#define INT_TO_FIXED(n) (n * (FACTOR))//ONE_LSHIFT_BITS))
#define FIXED_TO_INT_TOWARD_ZERO(x) (x / (FACTOR))//ONE_LSHIFT_BITS))
#define FIXED_TO_NEAREST_INT(x) (x >= 0 ? \
                                ((x + ((FACTOR/*ONE_LSHIFT_BITS*/) / 2)) / (FACTOR/*ONE_LSHIFT_BITS*/)) : \
                                ((x - ((FACTOR/*ONE_LSHIFT_BITS*/) / 2)) / (FACTOR/*ONE_LSHIFT_BITS*/)))
#define FIXED_ADD(x, y) (x + y)
#define FIXED_ADD_INT(x, n) (x + (n * (FACTOR)))//ONE_LSHIFT_BITS)))
#define FIXED_SUBTRACT(x, y) (x - y)
#define FIXED_SUBTRACT_INT(x, n) (x - (n * FACTOR))//ONE_LSHIFT_BITS))
#define FIXED_MULTIPLY(x, y) (((int64_t) x) * y / FACTOR)//ONE_LSHIFT_BITS)
#define FIXED_MULTIPLY_INT(x, n) (x * n)
#define FIXED_DIVIDE(x, y) (((int64_t) x) * FACTOR /*ONE_LSHIFT_BITS*/ / y)
#define FIXED_DIVIDE_INT(x, n) (x / n)
#endif /**< threads/fixed_point.h */
