/*
 * wait.c
 *
 * tests ordering of waitpid() and exit() without needing arguments
 *
 * Test case 1:
 *      result expected: wekp
 * Test case 2:
 *      result expected: ewp
 * Test case 3:
 *      result expected: acp
 * Test case 4: 
 *      result expected: att
 *
 * Authors:
 * Kuei Sun <kuei.sun@mail.utoronto.ca>
 * Ashvin Goel <ashvin@eecg.toronto.edu>
 *
 * University of Toronto, 2016
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

static volatile int mypid;
int pid_p, pid_c;

static int dofork(void)
{
        int pid;
        pid = fork();
        if (pid < 0) {
                warn("fork");
        }
        return pid;
}

/*
 * copied from address space test in forktest
 */
static void check(void)
{
        int i;

        mypid = getpid();
        
        /* Make sure each fork has its own address space. */
        for (i=0; i<800; i++) {
                volatile int seenpid;
                seenpid = mypid;
                if (seenpid != getpid()) {
                        errx(1, "pid mismatch (%d, should be %d) "
                             "- your vm is broken!", 
                             seenpid, getpid());
                }
        }
}

/*
 * based on dowait in forktest
 */
static void dowait(int pid)
{
        int x;

        if (pid<0) {
                /* fork in question failed; just return */
                return;
        }
        if (pid==0) {
                /* we were the child in the fork -- exit */
                exit(0);
        }
        if (waitpid(pid, &x, 0)<0) 
                warn("waitpid");
        else if (x!=0)
                warnx("pid %d: exit %d", pid, x);
                
}

#define TEST_BEGIN(num) void wait ## num(void) { printf(#num " ");
#define TEST_END() }

TEST_BEGIN(1)
        pid_p = getpid();
        putchar('w');
        pid_c = dofork();

        if (getpid() == pid_p) {
                check();
                dowait(pid_c);
        } else {
                putchar('e');
                _exit(0);
        }

        putchar('k');
        if (getpid() == pid_p)
                putchar('p');
        else 
                printf("wrong %d\n", getpid());

        putchar('\n');
TEST_END()

TEST_BEGIN(2)
        pid_p = getpid();
        putchar('e');
        pid_c = dofork();

        if (getpid() == pid_p)
                check();

        if (getpid() != pid_p) {
                check();
                _exit(0);
        } else {
                putchar('w');
                dowait(pid_c);
        }

        putchar('p');
        putchar('\n');
TEST_END()

TEST_BEGIN(3)
        pid_p = getpid();
        putchar('w');
        pid_c = dofork();

        if (getpid() == pid_p)
                dowait(pid_c);

        if (getpid() != pid_p) {
                check();
                putchar('e');
                _exit(0);
        } 

        if (getpid() == pid_p)
                dowait(pid_c);
        else
                printf("wrong!\n");

        putchar('\n');
TEST_END()

TEST_BEGIN(4)
        pid_p = getpid();          
        putchar('a');
        pid_c = dofork();          

        if (getpid() == pid_p)  
                check();               

        if (getpid() == pid_p) {
                dowait(pid_c);
                putchar('p');
                putchar('\n');
                _exit(0);            
        } else {
                putchar('c');          
                _exit(0);              
        }

        putchar('e');              
        putchar('\n');             
TEST_END()

TEST_BEGIN(5)
        pid_p = getpid();
        putchar('a');
        pid_c = dofork();

        if (getpid() == pid_p) {
                check();
                putchar('t');
                _exit(0);
        } else {
                putchar('t');
                _exit(0);
        }

        putchar('e');
        putchar('\n');
TEST_END()

int main(void)
{
        int pid;
        wait1();
        wait2();
        wait3();
        /* (jsun): test 3 requires both parent and child to exit... 
         * so we need to fork here */
        if ( (pid = dofork()) == 0 )
            wait4();
        else
            /* wait for test 3 to finish before starting test 4 */
            dowait(pid);    
        wait5();
        return 0;
}
