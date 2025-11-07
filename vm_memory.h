#ifndef VM_MEMORY_H
#define VM_MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// Configuration constants
#define VIRTUAL_ADDRESS_SPACE_SIZE (1ULL << 32)  // 4GB virtual address space
#define PAGE_SIZE 4096                           // 4KB pages
#define NUM_PAGES (VIRTUAL_ADDRESS_SPACE_SIZE / PAGE_SIZE)
#define NUM_PHYSICAL_FRAMES 256                  // Limited physical memory
#define PAGE_OFFSET_BITS 12                      // log2(PAGE_SIZE)
#define PAGE_NUMBER_BITS 20                      // 32 - PAGE_OFFSET_BITS

// TLB Configuration
#define TLB_SIZE 8
#define TLB_HIT_TIME 1    // cycles
#define PAGE_TABLE_ACCESS_TIME 10  // cycles
#define PAGE_FAULT_TIME 1000       // cycles

// 2-Level Page Table Configuration
#define L1_BITS 10        // First level page table bits
#define L2_BITS 10        // Second level page table bits
#define L1_SIZE (1 << L1_BITS)
#define L2_SIZE (1 << L2_BITS)

// Masks and shifts
#define PAGE_OFFSET_MASK ((1 << PAGE_OFFSET_BITS) - 1)
#define PAGE_NUMBER_MASK ((1 << PAGE_NUMBER_BITS) - 1)
#define L1_INDEX_MASK ((1 << L1_BITS) - 1)
#define L2_INDEX_MASK ((1 << L2_BITS) - 1)

// Simple Page Table Entry
typedef struct {
    bool valid;
    uint32_t frame_number;
    bool referenced;
    bool dirty;
} PageTableEntry;

// TLB Entry
typedef struct {
    bool valid;
    uint32_t virtual_page;
    uint32_t physical_frame;
    bool referenced;
    bool dirty;
} TLBEntry;

// Simple Direct-Mapped Page Table
typedef struct {
    PageTableEntry *entries;
    uint32_t size;
    uint64_t accesses;
    uint64_t hits;
    uint64_t faults;
} SimplePageTable;

// Two-Level Page Table
typedef struct {
    PageTableEntry **l1_table;  // Array of pointers to L2 tables
    bool *l1_valid;              // Valid bits for L1 entries
    uint32_t l1_size;
    uint64_t accesses;
    uint64_t hits;
    uint64_t faults;
} TwoLevelPageTable;

// TLB Structure
typedef struct {
    TLBEntry *entries;
    uint32_t size;
    uint32_t next_replace;       // For round-robin replacement
    uint64_t accesses;
    uint64_t hits;
    uint64_t misses;
} TLB;

// Combined Memory Management Unit
typedef struct {
    TLB tlb;
    TwoLevelPageTable page_table;
    uint32_t *physical_memory;
    bool *frame_allocated;
    uint32_t next_free_frame;
    uint64_t total_cycles;
} MMU;

// Statistics structure
typedef struct {
    uint64_t total_accesses;
    uint64_t tlb_hits;
    uint64_t tlb_misses;
    uint64_t page_hits;
    uint64_t page_faults;
    uint64_t total_cycles;
    double tlb_hit_rate;
    double page_hit_rate;
    double avg_access_time;
} MemoryStats;

// Function declarations
void init_simple_page_table(SimplePageTable *pt);
void cleanup_simple_page_table(SimplePageTable *pt);
uint32_t translate_simple_page_table(SimplePageTable *pt, uint32_t virtual_addr, bool *fault);

void init_two_level_page_table(TwoLevelPageTable *pt);
void cleanup_two_level_page_table(TwoLevelPageTable *pt);
uint32_t translate_two_level_page_table(TwoLevelPageTable *pt, uint32_t virtual_addr, bool *fault);

void init_tlb(TLB *tlb);
void cleanup_tlb(TLB *tlb);
bool tlb_lookup(TLB *tlb, uint32_t virtual_page, uint32_t *physical_frame);
void tlb_insert(TLB *tlb, uint32_t virtual_page, uint32_t physical_frame);
void tlb_invalidate_all(TLB *tlb);
void tlb_print_contents(TLB *tlb);

void init_mmu(MMU *mmu);
void cleanup_mmu(MMU *mmu);
uint32_t mmu_translate(MMU *mmu, uint32_t virtual_addr);

void generate_address_trace(uint32_t *addresses, int count, int locality);
void run_simulation(MMU *mmu, uint32_t *addresses, int count, MemoryStats *stats);
void print_statistics(MemoryStats *stats, const char *test_name);

// Utility functions
uint32_t get_page_number(uint32_t virtual_addr);
uint32_t get_page_offset(uint32_t virtual_addr);
uint32_t get_l1_index(uint32_t virtual_addr);
uint32_t get_l2_index(uint32_t virtual_addr);
void print_address_breakdown(uint32_t virtual_addr);
void save_addresses_to_file(uint32_t *addresses, int count, const char *filename);
void load_addresses_from_file(uint32_t *addresses, int *count, const char *filename);

#endif // VM_MEMORY_H