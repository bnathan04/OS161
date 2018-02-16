/* 
 * stoplight.c
 *
 * 31-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
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
#include <bitmap.h>
#include <synch.h>
#include <machine/spl.h>
#include <curthread.h>
#include <array.h>
 
/*
 * Constants
 *
 */

#define NCARS 20 
#define NQUADRANTS 4

#define MAX_CARS_ALLOWED 3 
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
#define LEFT 0
#define STRAIGHT 1
#define RIGHT 2
#define ENTERED 99

static const char *directions[] = { "N", "E", "S", "W" };
static const char *msgs[] = {
        "approaching:",
        "region1:    ",
        "region2:    ",
        "region3:    ",
        "leaving:    "
};

/* use these constants for the first parameter of message */
enum { APPROACHING, REGION1, REGION2, REGION3, LEAVING };

/* Declare locks for each quadrant and each entry point */
struct lock* NW_quad;
struct lock* NE_quad;
struct lock* SE_quad;
struct lock* SW_quad;

struct lock* N_entry;
struct lock* E_entry;
struct lock* S_entry;
struct lock* W_entry;

/* Declare an array to hold the entry point locks, indexed by car direction */
struct lock* entry_lanes[NQUADRANTS];

/* Declare cv to check if the intersection will be full if you enter */
struct cv* intersection_full;

int cars_running;
int num_finished;


struct array* n_entry_line;
struct array* s_entry_line;
struct array* e_entry_line;
struct array* w_entry_line;

struct array* entry_lines[4];

/* 
 * Declare struct to hold pathway information, i.e from one startpoint 
 * what are the possible destinations and the required locks on the way
 * to that destination.
 */ 

struct pathway
{
    int left_dest;
    int straight_dest;
    int right_dest;

    struct lock* left_lock;
    struct lock* straight_lock;
    struct lock* right_lock;
};

// Declare array to hold the pathways from each startpoint
struct pathway paths[NQUADRANTS];

/*
 * Function Definitions
 */

/*
 * init_path()
 *
 * Arguments:
 *      none
 *
 * Returns:
 *      none
 *
 * Notes:
 *      This function checks initializes the global struct array that holds all pathway information.
 */

static void
init_paths()
{
    paths[NORTH].left_dest = EAST;
    paths[NORTH].right_dest = WEST;
    paths[NORTH].straight_dest = SOUTH;
    paths[NORTH].left_lock = SE_quad;
    paths[NORTH].straight_lock = SW_quad;
    paths[NORTH].right_lock = NW_quad;
    
    paths[EAST].left_dest = SOUTH;
    paths[EAST].right_dest = NORTH;
    paths[EAST].straight_dest = WEST;
    paths[EAST].left_lock = SW_quad;
    paths[EAST].straight_lock = NW_quad;
    paths[EAST].right_lock = NE_quad;

    paths[SOUTH].left_dest = WEST;
    paths[SOUTH].right_dest = EAST;
    paths[SOUTH].straight_dest = NORTH;
    paths[SOUTH].left_lock = NW_quad;
    paths[SOUTH].straight_lock = NE_quad;
    paths[SOUTH].right_lock = SE_quad;
    
    paths[WEST].left_dest = NORTH;
    paths[WEST].right_dest = SOUTH;
    paths[WEST].straight_dest = EAST;
    paths[WEST].left_lock = NE_quad;
    paths[WEST].straight_lock = SE_quad;
    paths[WEST].right_lock = SW_quad;
}

/*
 * checkintersection()
 *
 * Arguments:
 *      none
 *
 * Returns:
 *      int, 0 => there are less than three cars in the intersection, 1 => there are 3 or more
 *
 * Notes:
 *      This function checks if it is safe to go into the intersetction.
 *      Idea is to use this in conjunction with a cv to prevent having more than 3 cars in the
 *      intersection at any given time (4 cars => deadlock).
 */

static int
checkintersection()
{
    
    int cars_in_intersection = 0;

    if(NW_quad->locked) cars_in_intersection++;
    if(NE_quad->locked) cars_in_intersection++;
    if(SE_quad->locked) cars_in_intersection++;
    if(SW_quad->locked) cars_in_intersection++;

    //assert(cars_in_intersection < NQUADRANTS); // Should never have a car in every quadrant

    if (cars_in_intersection <= MAX_CARS_ALLOWED) return(0);
    else return(1);

    //should never get here;
}

