#include <stdio.h>
#include <syscall-nr.h>
#include "userprog/syscall.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

#include <string.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "devices/timer.h"
#include "flist.h"
#include "threads/synch.h"
#include "devices/shutdown.h"

/* Helper functions */
static bool is_valid_fixed_buffer (void *, unsigned);
static bool is_valid_variable_buffer (char *);
static bool fd_is_valid (int, enum fd_type);
static int file_pointer_insert (struct file*);
static struct file* file_pointer_get (int);
static struct file* file_pointer_remove (int);

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  int32_t *esp = (int32_t *) f->esp;

  if (esp == NULL || !is_valid_fixed_buffer ((void *) esp, 4))
    exit (-1);

  switch (*esp)
    {
      case SYS_HALT: /* 0 args */
      halt ();
      break;

    case SYS_EXIT: /* 1 arg */
      /* Check that esp has 8 valid bytes */
      if (is_valid_fixed_buffer ((void *) esp, 8)) 
      {  
        exit (esp [1]);
      }
      else
      {
        exit (-1); 
      }
      break;

    case SYS_EXEC: /* 1 arg */
      if (is_valid_fixed_buffer ((void *) esp, 8) && 
          is_valid_variable_buffer ((void *) esp[1]))
      {
        f->eax = exec ((const char *) esp[1]); 
      }
      else
      {
        exit (-1);
      }
      break;

    case SYS_WAIT: /* 1 arg */
      if (is_valid_fixed_buffer ((void *) esp, 8)) 
      {  
        f->eax = wait (esp[1]); 
      }
      else
      {  
        exit (-1);
      }
      break;
    
    case SYS_SLEEP: /* 1 arg */ 
      if (is_valid_fixed_buffer ((void *) esp, 8))
      {
        sleep (esp[1]);
      }
      else
      {
        exit (-1);
      }
      break;

    case SYS_PLIST: /* 0 args */ 
      plist ();
      break;

    case SYS_CREATE:  /* 2 args */
      /* Check that esp has 12 valid bytes */
      if (is_valid_fixed_buffer ((void *) esp, 12) &&
          is_valid_variable_buffer ((char *) esp[1]))
      {
        f->eax = create ((const char *) esp[1], esp[2]);
      }
      else
      {  
        exit (-1);
      }
      break; 
    
    case SYS_REMOVE: /* 1 arg */
      if (is_valid_fixed_buffer ((void *) esp, 8) &&
          is_valid_variable_buffer ((void *) esp[1]))
      {
        f->eax = remove ((const char *) esp[1]);
      }
      else
      {
        exit (-1);
      }
      break;

    case SYS_OPEN: /* 1 arg */
      if (is_valid_fixed_buffer ((void *) esp, 8) &&
          is_valid_variable_buffer ((void *) esp[1]))
      {
        f->eax = open ((const char *) esp[1]);
      }
      else
      {
        exit (-1);
      } 
      break;

    case SYS_FILESIZE: /* 1 arg */
      if (is_valid_fixed_buffer ((void *) esp, 8))
      {
        f->eax = filesize (esp[1]);
      }
      else
      {  
        exit (-1);
      }
      break;
    
    case SYS_READ: /* 3 args */
      /* Check that esp has 16 valid bytes */
      if (is_valid_fixed_buffer ((void *) esp, 16) &&
          is_valid_fixed_buffer ((void *) esp[2], esp[3]))
      {
        f->eax = read (esp[1], (void*) esp[2], esp[3]); 
      }
      else
      {
        exit (-1);
      }
      break;

    case SYS_WRITE: /* 3 args */
      if (is_valid_fixed_buffer ((void *) esp, 16) && 
          is_valid_fixed_buffer ((void *) esp[2], esp[3]))
      {
        f->eax = write (esp[1], (void *) esp[2], esp[3]);
      }
      else
      {  
        exit (-1);
      }
      break;

    case SYS_SEEK: /* 2 args */
      if (is_valid_fixed_buffer ((void *) esp, 12))
      {
        seek (esp[1], esp[2]);
      }
      else
      {  
        exit (-1);
      }
      break;
    
    case SYS_TELL: /* 1 arg */
      if (is_valid_fixed_buffer ((void *) esp, 8))
      {
        f->eax = tell (esp[1]);
      } 
      else
      {
        exit (-1);
      }
      break;

    case SYS_CLOSE: /* 1 arg */
      if (is_valid_fixed_buffer ((void *) esp, 8)) 
      {
        close (esp[1]);
      }
      else 
      {
        exit (-1);
      }  
      break; 
    
    default:
    {
      printf ("Executed an unknown system call!\n");
      printf ("Stack top + 0: %d\n", esp[0]);
      printf ("Stack top + 1: %d\n", esp[1]);

      thread_exit ();
      break;
    }
    }
}

