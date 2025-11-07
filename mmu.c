#include "vm_memory.h"

void init_mmu(MMU *mmu) {
    init_tlb(&mmu->tlb);
    init_two_level_page_table(&mmu->page_table);
    
    mmu->physical_memory = (uint32_t *)calloc(NUM_PHYSICAL_FRAMES * PAGE_SIZE / sizeof(uint32_t), sizeof(uint32_t));
    mmu->frame_allocated = (bool *)calloc(NUM_PHYSICAL_FRAMES, sizeof(bool));
    mmu->next_free_frame = 0;
    mmu->total_cycles = 0;
    
    if (!mmu->physical_memory || !mmu->frame_allocated) {
        fprintf(stderr, "Failed to allocate physical memory simulation\n");
        exit(1);
    }
    
    printf("MMU initialized with TLB and two-level page table\n");
}

void cleanup_mmu(MMU *mmu) {
    cleanup_tlb(&mmu->tlb);
    cleanup_two_level_page_table(&mmu->page_table);
    
    if (mmu->physical_memory) {
        free(mmu->physical_memory);
        mmu->physical_memory = NULL;
    }
    
    if (mmu->frame_allocated) {
        free(mmu->frame_allocated);
        mmu->frame_allocated = NULL;
    }
}

uint32_t mmu_translate(MMU *mmu, uint32_t virtual_addr) {
    uint32_t virtual_page = get_page_number(virtual_addr);
    uint32_t page_offset = get_page_offset(virtual_addr);
    uint32_t physical_frame;
    
    // First, check TLB
    if (tlb_lookup(&mmu->tlb, virtual_page, &physical_frame)) {
        // TLB hit
        mmu->total_cycles += TLB_HIT_TIME;
        return (physical_frame << PAGE_OFFSET_BITS) | page_offset;
    }
    
    // TLB miss, check page table
    bool page_fault;
    uint32_t physical_addr = translate_two_level_page_table(&mmu->page_table, virtual_addr, &page_fault);
    
    if (page_fault) {
        // Page fault occurred
        mmu->total_cycles += PAGE_FAULT_TIME;
        physical_frame = get_page_number(physical_addr);
        
        // Insert into TLB after page fault handling
        tlb_insert(&mmu->tlb, virtual_page, physical_frame);
    } else {
        // Page table hit
        mmu->total_cycles += PAGE_TABLE_ACCESS_TIME;
        physical_frame = get_page_number(physical_addr);
        
        // Insert into TLB
        tlb_insert(&mmu->tlb, virtual_page, physical_frame);
    }
    
    return physical_addr;
}

uint32_t allocate_physical_frame(MMU *mmu) {
    // Simple round-robin frame allocation
    uint32_t frame = mmu->next_free_frame % NUM_PHYSICAL_FRAMES;
    mmu->frame_allocated[frame] = true;
    mmu->next_free_frame++;
    return frame;
}

void mmu_print_stats(MMU *mmu) {
    printf("\nMMU Statistics:\n");
    printf("TLB Accesses: %lu\n", mmu->tlb.accesses);
    printf("TLB Hits: %lu\n", mmu->tlb.hits);
    printf("TLB Misses: %lu\n", mmu->tlb.misses);
    printf("TLB Hit Rate: %.2f%%\n", 
           mmu->tlb.accesses > 0 ? (double)mmu->tlb.hits / mmu->tlb.accesses * 100 : 0);
    
    printf("Page Table Accesses: %lu\n", mmu->page_table.accesses);
    printf("Page Table Hits: %lu\n", mmu->page_table.hits);
    printf("Page Faults: %lu\n", mmu->page_table.faults);
    printf("Page Hit Rate: %.2f%%\n", 
           mmu->page_table.accesses > 0 ? (double)mmu->page_table.hits / mmu->page_table.accesses * 100 : 0);
    
    printf("Total Cycles: %lu\n", mmu->total_cycles);
}