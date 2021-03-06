/* Author(s): <Your name here>
 * Defines the memory manager for the kernel.
*/

#ifndef MEMORY_H
#define MEMORY_H

enum {
  /* physical page facts */
  PAGE_SIZE = 4096,
  PAGE_N_ENTRIES = (PAGE_SIZE / sizeof(uint32_t)),

  // Global bit
  PE_G = (0x40 >> 6),
  // Valid bit
  PE_V = (0x80 >> 6),
  // Writable bit
  PE_D = (0x100 >> 6),
  // Uncache bit
  PE_UC = (0x400 >> 6),

  /* Constants to simulate a very small physical memory. */
  PAGEABLE_PAGES = 5,
};

/* DONE: Structure of an entry in the page map */
typedef struct {
    node_t p_node;
    uint32_t paddr;
    uint32_t vaddr;
    uint32_t disk_addr;
    int pinned;
    int is_used;
    int index;
    int pid;
} page_map_entry_t;


/* Prototypes */

/* Return the physical address of the i-th page */
uint32_t* page_addr(int i);

/* Allocate a page.  If necessary, swap a page out.
 * On success, return the index of the page in the page map.  On
 * failure, abort.  BUG: pages are not made free when a process
 * exits.
 */
int page_alloc(int pinned);

/* init page_map */
uint32_t init_memory(void);

/* Set up a page directory and page table for the given process. Fill in
 * any necessary information in the pcb.
 */
uint32_t setup_page_table(int pid);

// other functions defined here
//
void refresh_page_map(int cindex, uint32_t vaddr, uint32_t daddr, uint32_t flag, int pid);
void refresh_page_map_s(int sindex, uint32_t vaddr, int pid);
void refresh_page_queue(uint32_t bad_addr);
#endif /* !MEMORY_H */
