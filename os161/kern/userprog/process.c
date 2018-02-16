#include <thread.h>
#include <process.h>
#include <curthread.h>



int 
process_create(char* name, void* trapframe, unsigned long addr, void (*entrypoint)(void *, unsigned long), int* retVal){
 
	struct thread* child_process;

	int err = thread_fork(name, trapframe, addr, entrypoint, &child_process);

	if (err){
		return (err);
	}

	child_process->parentPid = curthread->pid;
	curthread->childPid = child_process->pid;

 	
	*retVal = child_process->pid;
 	

 	return (0);
}