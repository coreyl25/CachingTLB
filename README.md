# TLB and Multi-level Page Tables Simulator

This project implements a virtual memory management system simulator featuring Translation Lookaside Buffers (TLB) and multi-level page tables for an Operating Systems course.

## Features

### 1. Address Translation Components
- **Simple Direct-Mapped Page Table**: Basic page table implementation
- **Two-Level Page Table**: Hierarchical page table with L1/L2 structure
- **TLB (Translation Lookaside Buffer)**: 8-entry cache with round-robin replacement
- **MMU (Memory Management Unit)**: Integrated system combining TLB and page tables

### 2. Memory Access Patterns
- **Random Access**: Completely random memory references
- **Sequential Access**: Linear memory access pattern
- **Locality of Reference**: 80/20 pattern (80% of accesses in 5% of address space)

### 3. Performance Analysis
- TLB hit/miss rates
- Page hit/fault rates
- Average memory access times
- Cycle-accurate timing simulation

### 4. Configuration
- 4GB virtual address space (32-bit)
- 4KB page size
- 256 physical frames
- 8-entry TLB (configurable)
- Two-level page table (10-bit L1, 10-bit L2, 12-bit offset)

## Compilation Instructions

# Use Makefile target
make install-deps
```

### Makefile Compilation
```bash
make


## Testing Instructions

### 1. Run Complete Simulation
```bash
make run
```
This will execute all test cases including:
- Address translation verification
- Simple page table testing
- Two-level page table testing
- TLB functionality testing
- Comprehensive performance analysis with different access patterns

### 2. Quick Testake test
Runs a shorter version of the simulation and saves results to test_results.txt.

### 3. Performance Testing
```bash
make perf-test
```
Tests different TLB sizes (4, 16 entries) and compares performance.
