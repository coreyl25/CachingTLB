#include "vm_memory.h"

void init_simple_page_table(SimplePageTable *pt) {
    pt->size = NUM_PAGES;
    pt->entries = (PageTableEntry *)calloc(pt->size, sizeof(PageTableEntry));
    pt->accesses = 0;
    pt->hits = 0;
    pt->faults = 0;
    
    if (!pt->entries) {
        fprintf(stderr, "Failed to allocate memory for simple page table\n");
        exit(1);
    }
    
    printf("Simple page table initialized with %u entries\n", pt->size);
}

void cleanup_simple_page_table(SimplePageTable *pt) {
    if (pt->entries) {
        free(pt->entries);
        pt->entries = NULL;
    }
}

uint32_t translate_simple_page_table(SimplePageTable *pt, uint32_t virtual_addr, bool *fault) {
    uint32_t page_number = get_page_number(virtual_addr);
    uint32_t page_offset = get_page_offset(virtual_addr);
    
    pt->accesses++;
    
    if (page_number >= pt->size) {
        *fault = true;
        pt->faults++;
        return 0;
    }
    
    PageTableEntry *entry = &pt->entries[page_number];
    
    if (!entry->valid) {
        *fault = true;
        pt->faults++;
        
        // Simulate page fault handling - allocate a frame
        static uint32_t next_frame = 0;
        entry->frame_number = next_frame % NUM_PHYSICAL_FRAMES;
        entry->valid = true;
        entry->referenced = true;
        entry->dirty = false;
        next_frame++;
        
        return (entry->frame_number << PAGE_OFFSET_BITS) | page_offset;
    }
    
    *fault = false;
    pt->hits++;
    entry->referenced = true;
    
    return (entry->frame_number << PAGE_OFFSET_BITS) | page_offset;
}