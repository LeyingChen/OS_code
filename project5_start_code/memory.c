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
#include "queue.h"

#define MEM_START 0xa0908000

node_t page_queue;

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
    int i, free_index = -1;
    for(i=0; i<PAGEABLE_PAGES; i++){
        if(page_map[i].is_used == 0){  //find an unused frame
            free_index = i;
            break;
        }
    }
    if(free_index == -1){ // no frame avaliable
        //may need to write back
        //node_t *item_to_swap;
        page_map_entry_t *page_to_swap;
        //item_to_swap = dequeue(&page_queue);
        page_to_swap = (page_map_entry_t *)dequeue(&page_queue);
        free_index = page_to_swap->index;
        //uint32_t pt_addr = current_running->page_table;
        uint32_t pt_addr = pcb[page_to_swap->pid].page_table;
        insert_page_table_entry((uint32_t *)pt_addr, pcb[page_to_swap->pid].entry_point, 
            0x00000000, 0x0, page_to_swap->pid);
        //printk("swap page %d\n", free_index);
    }
    page_map[free_index].is_used = 1;
    page_map[free_index].pinned = pinned;
    if(!pinned){
        //printk("unpinned page %d\n", free_index);
        enqueue(&page_queue, (node_t *)(&page_map[free_index]));
    }
    bzero((char *)page_map[free_index].paddr, PAGE_SIZE);
    return free_index;

    ASSERT( free_index < PAGEABLE_PAGES );
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
        page_map[i].index = i;
        page_map[i].p_node.prev = NULL;
        page_map[i].p_node.next = NULL;
    }
    queue_init(&page_queue);
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
    printk("page table %d pinned: %d index:%d\n", pid, page_map[frame_index].pinned, frame_index);
    // initialize PTE and insert several entries into page tables using insert_page_table_entry
    int i;
    page_table = page_map[frame_index].paddr;
    int page_num;
    page_num = pcb[pid].size/PAGE_SIZE;
    uint32_t pte_flag = 0x04;  //C=000, D=1, V=0, G=0
    for(i=0; i<page_num; i++){
        //int code_findex;
        //code_findex = page_alloc(0); //alloc a page for code
        //page_map[code_findex].disk_addr = pcb[pid].loc + i*PAGE_SIZE; 
        //page_map[code_findex].vaddr = pcb[pid].entry_point + i*PAGE_SIZE;
        //bcopy((char *)page_map[code_findex].disk_addr,
        //    (char *)(page_map[code_findex].paddr+i*(PAGE_SIZE)), PAGE_SIZE);//copy to frame directly
        insert_page_table_entry((uint32_t *)page_table, pcb[pid].entry_point,
            0x00000000, pte_flag, pid);
    }

    return page_table;
}

void refresh_page_map(int cindex, uint32_t vaddr, uint32_t daddr, uint32_t flag, int pid){
    page_map[cindex].pid = pid;
    page_map[cindex].vaddr = vaddr;
    page_map[cindex].disk_addr = daddr;
    bcopy((char *)(page_map[cindex].disk_addr), (char *)(page_map[cindex].paddr), PAGE_SIZE);
    insert_page_table_entry((uint32_t *)(pcb[pid].page_table), page_map[cindex].vaddr, 
        page_map[cindex].paddr, flag, pid);
}

uint32_t do_tlb_miss(uint32_t vaddr, int pid) {
    return 0;
}

void create_pte(uint32_t vaddr, int pid) {
    return;
}
