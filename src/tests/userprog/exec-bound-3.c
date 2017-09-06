/* Invokes an exec system call with the exec string straddling a
   page boundary such that the first byte of the string is valid
   but the remainder of the string is in invalid memory. Must
   kill process. */

#include <syscall-nr.h>
#include "tests/userprog/boundary.h"
#include "tests/lib.h"
#include "tests/main.h"

void
test_main (void) 
{
  char *p = get_bad_boundary () - 1;
  *p = 'a';
  exec(p);

  /* Note: if this test fails to pass even with the official solutions,
     it's probably because memory layout has changed and p no longer
     refers to the proper page boundary. To fix the problem, uncomment
     the line below to print out the boundary address. In addition,
     add a printf line in load_segment to print out the address range
     of each segment. From that, you'll be able to figure out how to
     modify get_bad_boundary to make things work again. */

  // msg("boundary address: 0x%x", p);
  fail ("should have killed process");
}
