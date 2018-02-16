//coremap.h
#ifndef _COREMAP_H_
#define _COREMAP_H_

#include <types.h>

#define PADDR_TO_COREMAP(paddr) ((paddr)/PAGE_SIZE - firstcoremappage)

//A coremap keeps track of which pages are in use and also keeps track of which address spaces a page is mapped into. 
//http://www.eecg.toronto.edu/~lie/Courses/ECE344/asst3.shtml old assignment handout

//test comment

typedef struct coremapObject {
	// unsigned int ppn; //key = physical page number. should be rendundant because array stores physical page
	int used;  //0 if empty, 1 else
	// int modified;//1 if mm was written to
	// struct addrspace* as;

	int length;

	paddr_t paddr;	
	vaddr_t vaddr;

	paddr_t startAddr;

	// int timein;

	// for eviction
	// int32_t timestamp;

	// nanoseconds, if necessary
	// u_int32_t timestampNano;

} coremapObject;

extern u_int32_t totalCoremapEntries;//total number of entries available for the coremap
extern u_int32_t coremapEntriesFree;
extern u_int32_t firstcoremappage;
extern coremapObject* coremap;

void init_coremap();
paddr_t find_pages_in_core_map(unsigned long npages);
void coremap_free(paddr_t addr);

// paddr_t evict_coremap_entry();

#endif
