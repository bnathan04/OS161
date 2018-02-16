
// include header

#include <kern/syscall_func.h>

// includes from syscall.c
#include <../arch/mips/include/vm.h>
#include <../arch/mips/include/types.h>
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <machine/pcb.h>
#include <machine/spl.h>
#include <machine/trapframe.h>
#include <kern/callno.h>
#include <syscall.h>
#include <kern/unistd.h>

// include the dumbvm

#include <addrspace.h>

// include the process stuff
#include <thread.h>
#include <curthread.h>

// include synch stuff
#include <synch.h>

// include absolute limits (for path length)
#include <kern/limits.h>

// include test header for runprogram and associated helper functions
#include <test.h>

// integrate vm
#include <vm.h>
#include <pagetable.h>
#include <coremap.h>
#include <clock.h>

#define MAX_STR_LEN 256


/* Function Definitions */

//Sahan Nov 1
//sys_call for a user level thread to write aka(printf)
int sys_write(int fd, const void *buf, size_t nbytes, int32_t *retVal){

	
	//EIO	A hardware I/O error occurred writing the data -> DOES THIS NEED TO BE IMPLEMENTED
	
	//fd is not a valid file descriptor or was not opened for writing.
	//*we don't have to worry about non stdout/stderr right now  because we're not interfacing with other files/devices	
	if(fd != STDOUT_FILENO && fd != STDERR_FILENO){
		return(EBADF);
	}

	//EFAULT Part or all of the address space pointed to by buf is invalid.	
	//was told to hardcode answers for this one for now but edit me later asdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdfasdf
	if(buf == NULL || buf == 0x0 || buf == (void*)0xbadbeef|| buf == (void*)0xdeadbeef){
		return(EFAULT);
	}


	char *dest = kmalloc((sizeof(char*))*nbytes);

	//ENOSPC	There is no free space remaining on the filesystem containing the file.	
	if(dest == NULL){
		kfree(dest);
		return(ENOMEM);
	}

	int copyError = copyin(buf, dest, nbytes);
	
	if(copyError){
		kfree(dest);
		return(copyError);
	}

	//kernel level 
	size_t i;
	for(i =0; i< nbytes; i++){
		putch(dest[i]);
	}
	
	//Note that in some cases, particularly on devices, fewer than buflen (but greater than zero) bytes may be written.
	//This depends on circumstances and does not necessarily signify end-of-file. 
	//In most cases, one should loop to make sure that all output has actually been written.
	//*we don't have to worry about this because we're not writing to devices
	*retVal = nbytes;

	kfree(dest);
	dest = NULL;
	return(0);
}

int sys_read(int fd, void *buf, size_t buflen, int32_t *retVal ){


	// check if fd is a valid file descripter 
	// and was opened for reading
	if(fd != STDIN_FILENO){
		return(EBADF);
	}

	// check validity of address space pointed to by buf
	if(buf == NULL){
		return(EFAULT);
	}

	// since this implementation only works for buflen =1, handle >=1 as an error
	// if(buflen > 1){
	// 	return(ENOSYS);
	// }

	// **kernel level operations**
	
	// every read write should be atomic? (HOW TO IMPLEMENT? getch complains with curspl > 0, so have to use locks? how?)

	// note here -> we are reading in only one char per sys_read <- this is probably not the best way to do it
	char *tmp = kmalloc(sizeof(char)*buflen);
	if(tmp == NULL){
		return (ENOMEM);
	}

	*tmp = getch();

	// // read in 
	// size_t i = 0;
	// for(i ; i < buflen; i++){
	// 	tmp[i] = getch();
	// 	*retVal ++;
	// 	if(tmp[i] == '\n' || tmp[i] = '\r'){
	// 		break;
	// 	}
	// }

	// copy over to userspace
	int copyError = copyout(tmp, (userptr_t) buf, buflen);
	kfree(tmp);
	tmp = NULL;
	if(copyError){	
		return copyError;
	}

	// temporary solution for retval
	*retVal = 1;

	// if we reach this point, success

	return(0);

}

void
sys_getpid(int *retVal){
	*retVal = curthread->pid;
}


// int 
// process_create(const char* name, void* tf, unsigned long addr, void (*func)(void*, unsigned long), int32_t *retVal){

// 	kprintf("HELLO WE ARE IN PROCESS CREATE\n ");

// 	process* newProc;
// 	int tableIndex = 0; 

