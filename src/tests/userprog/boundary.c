/* Utility function for tests that try to break system calls by
   passing them data that crosses from one virtual page to
   another. */

#include <inttypes.h>
#include <round.h>
#include <string.h>
#include "tests/userprog/boundary.h"

/* Together with statements in src/lib/user/user.lds, arranges
   for the following array to be at the very end of the .bss
   segment (needed for get_bad_boundary below). */
static char dst[8192] __attribute__ ((section (".testEndmem,\"aw\",@nobits#")));

/* Returns the beginning of a page.  There are at least 2048
   modifiable bytes on either side of the pointer returned. */
void *
get_boundary_area (void) 
{
  char *p = (char *) ROUND_UP ((uintptr_t) dst, 4096);
  if (p - dst < 2048)
    p += 4096;
  return p;
}

/* Returns a copy of SRC split across the boundary between two
   pages. */
char *
copy_string_across_boundary (const char *src) 
{
  char *p = get_boundary_area ();
  p -= strlen (src) < 4096 ? strlen (src) / 2 : 4096;
  strlcpy (p, src, 4096);
  return p;
}

/* Returns an address that is invalid, but the preceding bytes
 * are all valid (the highest address in the bss segment). Used
 * to position information such that the first byte of the
 * information is valid, but not all the information is valid. */
void *
get_bad_boundary (void)
{
  /* This code assumes that dst will be in the highest page
   * allocated to the user process. */
  return (void *) ROUND_UP ((uintptr_t) (dst + sizeof(dst) - 1), 4096);
}
