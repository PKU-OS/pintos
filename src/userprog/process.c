#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "userprog/gdt.h"      /* SEL_* constants */
#include "userprog/process.h"
#include "userprog/load.h"
#include "userprog/pagedir.h"  /* pagedir_activate etc. */
#include "userprog/tss.h"      /* tss_update */
#include "filesys/file.h"
#include "threads/flags.h"     /* FLAG_* constants */
#include "threads/thread.h"
#include "threads/vaddr.h"     /* PHYS_BASE */
#include "threads/interrupt.h" /* if_ */
#include "threads/init.h"      /* power_off() */

/* Headers not yet used that you may need for various reasons. */
#include "threads/synch.h"
#include "threads/malloc.h"
#include "lib/kernel/list.h"

#include "userprog/flist.h"
#include "userprog/plist.h"

/* HACK defines code you must remove and implement in a proper way */
#define HACK

/* This function is called at boot time (threads/init.c) to initialize
 * the process subsystem. */
void process_init (void)
{
  /* Initialize the global list of processes */
  plist_init ();
  rw_lock_init (&lock_plist_rw);
}

/* Prepare for exiting the system by updating the process table */
void process_exit (int status) 
{
  rw_lock_acquire_readlock (&lock_plist_rw);
  pid_t p_id = plist_get_proc_from_tid (thread_current ()->tid);
  process_t p = plist_find (p_id);
  if (p != NULL)
  {
    sema_down (&p->sema_p);
    p->exit_status = status;
    sema_up (&p->sema_p);
  }
  rw_lock_release_readlock (&lock_plist_rw);
}

/* Print a list of all running processes. The list shall include all
 * relevant debug information in a clean, readable format. */
void process_print_list ()
{
  plist_print ();
}


/* Uses these members for communication between parent and child
 * during loading and starting a new process. 
 * Tip from: https://cs.lth.se/edaf35/labs/lab3/
 */
struct parameters_to_start_process
{
  char *command_line;
  struct semaphore sema_start_p;
  pid_t pid_new;
  tid_t parent;
  unsigned child_load_success;
};


static void
start_process (struct parameters_to_start_process* parameters) NO_RETURN;


/* Starts a new proccess by creating a new thread to run it. The
   process is loaded from the file specified in the COMMAND_LINE and
   started with the arguments on the COMMAND_LINE. The new thread may
   be scheduled (and may even exit) before process_execute() returns.
   Returns the new process's thread id, or TID_ERROR if the thread
   cannot be created. */
int
process_execute (const char *command_line)
{
  char debug_name[64];
  int command_line_size = strlen (command_line) + 1;
  tid_t thread_id = -1;
  int  process_id = -1;
  
  /* LOCAL variable will cease existence when function return! */
  struct parameters_to_start_process arguments;
  sema_init (&arguments.sema_start_p, 0);

  /* COPY command line out of parent process memory */
  arguments.command_line = malloc (command_line_size);
  strlcpy (arguments.command_line, command_line, command_line_size);

  /* Copy the name from the command line to debug name */ 
  strlcpy_first_word (debug_name, command_line, 64);
  arguments.child_load_success = 0;
  arguments.pid_new = -1;
  arguments.parent = thread_current ()->tid;
  
  /* SCHEDULES function `start_process' to run (LATER) */
  thread_id = thread_create (debug_name, PRI_DEFAULT,
                             (thread_func*)start_process, &arguments);

  /* If thread create failed, it returned -1, which is TID_ERROR. */
  if (TID_ERROR == thread_id) 
  {
    free (arguments.command_line); 
    return process_id;
  }
  
  /* Here we need to wait for start_process to load successfully */
  sema_down (&arguments.sema_start_p);

  /* If start_process() loading was successful */
  if (arguments.child_load_success)
  {
    /* Set process ID to the id of the thread we started */
    process_id = arguments.pid_new; 
  }
  else
    process_id = -1;

  /* WHICH thread may still be using this right now? */
  free (arguments.command_line);

  /* MUST be -1 if `load' in `start_process' return false */
  return process_id;
}

