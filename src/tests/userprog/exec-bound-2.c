/* Invokes an exec system call with the exec string pointer argument
   positioned such that only its first byte is valid  memory (bytes 1-3
   of the pointer are invalid).  Must kill process. */

#include <syscall-nr.h>
#include "tests/userprog/boundary.h"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  char *p = get_bad_boundary () - 5;
  *((int *) p) = SYS_EXEC;
  p[4] = '!';

  /* Invoke the system call. */
  asm volatile ("movl %0, %%esp; int $0x30" : : "g" (p));
  fail ("should have killed process");
}