static void
message(int msg_nr, int carnumber, int cardirection, int destdirection)
{
    kprintf("%s car = %2d, direction = %s, destination = %s\n",
            msgs[msg_nr], carnumber,
            directions[cardirection], directions[destdirection]);
}

void
entersafely(unsigned long cardirection, int destdirection, unsigned long carnumber, struct lock *desired_lock,
            struct lock *incoming_car, struct cv *danger_to_me)
{
    //get the required lock
    lock_acquire(desired_lock);

    /*
     * Now once you have the lock, check for "incoming traffic".
     * If a car is coming (i.e. the quadrant to the "left" of the one you
     * want to enter is locked), wait until it is safe (unlocked). 
     * Also turn interrupts on, in case the while loop says you're clear
     * and you get interrupted by a process that locks the quadrant you just checked.
     */

    int spl = splhigh();

    while(checkintersection()){
        cv_wait(intersection_full,desired_lock);
    }

    //once the way is clear, proceed into the region 
    message(REGION1, carnumber, cardirection, destdirection);

    //return interrupts to previous state
    splx(spl);    

    return;                
}
 
/*
 * enterintersection()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      struct lock*, points to the corresponding lock of the quadrant just entered
 *
 * Notes:
 *      This function implements moving forward into the first quadrant 
 *      of the intersection from any direction.
 */

struct lock * 
enterintersection(unsigned long cardirection, int destdirection, unsigned long carnumber)
{
    // Find which lock is needed and which one to monitor for our cv, based on intersection entry point.
    // Use global struct array for look up.
    struct lock* desired_lock = paths[cardirection].right_lock;

    // Find which cv is needed to enter intersection safely based on entry point.
    // Use global struct array for lookup
    
    // Now we have all the parameters to try and enter safely
    entersafely(cardirection, destdirection, carnumber, desired_lock, NULL, NULL);

    return (desired_lock);
}


