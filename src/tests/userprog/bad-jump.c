/* This program attempts to execute code at address 0, which is not mapped.
   This should terminate the process with a -1 exit code. */

#include "tests/lib.h"
#include "tests/main.h"

typedef int (* volatile functionptr)(void);

void
test_main (void) 
{
  functionptr fp = NULL;
  msg ("Congratulations - you have successfully called NULL: %d", 
        fp());
  fail ("should have exited with -1");
}