// 	// probably should do atomically
//  	// while (pTable[tableIndex] != NULL || pTable[tableIndex]->pState == P_EXIT){
//  	// 	kprintf("HELLO WE ARE IN THE LOOP\n ");
//  	// 	tableIndex++;
//  	// }

//  	pTable[tableIndex] = *newProc;
//  	newProc->pState = P_RUN;

//  	//end atomic

 	

//  	//how to track parent ID?

//  	int err = thread_fork (name, tf, addr, func, &(newProc->thread));
//  	if(err){
//  		return (err);
//  	}

//  	kprintf("HELLO WE ARE BACK FROM FORKING\n ");

//  	newProc->thread->pid = tableIndex;

//  	*retVal = newProc->thread->pid;

//  	return (0);
// }

//fork a user level thread
int
sys_fork(struct trapframe *tf, int32_t *retVal){
	

	// create a child trapframe, which has to be dynamic
	// in case the parent finished before the child, which could
	// prevent thread_fork from working because the variable woud
	// be lost


	struct trapframe *tf_child = kmalloc(sizeof(struct trapframe));

	// if kmalloc returns null, return error 
	if(tf_child == NULL){
		return ENOMEM;
	}

	// copy the trapframe into the child trapframe and create a new thread
	// create new address space for child also, as each process/thread needs its own stack
	*tf_child = *tf;	

	struct addrspace* addr_parent = curthread->t_vmspace;
	struct addrspace* addr_child;

	int copyError = as_copy(addr_parent, &addr_child);
	if (copyError){
		// kprintf("copy error\n");
		return (copyError);
	}

	// must learn wtf as_activate accomplishes
	// as_activate(curthread->t_vmspace);
	char name[] = "child_proc"; 
	// struct thread* child_process;	

	// int err = process_create(name, (void*)tf_child, (unsigned long)addr_child, md_forkentry_wrapper, retVal);

	// fork a new thread/process
	// explanation of params:
	// 1 - name: no one cares
	// 2 - param1 to forked function: should be pointer to the trapframe, casted to void*, because that is what thread_fork takes
	// 3 - param2 to forked function: should be new address space for child, for creation of a new stack, casted to unsigned long, b/c that is what thread fork takes
	// 4 - function: this is the actual function that is the entry point for the fork; it must be a func that returns void and takes (void*, unsigned long) as parameters
	// 5 - thread double pointer: this is pointer to pointer of thread - thread fork will dereference this once and modify child_proces to point to the new thread

	// int err = thread_fork(name, (void*)tf_child, (unsigned long)addr_child, md_forkentry_wrapper, &child_process);
	int err = process_create(name, (void*)tf_child, (unsigned long)addr_child, md_forkentry_wrapper, retVal);
	
	// if thread fork doesn't work return the error	
	if (err){
		// kprintf("process create error \n");
		return (err);
	}

	// if successful, set the retVal and return 0 -- note: retVal is set inside proc_create

	// *retVal = child_process->pid;

	return(0);
}


//Sahan Nov 4th
//wait until a process exits and it's process id(pid)
//retVal is the process id whose exit stauts is reported in status.
//in os161, this is walways the value of pid
//currently supports no options
pid_t
sys_waitpid(pid_t pid, int *status, int options, int* retVal, const int* fromUser){
	

	if(pid <= 0){
		return(EINVAL);
	}

	if(pid >= MAXPROCESS){
		return(EINVAL);
	}

	if(fromUser != NULL){
		//check invalid status pointer location
		int result = 0;
		int* kerstatus = kmalloc(sizeof(int));
		result = copyin((userptr_t)status, kerstatus, sizeof(int));

		if(result){
			kfree(kerstatus);
			return(EFAULT);
		}		
	}

	//misaligned
	if((int)status % 4 != 0){
		return(EFAULT);
	}


	// kprintf("HELLO WE ARE IN WAITPID\n");
	//EINVAL	The options argument requested invalid or unsupported options.
	if(options != NO_OPTIONS){
		return(EINVAL);
	}

	//EFAULT	The status argument was an invalid pointer.
	//1. check null
	//2. check in user space. for now hardcode me
	if(status == NULL || status == 0x0 || status == (int*)0xbadbeef|| status == (int*)0xdeadbeef){
		return(EFAULT);
	}


	//check that the process existed 
	process* _process = &pTable[pid];
	assert(_process->pState != P_UNUSED);
	
	// kprintf("\npid = %d and waited on = %d \n", pid, _process->waitedOn);

	if(_process->waitedOn == 1){
		// kprintf("\nalready waited on\n");		
		return(EINVAL);
	}
	else _process->waitedOn = 1;	


	//wait for it to end then check exit status
	P(_process->exitSem);
	
	*retVal = _process->pid;
	*status = _process->exitStatus;

	V(_process->exitSem);

	return(0);//success	
}

