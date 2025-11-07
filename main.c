#include "vm_memory.h"

void test_address_translation() {
    printf("\n=== Address Translation Test ===\n");
    
    uint32_t test_addresses[] = {
        0x00000000, 0x00001000, 0x12345678, 0x87654321, 0xFFFFFFFF
    };
    
    int num_addresses = sizeof(test_addresses) / sizeof(test_addresses[0]);
    
    for (int i = 0; i < num_addresses; i++) {
        printf("\nTest Address %d:\n", i + 1);
        print_address_breakdown(test_addresses[i]);
    }
}

void test_simple_page_table() {
    printf("\n=== Simple Page Table Test ===\n");
    
    SimplePageTable pt;
    init_simple_page_table(&pt);
    
    uint32_t test_addresses[] = {0x00001000, 0x00002000, 0x00001000, 0x00003000};
    int num_tests = sizeof(test_addresses) / sizeof(test_addresses[0]);
    
    for (int i = 0; i < num_tests; i++) {
        bool fault;
        uint32_t physical_addr = translate_simple_page_table(&pt, test_addresses[i], &fault);
        printf("Virtual: 0x%08X -> Physical: 0x%08X %s\n", 
               test_addresses[i], physical_addr, fault ? "(Page Fault)" : "(Hit)");
    }
    
    printf("\nSimple Page Table Stats:\n");
    printf("Accesses: %lu, Hits: %lu, Faults: %lu\n", pt.accesses, pt.hits, pt.faults);
    printf("Hit Rate: %.2f%%\n", (double)pt.hits / pt.accesses * 100);
    
    cleanup_simple_page_table(&pt);
}

void test_two_level_page_table() {
    printf("\n=== Two-Level Page Table Test ===\n");
    
    TwoLevelPageTable pt;
    init_two_level_page_table(&pt);
    
    uint32_t test_addresses[] = {0x00001000, 0x40002000, 0x00001000, 0x80003000, 0x40002000};
    int num_tests = sizeof(test_addresses) / sizeof(test_addresses[0]);
    
    for (int i = 0; i < num_tests; i++) {
        bool fault;
        uint32_t physical_addr = translate_two_level_page_table(&pt, test_addresses[i], &fault);
        printf("Virtual: 0x%08X -> Physical: 0x%08X %s\n", 
               test_addresses[i], physical_addr, fault ? "(Page Fault)" : "(Hit)");
    }
    
    printf("\nTwo-Level Page Table Stats:\n");
    printf("Accesses: %lu, Hits: %lu, Faults: %lu\n", pt.accesses, pt.hits, pt.faults);
    printf("Hit Rate: %.2f%%\n", (double)pt.hits / pt.accesses * 100);
    
    cleanup_two_level_page_table(&pt);
}

void test_tlb() {
    printf("\n=== TLB Test ===\n");
    
    TLB tlb;
    init_tlb(&tlb);
    
    uint32_t virtual_pages[] = {0x123, 0x456, 0x789, 0x123, 0xABC, 0x456, 0xDEF, 0x123, 0x111};
    int num_tests = sizeof(virtual_pages) / sizeof(virtual_pages[0]);
    
    for (int i = 0; i < num_tests; i++) {
        uint32_t physical_frame;
        bool hit = tlb_lookup(&tlb, virtual_pages[i], &physical_frame);
        
        if (hit) {
            printf("TLB Hit: VP 0x%X -> PF 0x%X\n", virtual_pages[i], physical_frame);
        } else {
            printf("TLB Miss: VP 0x%X\n", virtual_pages[i]);
            // Simulate adding to TLB
            tlb_insert(&tlb, virtual_pages[i], 0x100 + virtual_pages[i] % 256);
        }
    }
    
    tlb_print_contents(&tlb);
    
    printf("\nTLB Stats:\n");
    printf("Accesses: %lu, Hits: %lu, Misses: %lu\n", tlb.accesses, tlb.hits, tlb.misses);
    printf("Hit Rate: %.2f%%\n", (double)tlb.hits / tlb.accesses * 100);
    
    cleanup_tlb(&tlb);
}

