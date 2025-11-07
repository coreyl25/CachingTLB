#include "vm_memory.h"

uint32_t get_page_number(uint32_t virtual_addr) {
    return virtual_addr >> PAGE_OFFSET_BITS;
}

uint32_t get_page_offset(uint32_t virtual_addr) {
    return virtual_addr & PAGE_OFFSET_MASK;
}

uint32_t get_l1_index(uint32_t virtual_addr) {
    return (virtual_addr >> (PAGE_OFFSET_BITS + L2_BITS)) & L1_INDEX_MASK;
}

uint32_t get_l2_index(uint32_t virtual_addr) {
    return (virtual_addr >> PAGE_OFFSET_BITS) & L2_INDEX_MASK;
}

void print_address_breakdown(uint32_t virtual_addr) {
    uint32_t page_number = get_page_number(virtual_addr);
    uint32_t page_offset = get_page_offset(virtual_addr);
    uint32_t l1_index = get_l1_index(virtual_addr);
    uint32_t l2_index = get_l2_index(virtual_addr);
    
    printf("Address: 0x%08X\n", virtual_addr);
    printf("  Binary: ");
    for (int i = 31; i >= 0; i--) {
        printf("%d", (virtual_addr >> i) & 1);
        if (i == PAGE_OFFSET_BITS || i == PAGE_OFFSET_BITS + L2_BITS) {
            printf(" | ");
        }
    }
    printf("\n");
    printf("  L1 Index: %u (0x%X)\n", l1_index, l1_index);
    printf("  L2 Index: %u (0x%X)\n", l2_index, l2_index);
    printf("  Page Number: %u (0x%X)\n", page_number, page_number);
    printf("  Page Offset: %u (0x%X)\n", page_offset, page_offset);
}

void generate_address_trace(uint32_t *addresses, int count, int locality) {
    srand(time(NULL));
    
    switch (locality) {
        case 0: // Random access
            for (int i = 0; i < count; i++) {
                addresses[i] = rand() % VIRTUAL_ADDRESS_SPACE_SIZE;
            }
            printf("Generated %d random addresses\n", count);
            break;
            
        case 1: // Sequential access
            {
                uint32_t base = rand() % (VIRTUAL_ADDRESS_SPACE_SIZE - count * 4);
                for (int i = 0; i < count; i++) {
                    addresses[i] = base + i * 4;
                }
                printf("Generated %d sequential addresses starting from 0x%08X\n", count, base);
            }
            break;
            
        case 2: // Locality of reference (80/20 rule)
            {
                uint32_t hot_region_start = rand() % (VIRTUAL_ADDRESS_SPACE_SIZE / 4);
                uint32_t hot_region_size = VIRTUAL_ADDRESS_SPACE_SIZE / 20; // 5% of address space
                
                for (int i = 0; i < count; i++) {
                    if (rand() % 100 < 80) { // 80% in hot region
                        addresses[i] = hot_region_start + (rand() % hot_region_size);
                    } else { // 20% random
                        addresses[i] = rand() % VIRTUAL_ADDRESS_SPACE_SIZE;
                    }
                }
                printf("Generated %d addresses with locality (hot region: 0x%08X-0x%08X)\n", 
                       count, hot_region_start, hot_region_start + hot_region_size);
            }
            break;
            
        default:
            generate_address_trace(addresses, count, 0);
            break;
    }
}

void run_simulation(MMU *mmu, uint32_t *addresses, int count, MemoryStats *stats) {
    memset(stats, 0, sizeof(MemoryStats));
    
    printf("Running simulation with %d memory accesses...\n", count);
    
    uint64_t start_cycles = mmu->total_cycles;
    uint64_t start_tlb_hits = mmu->tlb.hits;
    uint64_t start_page_faults = mmu->page_table.faults;
    
    for (int i = 0; i < count; i++) {
        uint32_t physical_addr = mmu_translate(mmu, addresses[i]);
        (void)physical_addr; // Suppress unused variable warning
        
        // Print progress every 10000 accesses
        if (i % 10000 == 0 && i > 0) {
            printf("  Processed %d accesses...\n", i);
        }
    }
    
    // Calculate statistics
    stats->total_accesses = count;
    stats->tlb_hits = mmu->tlb.hits - start_tlb_hits;
    stats->tlb_misses = stats->total_accesses - stats->tlb_hits;
    stats->page_faults = mmu->page_table.faults - start_page_faults;
    stats->page_hits = stats->total_accesses - stats->page_faults;
    stats->total_cycles = mmu->total_cycles - start_cycles;
    
    stats->tlb_hit_rate = (double)stats->tlb_hits / stats->total_accesses * 100;
    stats->page_hit_rate = (double)stats->page_hits / stats->total_accesses * 100;
    stats->avg_access_time = (double)stats->total_cycles / stats->total_accesses;
    
    printf("Simulation completed.\n");
}

void print_statistics(MemoryStats *stats, const char *test_name) {
    printf("\n=== %s Results ===\n", test_name);
    printf("Total Memory Accesses: %lu\n", stats->total_accesses);
    printf("TLB Hits: %lu (%.2f%%)\n", stats->tlb_hits, stats->tlb_hit_rate);
    printf("TLB Misses: %lu (%.2f%%)\n", stats->tlb_misses, 100.0 - stats->tlb_hit_rate);
    printf("Page Hits: %lu (%.2f%%)\n", stats->page_hits, stats->page_hit_rate);
    printf("Page Faults: %lu (%.2f%%)\n", stats->page_faults, 100.0 - stats->page_hit_rate);
    printf("Total Cycles: %lu\n", stats->total_cycles);
    printf("Average Access Time: %.2f cycles\n", stats->avg_access_time);
    printf("==============================\n");
}

void save_addresses_to_file(uint32_t *addresses, int count, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Failed to open file %s for writing\n", filename);
        return;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(file, "0x%08X\n", addresses[i]);
    }
    
    fclose(file);
    printf("Saved %d addresses to %s\n", count, filename);
}

void load_addresses_from_file(uint32_t *addresses, int *count, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file %s for reading\n", filename);
        *count = 0;
        return;
    }
    
    int i = 0;
    uint32_t addr;
    while (fscanf(file, "0x%X", &addr) == 1 && i < *count) {
        addresses[i] = addr;
        i++;
    }
    
    fclose(file);
    *count = i;
    printf("Loaded %d addresses from %s\n", i, filename);
}