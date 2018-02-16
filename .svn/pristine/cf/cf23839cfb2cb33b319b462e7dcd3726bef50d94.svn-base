#ifndef _TEST_H_
#define _TEST_H_

#define WORD_SIZE 4 //smallest size of arguments on stack allowed
#define TER_CHAR_SIZE 1 //for the terminating character at the end of the string

#define USERCALL 0
#define KERNELCALL 1

/*
 * Declarations for test code and other miscellaneous functions.
 */


/* These are only actually available if OPT_SYNCHPROBS is set. */
int catmousesem(int, char **);
int catmouselock(int, char **);
int createcars(int, char **);

/*
 * Test code.
 */

/* lib tests */
int arraytest(int, char **);
int bitmaptest(int, char **);
int queuetest(int, char **);

/* thread tests */
int threadtest(int, char **);
int threadtest2(int, char **);
int threadtest3(int, char **);
int semtest(int, char **);
int locktest(int, char **);
int cvtest(int, char **);

/* filesystem tests */
int fstest(int, char **);
int readstress(int, char **);
int writestress(int, char **);
int writestress2(int, char **);
int createstress(int, char **);
int printfile(int, char **);

/* other tests */
int malloctest(int, char **);
int mallocstress(int, char **);
int nettest(int, char **);

/* Kernel menu system */
void menu(char *argstr);

/* Routine for running userlevel test code. */
int runprogram(char *progname, unsigned long argc, char**argv, int kernelCall, vaddr_t stackptr);
//helper functions for run program
int move_kernel_to_stack(unsigned long argc, char** argv, vaddr_t* stackPtr);
int how_many_words(char* arg);

#endif /* _TEST_H_ */