/* ASM version of the code to set up the main stack. */
void *setup_main_stack_asm (const char *command_line, void *esp);


/* A thread function that loads a user process and starts it
   running. */
static void
start_process (struct parameters_to_start_process* parameters)
{
  /* The last argument passed to thread_create is received here... */
  struct intr_frame if_;
  bool success;
  struct thread *t_current = thread_current ();
  struct file *executable = NULL;
  char file_name[64];
  char debug_params[64];

  strlcpy_first_word (file_name, parameters->command_line, 64);
  
  /* Copy parameters->command_line for debug purposes
   * and to avoid having them free'd by process_execute ()
   */
  strlcpy (debug_params, parameters->command_line, 64);

  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;
  
  executable = load (file_name, &if_.eip, &if_.esp); 
  success = executable != NULL ? true : false;
  
  if (success)
  {

    /*************** Notes from TDIU16 version ***************/

    /* We managed to load the new program to a process, and have
       allocated memory for a process stack. The stack top is in
       if_.esp, now we must prepare and place the arguments to main on
       the stack. */

    /* A temporary solution is to modify the stack pointer to
       "pretend" the arguments are present on the stack. A normal
       C-function expects the stack to contain, in order, the return
       address, the first argument, the second argument etc. */

   // if_.esp -= 12; /* this is a very rudimentary solution */

    /* This uses a "reference" solution in assembler that you
       can replace with C-code if you wish. */
    //if_.esp = setup_main_stack_asm(parameters->command_line, if_.esp);

    /* The stack and stack pointer should be setup correct just before
       the process start, so this is the place to dump stack content
       for debug purposes. Disable the dump when it works. */

    // dump_stack ( PHYS_BASE + 15, PHYS_BASE - if_.esp + 16 );

    /*************** END Notes from TDIU16 version ***************/
    
    /* Set the child load success flag */
    parameters->child_load_success = 1;
    
    /* Create a new process. Will be free'd in cleanup or wait */
    rw_lock_acquire_readlock (&lock_plist_rw); 
    pid_t parent = plist_get_proc_from_tid (parameters->parent);
    rw_lock_release_readlock (&lock_plist_rw); 

    /** Create a new process for the process table (plist) to manage this process. */ 
    struct process *p = process_create (t_current->tid, t_current->name, parent, executable); 
        
    if (p != NULL)
    {
      rw_lock_acquire_writelock (&lock_plist_rw);
      parameters->pid_new = plist_insert (p);
      rw_lock_release_writelock (&lock_plist_rw);
      
      /* If somehow insertion failed, free this now */
      if (-1 == parameters->pid_new)
        process_destroy (p);
    }
    else
    {
      /* Get out if we could not allocate memory for a process */
      thread_exit ();
    }

    if_.esp = setup_main_stack_asm(parameters->command_line, if_.esp);
  }
  
  /* If load fail, quit. Load may fail for several reasons.
     Some simple examples:
     - File does not exist
     - File does not contain a valid program
     - Not enough memory
  */
  if (!success)
  {
    /* Notify the parent that the child load failed. */
    parameters->child_load_success = 0;
    sema_up (&parameters->sema_start_p);
    
    /* Get out of this thread */
    thread_exit ();
  }

  /* Unblock for success */
  sema_up (&parameters->sema_start_p);

  /* Start the user process by simulating a return from an interrupt,
     implemented by intr_exit (in threads/intr-stubs.S). Because
     intr_exit takes all of its arguments on the stack in the form of
     a `struct intr_frame', we just point the stack pointer (%esp) to
     our stack frame and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();
}


/* Wait for process `child_id' to die and then return its exit
   status. If it was terminated by the kernel (i.e. killed due to an
   exception), return -1. If `child_id' is invalid or if it was not a
   child of the calling process, or if process_wait() has already been
   successfully called for the given `child_id', return -1
   immediately, without waiting.

   This function will be implemented last, after a communication
   mechanism between parent and child is established. */
int
process_wait (int child_id)
{
  int status = -1;
  struct thread *cur = thread_current ();
  
  rw_lock_acquire_readlock (&lock_plist_rw);
  pid_t pid_current = plist_get_proc_from_tid (cur->tid);
  process_t p_child = plist_find (child_id);

  /* Return -1 if the child is NULL or this is NOT our child */ 
  if (p_child == NULL || p_child->pid_parent != pid_current)
  {
    rw_lock_release_readlock (&lock_plist_rw);
    return -1;
  }
  rw_lock_release_readlock (&lock_plist_rw);

  /* Wait for the child to finish if it's still alive */
  sema_down (&p_child->sema_p_wait);
  
  rw_lock_acquire_writelock (&lock_plist_rw);
  
  /* Get the status, remove the finished child and free the memory */
  status = p_child->exit_status;
  p_child = plist_remove (child_id);
  
  rw_lock_release_writelock (&lock_plist_rw);
  
  process_destroy (p_child);
  
  return status;
}

/* Free the current process's resources. This function is called
   automatically from thread_exit() to make sure cleanup of any
   process resources is always done. That is correct behaviour. But
   know that thread_exit() is called at many places inside the kernel,
   mostly in case of some unrecoverable error in a thread.

   In such case it may happen that some data is not yet available, or
   initialized. You must make sure that nay data needed IS available
   or initialized to something sane, or else that any such situation
   is detected.
*/

void
process_cleanup (void)
{
  struct thread  *cur = thread_current ();
  uint32_t       *pd  = cur->pagedir;
  int status = -1;
  
  // debug("%s#%d: process_cleanup() ENTERED\n", cur->name, cur->tid);
  
  /* Later tests DEPEND on this output to work correct. You will have
   * to find the actual exit status in your process list. It is
   * important to do this printf BEFORE you tell the parent process
   * that you exit.  (Since the parent may be the main() function,
   * that may sometimes poweroff as soon as process_wait() returns,
   * possibly before the printf is completed.)
   */
  
  rw_lock_acquire_readlock (&lock_plist_rw);
  pid_t pid = plist_get_proc_from_tid (cur->tid);
  process_t p = plist_find (pid);

  /* Fetch the exit status of this process, mark alive status as false */
  if (p !=  NULL)
  {
    sema_down (&p->sema_p);
    status = p->exit_status;
    p->is_alive = 0;
     
    printf ("%s: exit(%d)\n", thread_name (), status);

    /* Notify other processes of a dying parent process */ 
    for (unsigned i = 0; i < PLIST_MAX_LENGTH; i++)
    {
      if (p_list.content[i] != NULL && pid == p_list.content[i]->pid_parent)
      {  
        sema_down (&p_list.content[i]->sema_p);
        p_list.content[i]->parent_alive = 0;
        sema_up (&p_list.content[i]->sema_p);
      }
    }
    
    /* Signal to waiting parent process that this process is ready for removal */
    sema_up (&p->sema_p_wait); 
    sema_up (&p->sema_p);
    rw_lock_release_readlock (&lock_plist_rw);
    
    /* Destroy the current process's page directory and switch back
      to the kernel-only page directory. */
    if (pd != NULL)
    {
      /* Correct ordering here is crucial.  We must set
        cur->pagedir to NULL before switching page directories,
        so that a timer interrupt can't switch back to the
        process page directory.  We must activate the base page
        directory before destroying the process's page
        directory, or our active page directory will be one
        that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
    }
  }
  else
  {
    printf ("%s: exit(%d)\n", thread_name (), status);
    rw_lock_release_readlock (&lock_plist_rw); 
  }
  
  rw_lock_acquire_writelock (&lock_plist_rw);

  /* Clean any remaining processes with parent and child not alive */ 
  for (unsigned i = 0; i < PLIST_MAX_LENGTH; i++)
  {
    if (p_list.content[i] != NULL)
    {
      process_t process = p_list.content[i];
      if (!process->parent_alive) 
      {
        process = plist_remove (i);
        process_destroy (process);
      }
    }
  }
  
  rw_lock_release_writelock (&lock_plist_rw);
  
  cur->parent = NULL;

  /* Close all files in file table */
  flist_close_all ();
}


/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}