//exit the from the current process
void
sys_exit(int exitCode){

	// set the process's exit code and change it's state to exited
	pTable[curthread->pid].exitStatus = exitCode;
	pTable[curthread->pid].pState = P_EXIT;

	// release binary semaphore allowing processes waiting on this one to move forward
	V(pTable[curthread->pid].exitSem);

	// actually exit
	thread_exit();
}

// the "heart" of code 
int 
sys_execv(char* program, char** args){

	// check for invalid arguments 
	if(program == NULL || args == NULL){
		return (EFAULT); 
	}

	struct addrspace* as = curthread->t_vmspace;


	// if(args == 0x40000000){
	// 	return(EFAULT);
	// }

	// start by setting up the kernel pointer
	char* progName_k = kmalloc(PATH_MAX);

	// always double check kmalloc
	if(progName_k == NULL){
		kfree(progName_k);
		return (ENOMEM);
	}


	// copy in and handle errors
	unsigned size;
	int copyError = copyinstr((userptr_t) program, progName_k, PATH_MAX, &size);
	
	if( (size -1) <= 0){//stupid os161. no consistency.
		//copyinstr returns size with null therefore -1
		return (EINVAL);
	}


	if(!(
	(as->as_heapvbase >= args && args < as->as_heapvbase + MAX_HEAP_SIZE) ||
	(as->as_vbase1 >= args && args < as->as_vbase1 + as->as_npages1*PAGE_SIZE) ||
	(as->as_vbase2 >= args && args < as->as_vbase2 + as->as_npages2*PAGE_SIZE) ||
	(as->as_stackvbase > args && args >= as->as_stackvbase - 500*PAGE_SIZE)
	)){
		return(EFAULT);
	}





	/* move the path name from userspace to kernelspace */

///

	if (copyError){
		kfree(progName_k);
		return copyError;
	}

	/* find the number of arguments */

	// start by making sure the double pointer is valid and handle errors
	char** dummyForErrorCheck = kmalloc(sizeof(char*)*MAX_STR_LEN);

	copyError = copyinstr((userptr_t) args, dummyForErrorCheck, MAX_STR_LEN, NULL);
	kfree(dummyForErrorCheck);

	if(copyError){
		kfree(progName_k);
		return copyError;
	}

	// cycle through and find numArgs
	int numArgs;
	for (numArgs = 0 ; numArgs >= 0 ; numArgs++){
		if(args[numArgs] == NULL){
			break;
		}
	}

	/* move the argument vector from user space to kernelspace */

	// start by setting up kernel pointer
	char** argVector_k = kmalloc(sizeof(char*)*numArgs);

	// always double check kmalloc	
	if (argVector_k == NULL){
		kfree(progName_k);
		return (ENOMEM);
	}

	// copy in and handle errors FOR THE DOUBLE POINTER ONLY 
	copyError = copyinstr((userptr_t) args, argVector_k, sizeof(char*)*numArgs, NULL);
	if (copyError){
		kfree(progName_k);
		kfree(argVector_k);
		return copyError;
	}


	// now time to copy in each individual argument (single pointers)

	int i;

	for(i = 0 ; i < numArgs ; i++){
		
		
		argVector_k[i] = kmalloc(sizeof(char)*MAX_STR_LEN);

		// always double check kmalloc
		if (argVector_k[i] == NULL){
			
			// have to loop to free everything allocated so far
			int a;

			for(a = 0 ; a < i ; a++){
				kfree(argVector_k[a]);
			}

			kfree(progName_k);
			kfree(argVector_k);

			return (ENOMEM);
		}

		// copy in and handle errors
		copyError = copyinstr((userptr_t) args[i], argVector_k[i], MAX_STR_LEN, NULL);
		if (copyError){
			
			// have to loop to free everything allocated so far
			int a;

			for(a = 0 ; a < i ; a++){
				kfree(argVector_k[a]);
			}

			kfree(progName_k);
			kfree(argVector_k);

			return (copyError);
		}	
	}

	// pass our information to runprogram
	int result = runprogram(progName_k, numArgs, argVector_k, USERCALL, 0);

	// should only get here in case of error
	if(result){
		return (result);
	}


	
}

