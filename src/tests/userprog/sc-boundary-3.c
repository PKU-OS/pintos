/* Invokes a system call with the system call number positioned
   such that its first byte is valid but the remaining bytes of
   the number are in invalid memory. Must kill process. */

#include <syscall-nr.h>
#include "tests/userprog/boundary.h"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  char *p = get_bad_boundary ();
  p--;
  *p = 100;

  /* Invoke the system call. */
  asm volatile ("movl %0, %%esp; int $0x30" : : "g" (p));
  fail ("should have killed process");
}
