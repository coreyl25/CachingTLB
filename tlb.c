#include "vm_memory.h"

void init_tlb(TLB *tlb) {
    tlb->size = TLB_SIZE;
    tlb->entries = (TLBEntry *)calloc(tlb->size, sizeof(TLBEntry));
    tlb->next_replace = 0;
    tlb->accesses = 0;
    tlb->hits = 0;
    tlb->misses = 0;
    
    if (!tlb->entries) {
        fprintf(stderr, "Failed to allocate memory for TLB\n");
        exit(1);
    }
    
    printf("TLB initialized with %u entries\n", tlb->size);
}

void cleanup_tlb(TLB *tlb) {
    if (tlb->entries) {
        free(tlb->entries);
        tlb->entries = NULL;
    }
}

bool tlb_lookup(TLB *tlb, uint32_t virtual_page, uint32_t *physical_frame) {
    tlb->accesses++;
    
    // Search all TLB entries
    for (uint32_t i = 0; i < tlb->size; i++) {
        TLBEntry *entry = &tlb->entries[i];
        if (entry->valid && entry->virtual_page == virtual_page) {
            tlb->hits++;
            entry->referenced = true;
            *physical_frame = entry->physical_frame;
            return true;
        }
    }
    
    tlb->misses++;
    return false;
}

void tlb_insert(TLB *tlb, uint32_t virtual_page, uint32_t physical_frame) {
    // Use round-robin replacement policy
    TLBEntry *entry = &tlb->entries[tlb->next_replace];
    
    entry->valid = true;
    entry->virtual_page = virtual_page;
    entry->physical_frame = physical_frame;
    entry->referenced = true;
    entry->dirty = false;
    
    tlb->next_replace = (tlb->next_replace + 1) % tlb->size;
}

void tlb_invalidate_all(TLB *tlb) {
    for (uint32_t i = 0; i < tlb->size; i++) {
        tlb->entries[i].valid = false;
    }
    printf("TLB invalidated\n");
}

void tlb_print_contents(TLB *tlb) {
    printf("\nTLB Contents:\n");
    printf("Index | Valid | Virtual Page | Physical Frame | Referenced\n");
    printf("------|-------|-------------|----------------|-----------\n");
    
    for (uint32_t i = 0; i < tlb->size; i++) {
        TLBEntry *entry = &tlb->entries[i];
        printf("  %2u  |   %c   |   0x%06X   |     0x%04X     |     %c\n",
               i, entry->valid ? 'Y' : 'N', entry->virtual_page, 
               entry->physical_frame, entry->referenced ? 'Y' : 'N');
    }
    printf("\n");
}