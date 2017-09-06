# -*- makefile -*-

SRCDIR = ../..

all: kernel.bin loader.bin

include ../../Make.config
include ../Make.vars
include ../../tests/Make.tests

# Compiler and assembler options.
kernel.bin: CPPFLAGS += -I$(SRCDIR)/lib/kernel

# Core kernel.
threads_SRC  = threads/start.S		# Startup code.
threads_SRC += threads/init.c		# Main program.
threads_SRC += threads/thread.c		# Thread management core.
threads_SRC += threads/switch.S		# Thread switch routine.
threads_SRC += threads/interrupt.c	# Interrupt core.
threads_SRC += threads/intr-stubs.S	# Interrupt stubs.
threads_SRC += threads/synch.c		# Synchronization.
threads_SRC += threads/palloc.c		# Page allocator.
threads_SRC += threads/malloc.c		# Subpage allocator.

# Device driver code.
devices_SRC  = devices/pit.c		# Programmable interrupt timer chip.
devices_SRC += devices/timer.c		# Periodic timer device.
devices_SRC += devices/kbd.c		# Keyboard device.
devices_SRC += devices/vga.c		# Video device.
devices_SRC += devices/serial.c		# Serial port device.
devices_SRC += devices/block.c		# Block device abstraction layer.
devices_SRC += devices/partition.c	# Partition block device.
devices_SRC += devices/ide.c		# IDE disk block device.
devices_SRC += devices/input.c		# Serial and keyboard input.
devices_SRC += devices/intq.c		# Interrupt queue.
devices_SRC += devices/rtc.c		# Real-time clock.
devices_SRC += devices/shutdown.c	# Reboot and power off.
devices_SRC += devices/speaker.c	# PC speaker.

# Library code shared between kernel and user programs.
lib_SRC  = lib/debug.c			# Debug helpers.
lib_SRC += lib/random.c			# Pseudo-random numbers.
lib_SRC += lib/stdio.c			# I/O library.
lib_SRC += lib/stdlib.c			# Utility functions.
lib_SRC += lib/string.c			# String functions.
lib_SRC += lib/arithmetic.c		# 64-bit arithmetic for GCC.
lib_SRC += lib/ustar.c			# Unix standard tar format utilities.

# Kernel-specific library code.
lib/kernel_SRC  = lib/kernel/debug.c	# Debug helpers.
lib/kernel_SRC += lib/kernel/list.c	# Doubly-linked lists.
lib/kernel_SRC += lib/kernel/bitmap.c	# Bitmaps.
lib/kernel_SRC += lib/kernel/hash.c	# Hash tables.
lib/kernel_SRC += lib/kernel/console.c	# printf(), putchar().

# User process code.
userprog_SRC  = userprog/process.c	# Process loading.
userprog_SRC += userprog/pagedir.c	# Page directories.
userprog_SRC += userprog/exception.c	# User exception handler.
userprog_SRC += userprog/syscall.c	# System call handler.
userprog_SRC += userprog/gdt.c		# GDT initialization.
userprog_SRC += userprog/tss.c		# TSS management.

# No virtual memory code yet.
#vm_SRC = vm/file.c			# Some file.

# Filesystem code.
filesys_SRC  = filesys/filesys.c	# Filesystem core.
filesys_SRC += filesys/free-map.c	# Free sector bitmap.
filesys_SRC += filesys/file.c		# Files.
filesys_SRC += filesys/directory.c	# Directories.
filesys_SRC += filesys/inode.c		# File headers.
filesys_SRC += filesys/fsutil.c		# Utilities.

SOURCES = $(foreach dir,$(KERNEL_SUBDIRS),$($(dir)_SRC))
OBJECTS = $(patsubst %.c,%.o,$(patsubst %.S,%.o,$(SOURCES)))
DEPENDS = $(patsubst %.o,%.d,$(OBJECTS))

GDBPORT := $(shell expr `id -u` % 5000 + 25000)
IMAGE = kernel.img
QEMUOPTS = -serial mon:stdio -gdb tcp::$(GDBPORT)
QEMUOPTS += $(shell if $(QEMU) -nographic -help | grep -q '^-D '; then echo '-D qemu.log'; fi)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS) $(WARNINGS) $(DEFINES) $(DEPS)

%.o: %.S
	$(CC) -c $< -o $@ $(ASFLAGS) $(CPPFLAGS) $(DEFINES) $(DEPS)

threads/kernel.lds.s: CPPFLAGS += -P
threads/kernel.lds.s: threads/kernel.lds.S threads/loader.h

kernel.o: threads/kernel.lds.s $(OBJECTS) 
	$(LD) $(LDOPTIONS) -T $< -o $@ $(OBJECTS)
	$(OBJDUMP) -S $@ > kernel.asm
	$(NM) -n $@ > kernel.sym

kernel.bin: kernel.o
	$(OBJCOPY) -R .note -R .comment -S $< $@

threads/loader.o: threads/loader.S
	$(CC) -c $< -o $@ $(ASFLAGS) $(CPPFLAGS) $(DEFINES)

loader.bin: threads/loader.o
	$(LD) $(LDOPTIONS) -N -e 0 -Ttext 0x7c00 -o loader.out $<
	$(OBJDUMP) -S loader.out > loader.asm
	$(OBJCOPY) -S -O binary -j .text loader.out $@
	rm loader.out

os.dsk: kernel.bin
	cat $^ > $@

kernel.img: kernel.bin loader.bin
	rm -f $@
	$(PINTOS) --make-disk=$@ --kernel=kernel.bin --loader=loader.bin

qemu: $(IMAGE)
	$(QEMU) -drive file=$(IMAGE),index=0,media=disk,format=raw $(QEMUOPTS)

qemu-nox: $(IMAGE)
	@echo "***"
	@echo "*** Use Ctrl-a x to exit qemu. If you are under GNU Screen or Tmux and "
	@echo "*** its prefix key is Ctrl-a, press Ctrl-a twice and x to exit."
	@echo "***"
	$(QEMU) -nographic -drive file=$(IMAGE),index=0,media=disk,format=raw $(QEMUOPTS)

clean::
	rm -f $(OBJECTS) $(DEPENDS) 
	rm -f threads/loader.o threads/kernel.lds.s threads/loader.d
	rm -f kernel.bin.tmp
	rm -f kernel.o kernel.lds.s
	rm -f kernel.bin loader.bin
	rm -f loader.asm kernel.asm kernel.sym
	rm -f bochsout.txt bochsrc.txt
	rm -f results grade

Makefile: $(SRCDIR)/Makefile.build
	cp $< $@

-include $(DEPENDS)

.PHONY: all clean qemu image
