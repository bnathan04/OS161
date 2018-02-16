#ifndef _PAGETABLE_H_
#define _PAGETABLE_H_
//pagetable.h

// #include <array.h>

// #include <../arch/mips/include/ktypes.h>

//page tables map virtual addresses(addresses that cpu thinks it's addressing)
//to locations in main memory if they exist

typedef struct page{

	// physical and virtual page numbers
	int ppagenum;
	// int vpagenum; /* UNSURE IF VPAGE NUM IS NECESSARY*/

	// physical and virtual address bases
	unsigned int vaddr;
	unsigned int paddr;

	/* UNSURE IF PERMISSIONS NEED TO BE TRACKED*/
	// int read;
	// int write;
	// int exec;

	// pointer to next element
	struct page* next;

	// int valid;

} page;

typedef struct page* pagetable;

page* page_create(unsigned int faultaddr, int npages);
void page_destroy(page* victim);
void pagetable_destroy(page* table);

// int find_next_pagetable_index();
page* find_page(unsigned int vaddr);

void invalidate_page(page* invalid);

pagetable copy_pagetable(pagetable table);




// typedef struct+ array page_table;

// page_table* init_page_table();

//check if there exists a page associated with the vaddr given
int page_in_pagetable(unsigned int vaddr);

#endif /* _PAGETABLE_H_ */
