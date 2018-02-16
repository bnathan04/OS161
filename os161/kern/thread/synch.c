/*
 * Synchronization primitives.
 * See synch.h for specifications of the functions.
 */

#include <types.h>
#include <lib.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <machine/spl.h>

//added includes
#include <queue.h> 


////////////////////////////////////////////////////////////
//
// Semaphore.

//initial count is the amount of resources avialable right now
struct semaphore *
sem_create(const char *namearg, int initial_count)
{
	struct semaphore *sem;

	assert(initial_count >= 0);

	sem = kmalloc(sizeof(struct semaphore));
	if (sem == NULL) {
		return NULL;
	}

	sem->name = kstrdup(namearg);
	if (sem->name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->count = initial_count;
	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	spl = splhigh();
	assert(thread_hassleepers(sem)==0);
	splx(spl);

	/*
	 * Note: while someone could theoretically start sleeping on
	 * the semaphore after the above test but before we free it,
	 * if they're going to do that, they can just as easily wait
	 * a bit and start sleeping on the semaphore after it's been
	 * freed. Consequently, there's not a whole lot of point in 
	 * including the kfrees in the splhigh block, so we don't.
	 */

	kfree(sem->name);
	kfree(sem);
}

//taking resource
void 
P(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	assert(in_interrupt==0);

	spl = splhigh();
	while (sem->count==0) {
		thread_sleep(sem);
	}
	assert(sem->count>0);
	sem->count--;
	splx(spl);
}

//releasing resource
void
V(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);
	spl = splhigh();
	sem->count++;
	assert(sem->count>0);
	thread_wakeup(sem);
	splx(spl);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;

	lock = kmalloc(sizeof(struct lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->name = kstrdup(name);
	if (lock->name == NULL) {
		kfree(lock);
		return NULL;
	}

	//Balu - Oct 7/2017
	lock->locked = 0; // set lock to unlocked when created 
	lock->lock_holder = NULL; // initially, no one is holding the lock
	
	return lock;
}

void
lock_destroy(struct lock *lock)
{	
	int spl;

	//Balu - Oct 7/2017
	//make sure a valid lock address is passed, and no one is holding the lock
	assert(lock != NULL);
	assert(lock->locked == 0)
	assert(lock->lock_holder == NULL)
	
	//turn off interrupts and check if the lock has any sleepers
	spl = splhigh();
	assert(thread_hassleepers(lock)==0);
	splx(spl);	

	//Balu - Oct 8/2017
	//make sure to free all pointers within lock before freeing lock struct		
	kfree(lock->name);
	lock->name = NULL;
	kfree(lock->lock_holder); 
	lock->lock_holder = NULL;
	lock = NULL;
	
	kfree(lock);
	lock = NULL;
}

void
lock_acquire(struct lock *lock)
{
	// Write this

	//Balu - Oct 7/2017
	int spl;
	assert (lock != NULL); //make sure the passed address actually contains something	

	//turn interrupts off to make interaction with lock atomic
	spl = splhigh();
	while(lock->locked){
		thread_sleep(lock);
	}

	//checking our assumptions that the lock is released and doesn't still have a holder	
	assert (lock->locked == 0);
	assert (lock->lock_holder == NULL); 

	//acquire the lock, and identify the curthread as the holder 
	lock->locked = 1;
	lock->lock_holder = curthread;																	//MAKE SURE THIS LINE DOES WHAT IS INTENDED

	//return interupts to previous state
	splx(spl);

	return;
}

void
lock_release(struct lock *lock)																				//THE RELEASE ASSUMES YOU CALL "lock_do_i_hold" FIRST
{
	
	//Balu - Oct 8/2017
	int spl;
	assert (lock != NULL);

	//turn interrupts off to make interaction with the lock atomic
	spl = splhigh();

	//checking our assumptions that the lock is currently locked and has a holder	
	assert (lock->locked == 1);
	assert (lock->lock_holder != NULL); 

	//unlock and clear the lock_holder identification
	lock->locked = 0;
	lock->lock_holder = NULL;

	//wake up threads waiting on this lock
	thread_wakeup(lock);

	//return interrupts to previous state
	splx(spl);

	return;

	//(void)lock;  // suppress warning until code gets written

}

int
lock_do_i_hold(struct lock *lock)
{

	//Balu - Oct 8/2017
	int spl;
	int ret_val; 
	assert (lock != NULL);

	//turn interrupts off to make interaction with lock atomic
	spl = splhigh();

	//check the lock
	if (lock->lock_holder == curthread) ret_val = 1;
	else ret_val = 0;
	//(void)lock;  // suppress warning until code gets written

	//return interrupts to previous state
	splx(spl);

	return (ret_val); 

}

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(struct cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->name = kstrdup(name);
	if (cv->name==NULL) {
		kfree(cv);
		return NULL;
	}
	

	//create an empty wait q
	cv->wait_q = q_create(1); 
	int error = q_preallocate(cv->wait_q,(20*(sizeof(struct thread))));
	assert(error == 0);


	return cv;
}

void
cv_destroy(struct cv *cv)
{
	//checks for valid cv pointer, and no one sleeping on the cv
	assert(cv != NULL);
	assert(q_empty(cv->wait_q));

	// add stuff here as needed
	
	//free all elements of the struct before freeing entire struct
	kfree(cv->name);
	cv->name = NULL;
	q_destroy(cv->wait_q);
	cv->wait_q = NULL;

	kfree(cv);
	cv = NULL;
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	// Write this
	//Balu - Oct 8/2017

	int spl;

	//check that the cv and lock refs are valid
	assert(cv!=NULL);
	assert(lock!=NULL);

	//turn interrupts off
	spl = splhigh();

	//make sure the you hold the lock and release it
	assert(lock_do_i_hold(lock));
	lock_release(lock);

	//make the curthread's sleep address key itself
	curthread->t_sleepaddr = curthread;

	//put the curthread into the wait q
	q_addtail(cv->wait_q,curthread);

	//put the thread to sleep
	thread_sleep(curthread);

	//return interrupts to previous state
	splx(spl);

	//once the thread wakes up again, reacquire lock
	lock_acquire(lock);
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	assert(cv!=NULL);
	assert(lock!=NULL);

	//turn interrupts off
	int spl = splhigh();

	//make sure I hold the lock
	assert(lock_do_i_hold(lock));

	//wake up the thread at top of the cv wait q
	if(!q_empty(cv->wait_q)) thread_wakeup(((struct thread*)(q_remhead(cv->wait_q)))->t_sleepaddr);
	
	//return interrupts back to prev state
	splx(spl);
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	assert(cv!=NULL);
	assert(lock!=NULL);

	//turn interrupts off 
	int spl = splhigh();
	
	//check if I hold the lock, and 
	//repeatedly call cv_signal until every thread in waitq has been signaled
	assert(lock_do_i_hold(lock));
	while (!(q_empty(cv->wait_q))){
		cv_signal(cv, lock);
	}

	//return interrupts to previous state
	splx(spl); 
}