/* System calls */

/**
 * Shutdown Machine
*/
void 
halt (void)
{
  shutdown_power_off ();
  NOT_REACHED ();
}


/**
 * Close the current thread's files, processes and exit the current thread
*/
void 
exit (int status)
{
  process_exit ();
  thread_exit ();
  NOT_REACHED ();
}


/* Execute the given executable file */
pid_t
exec (const char *file)
{
  return process_execute (file);
}


/* Wait for process pid */
int
wait (pid_t pid)
{
  return process_wait (pid);
}


/* Sleep for the given milliseconds */
void
sleep (int ms)
{
  timer_msleep(ms);
}


/* Print a list of all current processes */
void
plist (void)
{
  process_print_list ();
}


/**
 * Create a file with the given name and size.
 * Return true on success or false.
*/
bool
create (const char *name, unsigned size)
{
  if ( strlen (name) > MAX_SIZE_FILENAME || name == NULL) 
    return false;
  return filesys_create (name, size);
}


/**
 * Remove the file with the given name from the file system
 * Return true on success or false.
*/
bool
remove (const char *name)
{
  if (name == NULL)
    return false;
  
  return filesys_remove (name);
}


/**
 * Open the file with the given name and return the file descriptor if successful.
 * Return -1 upon failure.
*/
int
open (const char *name)
{
  /* Open the file given the name */
  struct file *file = filesys_open (name);
  /*
   * filesys_open returns NULL if: 
   *  - no file with given name exists
   *  - internal memory allocation fails
   */
  if (file == NULL)
    return -1;

  /* If success, add the open file to the file table and
     get the file descriptor number. */
  int fd = file_pointer_insert (file);
  
  /* If file_pointer_insert fails, close (frees memory for file) */
  if (fd == -1)
    filesys_close (file);
  return fd;
}


/**
 * Return the size of the file for the given file descriptor
*/
int
filesize (int fd)
{
  /* Check that we aren't trying to grab the size of STDIN/STDOUT */
  if (!fd_is_valid (fd, SIZE))
    return -1;
  
  /* Get the file from the file table */
  struct file *file = file_pointer_get (fd); 

  /* If it is NULL, get out */
  if (file == NULL)
    return -1;

  /* From src/filesys/file.c */
  off_t size = file_length (file);

  return size;
}


/**
 * Read length bytes of buf from given file descriptor.
 * return bytes read
 * - fd = 0 reads from STDIN
 * - fd = 1 returns -1 (ERROR)
 * - fd > 1 reads from file with valid fd
 * - fd < 0 returns -1 (ERROR)
*/
int 
read (int fd, void *buf, unsigned length)
{
  /* Check if fd is valid */
  if (!fd_is_valid (fd, READ))
    return -1; 

  /* initialize helper variables */
  unsigned bytes_read = 0;
  uint8_t *buffer = (uint8_t *)buf;
  uint8_t c = 0;

  /* Read from STDIN */
  if (fd == STDIN_FILENO)
  { 
    while (bytes_read < length)
    {
      c = input_getc ();
      if (c == '\r')
        *(buffer + bytes_read) = '\n';
      else
        *(buffer + bytes_read) = c; 
      
      /* Display the character to the console */
      putchar (*(buffer + bytes_read));
      bytes_read++;
    }

    return bytes_read;
  } 

  /* Read from a File */

  /* Get the file pointer from the file_table flist */
  struct file *file = file_pointer_get (fd);

  /* Get out if the file was not found */
  if (file == NULL)
    return -1;
  
  bytes_read = file_read (file, buf, length);
  
  return bytes_read;
}


/**
 * Write length bytes from buffer to STDOUT or file
 * - fd == 1, write length bytes from buffer to STDOUT
 * - fd > 1 write length bytes from buffer to file with valid fd
 * - fd < 1 return -1
 * - Return bytes written
*/ 
int
write (int fd, const void *buffer, unsigned length)
{
  /* Check the fd for Writing actions */
  if (!fd_is_valid (fd, WRITE))
    return -1;

  /* Write content of buffer to STDOUT */

  if (fd == STDOUT_FILENO)
  {
    putbuf ((const char *)buffer, length);
    return length;
  } 

  /* Write to file */
 
  /* Get the file ptr from the thread's file table */
  struct file *file = file_pointer_get (fd); 
  
  /* Get out if the file was not found */
  if (file == NULL)
    return -1;

  unsigned bytes_written = file_write (file, buffer, length);

  return bytes_written; 
}


