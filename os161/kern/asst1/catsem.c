/*
 * catsem.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use SEMAPHORES to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */


#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <machine/spl.h>
#include <synch.h>


/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2

#define EAT_AMOUNT 4 

//Balu and Sahan - Oct 7th
#define NOONE_EATING 2
#define MOUSE_EATING 1
#define CAT_EATING 0

struct semaphore* bowlSem;//tells how many bowls are avaiable
int speciesEating;//tells whos eating from the bowls

//0 means free. 1 means occupied
int bowl0 = 0;
int bowl1 = 0;

/*
 * 
 * Function Definitions
 * 
 */

/* who should be "cat" or "mouse" */
static void
sem_eat(const char *who, int num, int bowl, int iteration)
{       
        kprintf("%s: %d starts eating: bowl %d, iteration %d\n", who, num, 
                bowl, iteration);
        clocksleep(1);
        kprintf("%s: %d ends eating: bowl %d, iteration %d\n", who, num, 
                bowl, iteration);
}

/*
 * catsem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
catsem(void* iteration, 
       unsigned long catnumber)
{       
    
    //eat n times
    while(*(int*)iteration < EAT_AMOUNT){
        
        int bowlNumber;
        int* theBowl;

        //stop interrupts
        int spl = splhigh();             
        
        DEBUG(DB_THREADS, "\ncat first %d\n", speciesEating);

        //waiting for bowl
        while(1){

            /*
            if i wait for species to not be mouse eating and then wait for a bowl, 
            it's possible for a mouse to grab a bowl while cat's waiting; thefore,
            you have to grab a bowl and check if you're allowed to have it before you start eating  
            */
            P(bowlSem);
            if(speciesEating == MOUSE_EATING){
                V(bowlSem);
                thread_yield();
            }
            else{            
                break;
            }
        }
        
        //eat
        speciesEating = CAT_EATING;
        bowlNumber = (bowl0 == 0)? 0 : 1;
        theBowl = (bowlNumber == 0)? &bowl0 : &bowl1;
        *theBowl = 1;
        sem_eat("cat", catnumber, bowlNumber + 1, *(int*)iteration);
        (*(int*)iteration)++;

        //say not eating at this bowl anymore
        *theBowl = 0;
        speciesEating = (bowl0 == 0 && bowl1 == 0)? NOONE_EATING : CAT_EATING; 
        theBowl = NULL;
        kfree(theBowl);

        //actaully give up the bowl
        V(bowlSem);
        thread_wakeup(bowlSem);    
        splx(spl);        
    }
}
        

/*
 * mousesem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *      mouse checks if there are no cats eating
        if no cats, wait until free bowl -> eat bowl -> leave
        if cats present, wait for 0 cats then eat
 */

static
void
mousesem(void * iteration, 
         unsigned long mousenumber)
{
    while(*(int*)iteration < EAT_AMOUNT){
        int bowlNumber;
        int* theBowl;

        //stop interrupts    
        int spl = splhigh();             
        DEBUG(DB_THREADS, "\nmouse first %d\n", speciesEating);

        //waiting for bowl
        while(1){
            P(bowlSem);
            if(speciesEating == CAT_EATING){
                V(bowlSem);
                thread_yield();
            }
            else{            
                break;
            }
        }
        
        speciesEating = MOUSE_EATING;
        bowlNumber = (bowl0 == 0)? 0 : 1;
        theBowl = (bowlNumber == 0)? &bowl0 : &bowl1;
        *theBowl = 1;

        sem_eat("mouse", mousenumber, bowlNumber + 1, *(int*)iteration);
        (*(int*)iteration)++;

        //give up bowl
        *theBowl = 0;
        speciesEating = (bowl0 == 0 && bowl1 == 0)? NOONE_EATING : MOUSE_EATING; 
        theBowl = NULL;
        kfree(theBowl);

        V(bowlSem);
        thread_wakeup(bowlSem); 
        splx(spl);
    }
}


/*
 * catmousesem()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
        creates cat and mice 
 *      Driver code to start up catsem() and mousesem() threads.  Change this 
 *      code as necessary for your solution.
 */

int
catmousesem(int nargs,
            char ** args)
{
        int index, error;
   
        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;
        
        //make semaphore for num bowls
        bowlSem = sem_create("bowls", 2);

        //make sempahore to check whos eating
        speciesEating  = NOONE_EATING;
        /*
         * Start NCATS catsem() threads.
         */
                                                                                                        //should we change the name to include cat 1
        for (index = 0; index < NCATS; index++) {
                
                int* iteration = kmalloc(sizeof(int));//the number of times it's eaten
                *iteration = 0;

                error = thread_fork("catsem Thread", 
                                    iteration, 
                                    index, 
                                    catsem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                    panic("catsem: thread_fork failed: %s\n", 
                          strerror(error)
                        );
                }
        }
        
        /*
         * Start NMICE mousesem() threads.
         */

        for (index = 0; index < NMICE; index++) {

                int* iteration = kmalloc(sizeof(int));//the number of times it's eaten   
                *iteration = 0;

                error = thread_fork("mousesem Thread", 
                                    iteration, 
                                    index, 
                                    mousesem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
         
                    panic("mousesem: thread_fork failed: %s\n", 
                          strerror(error)
                    );
                }
        }

        return 0;
}


/*
 * End of catsem.c
 */