/*
 * Time syscall - returns time of day
 * 
 * If input pointers are not NULL, seconds and nano seconds component of time is stored at those addresses
 * 	
 */
 int 
 sys_time(time_t* seconds, unsigned long* nseconds, int* retVal){

 	
 	// create kernel pointers to hold time values
 	time_t* kseconds = kmalloc(sizeof(time_t));
 	unsigned long* knseconds = kmalloc(sizeof(unsigned long));

 	// error handling variables
 	int copyError;

 	// get time
 	gettime(kseconds, knseconds);

 	// if either input pointer is non-null, copyout the required 
 	// time data
 	if (seconds != NULL){

 		copyError = copyout(kseconds, seconds, sizeof(time_t));
 		
 		if(copyError){

 			//kfree(knseconds);
 			return(copyError);

 		}

 	}

 	if (nseconds != NULL){

 		copyError = copyout(knseconds, nseconds, sizeof(unsigned long));
 		//kfree(knseconds);
 		if(copyError){

 			return(copyError);
 		
 		}

 	}

 
 	// set the return value to time in seconds
 	*retVal = *kseconds;
 	


 	// if we reach here => success
 	return(0);
 }

// heap allocations
int 
sys_sbrk(unsigned int amount, int* retVal){

	// kprintf("IN SYSBRK\n");

	struct addrspace* as = curthread->t_vmspace;

	// vaddr_t heapbase, heapvtop;

	// heapbase = as->as_heapvbase;
	// heaptop = as->as_heapvtop;

	/* WE ASSUME THAT THE BREAK AMOUNT IS PAGE ALIGNED */

	if (amount == 0){//not trying to move heap. jsut want to know where it is
		*retVal  = as->as_heapvtop;	
		return (0);
	}
	else if(coremapEntriesFree == 0){//no pages to put this extra stuff you want. 
		*retVal = 0x0;
		return (0);
	}	
	else if ((signed int)as->as_heapvtop + (signed int)amount < (signed int)as->as_heapvbase){//decreasing too much
		// return(ENOMEM);
		return EINVAL;
	}
	else if (as->as_heapvtop + amount >= as->as_heapvbase + MAX_HEAP_SIZE){//want to much space
		return ENOMEM;
	}	
	else if (amount % 4 == 0){//amount is already word aligned. move that much
		*retVal = as->as_heapvtop;
		
		// kprintf("New heap allocation: heaptop before: 0x%x\n", as->as_heapvtop);
		as->as_heapvtop += amount;

		// assert((as->as_heapvtop & PAGE_FRAME) == as->as_heapvtop);
	}
	else if (amount % 4 != 0){//word align then move
		amount = ROUNDUP(amount,  4);

		*retVal = as->as_heapvtop;
		
		// kprintf("New heap allocation: heaptop before: 0x%x\n", as->as_heapvtop);
		as->as_heapvtop += amount;

		// assert((as->as_heapvtop & PAGE_FRAME) == as->as_heapvtop);
	}

	// kprintf("heaptop after: 0x%x\n", as->as_heapvtop);

	return (0);

	// unsigned int pageAlignedAmount = *amount;

	// // page aligned amount
	// pageAlignedAmount = ROUNDUP(*amount, PAGE_SIZE);
	// assert((pageAlignedAmount & PAGE_FRAME) == pageAlignedAmount);

	// // get the number of pages this is 
	// int npages = pageAlignedAmount/PAGE_SIZE;

	// // "grow" the heap
	// // store the old amount first

	// int oldbrk = heaptop;
	// curthread->t_vmspace->as_heapvtop += *amount;

	// int i;
	// int vmError = 0;



	// for (i = 0; i < npages; i++){

	// 	assert((heaptop & PAGE_FRAME) == heaptop);	

	// 	vaddr_t faultaddress = heaptop + (i+1)*PAGE_SIZE;

	// 	assert((faultaddress & PAGE_FRAME) == heaptop);

	// 	vmError = vm_fault(-1,faultaddress);

	// 	if (vmError){
	// 		return (vmError);
	// 	}

		// page* newEntry;

		// // do the actual allocation
		// assert(npages == 1);
		// assert(page_in_pagetable(faultaddress))
		// newEntry = page_create(faultaddress,1);
		
		// if (newEntry == NULL){
		// 	return EFAULT;
		// }

		// paddr_t paddr = newEntry->paddr;

		// /* make sure it's page-aligned */
		// assert((paddr & PAGE_FRAME)==paddr);
		// assert(page_in_pagetable(faultaddress));


	// }

	// success

	// *retVal = oldbrk;

	// return (0);





}