/**
 * Move the current position inside the file to the given
 * unsigned position value. 
*/
void 
seek (int fd, unsigned position)
{
  if (!fd_is_valid (fd, SEEK))
    return;

  struct file *file = file_pointer_get (fd);
  
  /* If the file pointer is NULL, get out */
  if (file == NULL)
    return;
  
  /* Get the size */
  unsigned size = filesize(fd);

  /* Don't try and seek past the end of the file */  
  if (position > size)
  {
    /* Move to the EOF if the given position argument is larger than
       the filesize */
    file_seek(file, size);
    return;
  }
  
  file_seek(file, position);
}


/**
 * Return the current position inside the file with the given file descriptor
*/
unsigned
tell (int fd)
{ 
  if (!fd_is_valid (fd, TELL))
    return -1;
    
  struct file *file = file_pointer_get(fd); 

  /* Get out if file pointer is NULL */
  if (file == NULL)
    return -1;

  off_t position = file_tell(file);
  
  return (unsigned) position;
}


/**
 * Close the file with the given file descriptor
*/
void
close (int fd)
{
  if (!fd_is_valid(fd, CLOSE))
    return;
    
  struct file *file = file_pointer_remove (fd); 
  
  /* free'd in src/filesys/file.c */
  if (file != NULL)
    filesys_close (file);
}

/**********************************************************************************************************/

/* Helper functions */


/**
  * Verify that a fixed length argument space is valid by checking
  * on each page if translation succeeded.
 */
static bool 
is_valid_fixed_buffer (void *start, unsigned length)
{
  
  void *stop = (void *) ((char *) start + length);
  start = pg_round_down (start);

  /* Protect against inputs for a REALLY large buffer such that 
  start + length > sizeof (unsignged int) */ 
  if (stop < start)
    return false;

  while (start < stop)
  {
    if (!is_user_vaddr ((void *) start))
      return false;
    if (pagedir_get_page (thread_current ()->pagedir, start) == NULL)
      return false;
    start = (void *) ((char *) start + PGSIZE);
  }
  return true;
}


/**
  * Verify that a variable length argument space is valid by checking
  * on each page if translation succeeded.
  * We assume a C-string is what is given, otherwise will run forever.
 */
static bool 
is_valid_variable_buffer(char *start) 
{
  unsigned page_current = pg_no(start);
  
  if (!is_user_vaddr ((void *) start))
    return false;
  
  /* Get out if we are starting on a bad page */
  if (pagedir_get_page(thread_current()->pagedir, start) == NULL)
    return false;

  uint8_t offset = 0;

  /* Increase our search area each iteration by adding an offset, and then
     doing a translation IF we have gone over to the next page. */
  while (1) 
  {
    /* If we have gone to the next page, reset the current page */
    if (page_current != pg_no(start + offset)) 
    {
      page_current = pg_no(start + offset);

      /* Now do a translation and get out if we have reached invalid
         memory */ 
      if (!is_user_vaddr ((void *) start + offset))
        return false;
        
      if (pagedir_get_page(thread_current()->pagedir, start + offset) == NULL)
        return false;
      
    }

    /* Stop iteration if we stil have valid memory and finally reached the
     '\0' character */
    if ('\0' == *(start + (char) offset)) 
      return true;
    
    ++offset;
  }
}


/**
 * Return true if a given file descriptor is valid,
 * false otherwise
 * 
 * int fd - the actual file descriptor
 * enum fd_type type - the operation  
*/
static bool 
fd_is_valid (int fd, enum fd_type type)
{
  /*
   * file descriptor numbers 0, 1 are reserved 
   * for STDIN, STDOUT respectively.
   * 
   * For read(), fd must be 0 for STDIN or greater than
   * 1 for a file.
   * 
   * For write(), fd must be 1 for STDOUT, or greather than 2 for a file.
   */
  
  /* Check if file descriptor is valid based on intended use case */
  switch (type)
  {
    case READ:
      return fd == 0 || fd > 1;
    
    case WRITE:
      return fd > 0;
    
    case SIZE: case SEEK: case TELL: case CLOSE:
      return fd > 1;

    default:
      return false;
  }
}


/**
 * Wrapper for inserting a file pointer into the file table.
 * Returns the file descriptor for the file
*/
static int
file_pointer_insert (struct file *file)
{
  int fd = flist_insert (&(thread_current()->f_table), file);
  return fd;
}


/**
 * Wrapper for returning a file pointer from the file table
*/
static struct file*
file_pointer_get (int fd)
{
  struct file *file = flist_find (&(thread_current()->f_table), fd);
  return file;
}


/**
 * Wrapper for removing a file from the file table
*/
static struct file*
file_pointer_remove (int fd)
{
  struct file *file = flist_remove (&(thread_current()->f_table), fd);
  return file;
}
