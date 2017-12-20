/* Author(s): <Your name here>
 * Implementation of the memory manager for the kernel.
 */

#include "common.h"
#include "interrupt.h"
#include "kernel.h"
#include "memory.h"
#include "printf.h"
#include "scheduler.h"
#include "util.h"

#define MEM_START 0xa0908000

/* Static global variables */
// Keep track of all pages: their vaddr, status, and other properties
static page_map_entry_t page_map[ PAGEABLE_PAGES ];

// other global variables...
/*static lock_t page_fault_lock;*/

/* DONE: Returns physical address of page number i */
uint32_t page_vaddr( int i ) {
    return page_map[i].vaddr;
}

/* DONE: Returns virtual address (in kernel) of page number i */
uint32_t page_paddr( int i ) {
    return page_map[i].paddr;
}

/* get the physical address from virtual address (in kernel) */
uint32_t va2pa( uint32_t va ) {
    return (uint32_t) va - 0xa0000000;
}

/* get the virtual address (in kernel) from physical address */
uint32_t pa2va( uint32_t pa ) {
    return (uint32_t) pa + 0xa0000000;
}


// DONE: insert page table entry to page table
void insert_page_table_entry( uint32_t *table, uint32_t vaddr, uint32_t paddr,
                              uint32_t flag, uint32_t pid ) {
    // insert entry
    uint32_t pte_content;
    pte_content = ((paddr&0xfffff000)>>6) | (flag&0x0000003f);
    //int page_order = (vaddr - pcb[pid].entry_point)/PAGE_SIZE;
    table[vaddr>>12] = pte_content;

    // tlb flush
    uint32_t entry_hi;
    entry_hi = (vaddr&0xfffff000) | (pid&0x00000fff);
    tlb_flush(entry_hi);
}

/* DONE: Allocate a page. Return page index in the page_map directory.
 *
 * Marks page as pinned if pinned == TRUE.
 * Swap out a page if no space is available.
 */
int page_alloc( int pinned ) {
    int i, free_index;
    for(i=0; i<PAGEABLE_PAGES; i++){
        if(page_map[i].is_used == 0){  //find an unused map
            page_map[i].is_used = 1;
            page_map[i].pinned = pinned;
            free_index = i;
            break;
        } else free_index = -1; //no page swap currently
    } /* PAGE REPLACE */
    bzero(page_map[i].paddr, PAGE_SIZE);
    ASSERT( free_index < PAGEABLE_PAGES );
    return free_index;
}

/* DONE:
 * This method is only called once by _start() in kernel.c
 */
uint32_t init_memory( void ) {
    int i;
    // initialize all pageable pages to a default state
    for(i=0; i<PAGEABLE_PAGES; i++){
        page_map[i].paddr = MEM_START + i * PAGE_SIZE;
        page_map[i].vaddr = 0;
        page_map[i].disk_addr = 0;
        page_map[i].is_used = 0;
        page_map[i].pinned = 0;
    }
    return 0;
}

/* DONE:
 *
 */
uint32_t setup_page_table( int pid ) { //pid refers to the order of pcb(begin with 0)
    uint32_t page_table;
    // alloc page for page table
    int frame_index;
    frame_index = page_alloc(1); //page table can't be replace, it is pinned

    // initialize PTE and insert several entries into page tables using insert_page_table_entry
    int i;
    page_table = page_map[frame_index].paddr;
    int page_num;
    page_num = pcb[pid].size/PAGE_SIZE;
    uint32_t pte_flag = 0x2;
    for(i=0; i<page_num; i++){
        int code_findex;
        code_findex = page_alloc(0); //alloc a page for code
        //uint32_t p_vaddr = pcb[pid].entry_point;
        page_map[code_findex].disk_addr = pcb[pid].loc + i*PAGE_SIZE; 
        page_map[code_findex].vaddr = pcb[pid].entry_point + i*PAGE_SIZE;
        bcopy((char *)page_map[code_findex].disk_addr,
            (char *)(page_map[code_findex].paddr+i*(PAGE_SIZE)), PAGE_SIZE);//copy to frame directly
        insert_page_table_entry((uint32_t *)page_table, page_map[code_findex].vaddr,
            page_map[code_findex].paddr+i*(PAGE_SIZE), pte_flag, pid);
        //printk("pid: %d src addr:0x%08x dst addr:0x%08x\n",
        //    pid, (char *)(pcb[pid].loc+i*(PAGE_SIZE)), (char *)(page_map[code_findex].paddr+i*(PAGE_SIZE)));
    }

    return page_table;
}

uint32_t do_tlb_miss(uint32_t vaddr, int pid) {
    return 0;
}

void create_pte(uint32_t vaddr, int pid) {
    return;
}
