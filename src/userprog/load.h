#ifndef USERPROG_LOAD_H
#define USERPROG_LOAD_H

/*bool*/
struct file * load (const char *file_name, void (**eip) (void), void **esp);
void dump_stack(void* ptr, int size);

#endif /* userprog/load.h */