void test_mmu_performance() {
    printf("\n=== MMU Performance Test ===\n");
    
    const int num_accesses = 50000;
    uint32_t *addresses = (uint32_t *)malloc(num_accesses * sizeof(uint32_t));
    if (!addresses) {
        fprintf(stderr, "Failed to allocate memory for addresses\n");
        return;
    }
    
    MMU mmu;
    init_mmu(&mmu);
    
    MemoryStats stats_random, stats_sequential, stats_locality;
    
    // Test 1: Random Access
    printf("\n--- Test 1: Random Access Pattern ---\n");
    generate_address_trace(addresses, num_accesses, 0);
    run_simulation(&mmu, addresses, num_accesses, &stats_random);
    print_statistics(&stats_random, "Random Access");
    save_addresses_to_file(addresses, 1000, "addresses.txt"); // Save first 1000 for analysis
    
    // Reset MMU for next test
    cleanup_mmu(&mmu);
    init_mmu(&mmu);
    
    // Test 2: Sequential Access
    printf("\n--- Test 2: Sequential Access Pattern ---\n");
    generate_address_trace(addresses, num_accesses, 1);
    run_simulation(&mmu, addresses, num_accesses, &stats_sequential);
    print_statistics(&stats_sequential, "Sequential Access");
    
    // Reset MMU for next test
    cleanup_mmu(&mmu);
    init_mmu(&mmu);
    
    // Test 3: Locality of Reference
    printf("\n--- Test 3: Locality of Reference (80/20) ---\n");
    generate_address_trace(addresses, num_accesses, 2);
    run_simulation(&mmu, addresses, num_accesses, &stats_locality);
    print_statistics(&stats_locality, "Locality of Reference");
    
    // Performance comparison
    printf("\n=== Performance Comparison ===\n");
    printf("Access Pattern    | TLB Hit Rate | Page Hit Rate | Avg Access Time\n");
    printf("------------------|--------------|---------------|----------------\n");
    printf("Random            |    %6.2f%%    |     %6.2f%%    |     %8.2f\n", 
           stats_random.tlb_hit_rate, stats_random.page_hit_rate, stats_random.avg_access_time);
    printf("Sequential        |    %6.2f%%    |     %6.2f%%    |     %8.2f\n", 
           stats_sequential.tlb_hit_rate, stats_sequential.page_hit_rate, stats_sequential.avg_access_time);
    printf("Locality (80/20)  |    %6.2f%%    |     %6.2f%%    |     %8.2f\n", 
           stats_locality.tlb_hit_rate, stats_locality.page_hit_rate, stats_locality.avg_access_time);
    
    cleanup_mmu(&mmu);
    free(addresses);
}

void experiment_tlb_size_impact() {
    printf("\n=== TLB Size Impact Experiment ===\n");
    
    // Modify TLB size and test performance
    // This is a simplified version - in practice, you'd modify the TLB_SIZE constant
    printf("Note: TLB size is currently fixed at %d entries.\n", TLB_SIZE);
    printf("To test different TLB sizes, modify TLB_SIZE in vm_memory.h and recompile.\n");
    printf("Suggested sizes to test: 4, 8, 16, 32, 64\n");
    
    const int num_accesses = 10000;
    uint32_t *addresses = (uint32_t *)malloc(num_accesses * sizeof(uint32_t));
    if (!addresses) {
        fprintf(stderr, "Failed to allocate memory for addresses\n");
        return;
    }
    
    MMU mmu;
    init_mmu(&mmu);
    
    generate_address_trace(addresses, num_accesses, 2); // Use locality pattern
    
    MemoryStats stats;
    run_simulation(&mmu, addresses, num_accesses, &stats);
    
    printf("TLB Size: %d entries\n", TLB_SIZE);
    printf("TLB Hit Rate: %.2f%%\n", stats.tlb_hit_rate);
    printf("Average Access Time: %.2f cycles\n", stats.avg_access_time);
    
    cleanup_mmu(&mmu);
    free(addresses);
}

int main() {
    printf("=== Operating Systems Lab: TLB and Multi-level Page Tables ===\n");
    printf("Virtual Address Space: %llu bytes (%.2f GB)\n", 
           (unsigned long long)VIRTUAL_ADDRESS_SPACE_SIZE, (double)VIRTUAL_ADDRESS_SPACE_SIZE / (1024*1024*1024));
    printf("Page Size: %d bytes (%d KB)\n", PAGE_SIZE, PAGE_SIZE / 1024);
    printf("Number of Pages: %llu\n", (unsigned long long)NUM_PAGES);
    printf("Physical Frames: %d\n", NUM_PHYSICAL_FRAMES);
    printf("TLB Size: %d entries\n", TLB_SIZE);
    
    // Run all tests
    test_address_translation();
    test_simple_page_table();
    test_two_level_page_table();
    test_tlb();
    test_mmu_performance();
    experiment_tlb_size_impact();
    
    printf("\n=== All tests completed successfully ===\n");
    printf("Check the generated 'addresses.txt' file for sample address traces.\n");
    
    return 0;
}