/*
 * gostraight()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the cardirection
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
gostraight(unsigned long cardirection,
           unsigned long carnumber)
{
    int spl = splhigh();
    
    // Find destination using look up in global array of structs 
    int destdirection = paths[cardirection].straight_dest;

    //print out that car has approached the intersection
    lock_acquire(entry_lanes[cardirection]);
    message(APPROACHING, carnumber, cardirection, destdirection);

    // Enter the intersection.
    // See "enterintersection" func def for more details.
    struct lock* current_location = enterintersection(cardirection, destdirection, carnumber);
    lock_release(entry_lanes[cardirection]);
    
    thread_yield();
    // Once we are in the intersection, we need to acquire the lock for our next quadrant.
    // When that happens, signal to cars for whom we were dangerous that we have cleared our previous quadrant
    // and release the corresponding lock. All done atomically to avoid holding two locks simultaneously. 
    
    lock_acquire(paths[cardirection].straight_lock);
    lock_release(current_location);
    message(REGION2, carnumber, cardirection, destdirection);
    splx(spl);
    thread_yield();

    // Going straight => you can now leave having passed through two quadrants.
    // Release the lock once you have left (this must happened atomically)
    spl = splhigh();
    message(LEAVING, carnumber, cardirection, destdirection);
    cv_signal(intersection_full, paths[cardirection].straight_lock);
    lock_release(paths[cardirection].straight_lock);
    num_finished ++;
    splx(spl);
}


/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnleft(unsigned long cardirection,
         unsigned long carnumber)
{
    int spl = splhigh();
    // Find destination using look up in global array of structs 
    int destdirection = paths[cardirection].left_dest;

    //print out that car has approached the intersection
    lock_acquire(entry_lanes[cardirection]);
    message(APPROACHING, carnumber, cardirection, destdirection);    

    // Enter the intersection.
    // See "enterintersection" func def for more details.
    struct lock* current_location = enterintersection(cardirection, destdirection, carnumber);
    lock_release(entry_lanes[cardirection]);
    thread_yield();

    // Once we are in the intersection, we need to acquire the lock for our next quadrant.
    // When that happens, signal to cars for whom we were dangerous that we have cleared our previous quadrant
    // and release the corresponding lock. All done atomically to avoid holding two locks simultaneously. 
    
    lock_acquire(paths[cardirection].straight_lock);
    
    lock_release(current_location);
    message(REGION2, carnumber, cardirection, destdirection);
    splx(spl);

    thread_yield();

    spl = splhigh();
    lock_acquire(paths[cardirection].left_lock);
    
    lock_release(paths[cardirection].straight_lock);
    message(REGION3, carnumber, cardirection, destdirection);
    splx(spl);
    thread_yield();

    // Gone through three regions => Time to leave.
    // Release the lock once you have left (this must happened atomically)
    spl = splhigh();
    message(LEAVING, carnumber, cardirection, destdirection);
    cv_signal(intersection_full, paths[cardirection].left_lock);
    lock_release(paths[cardirection].left_lock);
    num_finished ++;
    splx(spl);
}


/*
 * turnright()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnright(unsigned long cardirection,
          unsigned long carnumber)
{
    int spl = splhigh();

    //find destination
    int destdirection = paths[cardirection].right_dest;

    //print out that car has approached the inter section
    lock_acquire(entry_lanes[cardirection]);
    message(APPROACHING, carnumber, cardirection, destdirection);
    thread_yield();
    

    // Enter the intersection.
    // See "enterintersection" func def for more details.
    struct lock* current_location = enterintersection(cardirection, destdirection, carnumber);
    lock_release(entry_lanes[cardirection]);
    
    thread_yield();
   
    message(LEAVING, carnumber, cardirection, destdirection);

    // Once you leave => intersection must have < MAX_CARS_ALLOWED => signal the waiting cars
    cv_signal(intersection_full, current_location);
    lock_release(current_location);
    num_finished ++;
    splx(spl);
}


/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
 
static
void
approachintersection(void * unusedpointer,
                     unsigned long carnumber)
{
    while(cars_running < NCARS){
        thread_yield();
    }

    int cardirection;
    int turn;

    // cardirection is set randomly and turn set randomly
    // value of cardirection maps to index in global directions array

    cardirection = random() % 4;
    turn = random() % 3;

    if (turn == LEFT) turnleft(cardirection, carnumber);
    else if (turn == STRAIGHT) gostraight(cardirection, carnumber);
    else if (turn == RIGHT) turnright(cardirection, carnumber);
    
    /*
     * Avoid unused variable and function warnings.
     */

    (void) unusedpointer;     
}


/*
 * createcars()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

int
createcars(int nargs,
           char ** args)
{
    int index, error;

    /*
     * Avoid unused variable warnings.
     */

    (void) nargs;
    (void) args;     

    /*
     * Init global locks, and cv's
     */

    intersection_full = cv_create("intersection_full");

    NW_quad = lock_create("NW_quad");
    NE_quad = lock_create("NE_quad");
    SE_quad = lock_create("SE_quad");
    SW_quad = lock_create("SW_quad");

    N_entry = lock_create("N_entry");
    E_entry = lock_create("E_entry");
    S_entry = lock_create("S_entry");
    W_entry = lock_create("W_entry");

    entry_lanes[NORTH] = N_entry;
    entry_lanes[EAST] = E_entry;
    entry_lanes[SOUTH] = S_entry;
    entry_lanes[WEST] = W_entry;
        
    init_paths();

    cars_running = 0;
    num_finished = 0;

    /*
     * Start NCARS approachintersection() threads.
     */
 
    for (index = 0; index < NCARS; index++) {

            error = thread_fork("approachintersection thread",
                                NULL,
                                index,
                                approachintersection,
                                NULL
                                );

            /*
             * panic() on error.
             */

            if (error) {
                    
                    panic("approachintersection: thread_fork failed: %s\n",
                          strerror(error)
                          );
            }

            cars_running++;
    }

    //KEEP MAIN THREAD FROM DYING
    while (num_finished<NCARS){
        //loop until all car threads finished
    }

    //free lock/cv memory 
    lock_destroy(NW_quad);
    lock_destroy(NE_quad);
    lock_destroy(SE_quad);
    lock_destroy(SW_quad);
    lock_destroy(N_entry);
    lock_destroy(E_entry);
    lock_destroy(S_entry);
    lock_destroy(W_entry);
    

    cv_destroy(intersection_full);

    return 0;
}
