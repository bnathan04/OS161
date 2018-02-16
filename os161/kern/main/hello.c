


/* Balu, 2017/09/18 - This function is a test to print out "hello world" using kprintf() */

#include <hello.h>
#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <lib.h>
#include <machine/spl.h>
#include <test.h>
#include <synch.h>
#include <thread.h>
#include <scheduler.h>
#include <dev.h>
#include <vfs.h>
#include <vm.h>
#include <syscall.h>
#include <version.h>

#include <hello.h>

void hello_world(){
	kprintf("Hello World\n");
	return;
}

