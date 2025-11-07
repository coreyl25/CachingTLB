#include "vm_memory.h"

void init_two_level_page_table(TwoLevelPageTable *pt) {
    pt->l1_size = L1_SIZE;
    pt->l1_table = (PageTableEntry **)calloc(pt->l1_size, sizeof(PageTableEntry *));
    pt->l1_valid = (bool *)calloc(pt->l1_size, sizeof(bool));
    pt->accesses = 0;
    pt->hits = 0;
    pt->faults = 0;
    
    if (!pt->l1_table || !pt->l1_valid) {
        fprintf(stderr, "Failed to allocate memory for two-level page table\n");
        exit(1);
    }
    
    printf("Two-level page table initialized with L1 size: %u\n", pt->l1_size);
}

void cleanup_two_level_page_table(TwoLevelPageTable *pt) {
    if (pt->l1_table) {
        // Free all allocated L2 tables
        for (uint32_t i = 0; i < pt->l1_size; i++) {
            if (pt->l1_valid[i] && pt->l1_table[i]) {
                free(pt->l1_table[i]);
            }
        }
        free(pt->l1_table);
        pt->l1_table = NULL;
    }
    
    if (pt->l1_valid) {
        free(pt->l1_valid);
        pt->l1_valid = NULL;
    }
}

uint32_t translate_two_level_page_table(TwoLevelPageTable *pt, uint32_t virtual_addr, bool *fault) {
    uint32_t l1_index = get_l1_index(virtual_addr);
    uint32_t l2_index = get_l2_index(virtual_addr);
    uint32_t page_offset = get_page_offset(virtual_addr);
    
    pt->accesses++;
    
    // Check if L1 entry is valid
    if (!pt->l1_valid[l1_index]) {
        *fault = true;
        pt->faults++;
        
        // Allocate L2 table on demand
        pt->l1_table[l1_index] = (PageTableEntry *)calloc(L2_SIZE, sizeof(PageTableEntry));
        if (!pt->l1_table[l1_index]) {
            fprintf(stderr, "Failed to allocate L2 page table\n");
            exit(1);
        }
        pt->l1_valid[l1_index] = true;
        
        // Set up the new page entry
        PageTableEntry *entry = &pt->l1_table[l1_index][l2_index];
        static uint32_t next_frame = 0;
        entry->frame_number = next_frame % NUM_PHYSICAL_FRAMES;
        entry->valid = true;
        entry->referenced = true;
        entry->dirty = false;
        next_frame++;
        
        return (entry->frame_number << PAGE_OFFSET_BITS) | page_offset;
    }
    
    // L1 is valid, check L2 entry
    PageTableEntry *entry = &pt->l1_table[l1_index][l2_index];
    
    if (!entry->valid) {
        *fault = true;
        pt->faults++;
        
        // Allocate physical frame for this page
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