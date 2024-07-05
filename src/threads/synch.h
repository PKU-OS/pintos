#ifndef THREADS_SYNCH_H
#define THREADS_SYNCH_H

#include <list.h>
#include <stdbool.h>

/** A counting semaphore. */
struct semaphore 
  {
    unsigned value;             /**< Current value. */
    struct list waiters;        /**< List of waiting threads. */
  };

void sema_init (struct semaphore *, unsigned value);
void sema_down (struct semaphore *);
bool sema_try_down (struct semaphore *);
void sema_up (struct semaphore *);
void sema_self_test (void);

/** Lock. */
struct lock 
  {
    struct thread *holder;      /**< Thread holding lock (for debugging). */
    struct semaphore semaphore; /**< Binary semaphore controlling access. */
    struct list_elem elem;
    unsigned char priority_current; /**< Copy of the highest priority of waiting threads */
  };

void lock_init (struct lock *);
void lock_acquire (struct lock *);
bool lock_try_acquire (struct lock *);
void lock_release (struct lock *);
bool lock_held_by_current_thread (const struct lock *);

/** Condition variable. */
struct condition 
  {
    struct list waiters;        /**< List of waiting threads. */
  };

void cond_init (struct condition *);
void cond_wait (struct condition *, struct lock *);
void cond_signal (struct condition *, struct lock *);
void cond_broadcast (struct condition *, struct lock *);


/** Readers / Writers Lock 
 *  This is implemented based on the description given by Arpaci-Dusseau in
 *  Operating Systems: Three Easy Pieces, pages 400-402
*/
struct rw_lock
  {
    struct semaphore lock;
    struct semaphore write_lock;
    int readers;
  };

void rw_lock_init (struct rw_lock *);
void rw_lock_acquire_readlock (struct rw_lock *);
void rw_lock_release_readlock (struct rw_lock *);
void rw_lock_acquire_writelock (struct rw_lock *);
void rw_lock_release_writelock (struct rw_lock *);


/** Optimization barrier.

   The compiler will not reorder operations across an
   optimization barrier.  See "Optimization Barriers" in the
   reference guide for more information.*/
#define barrier() asm volatile ("" : : : "memory")

#endif /**< threads/synch.h */
