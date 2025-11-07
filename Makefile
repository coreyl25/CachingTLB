CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
TARGET = vm_simulator
SOURCES = main.c simple_page_table.c two_level_page_table.c tlb.c mmu.c utils.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = vm_memory.h

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile individual object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Run the simulation
run: $(TARGET)
	./$(TARGET)

# Run with memory checking (if valgrind is available)
memcheck: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET) addresses.txt

# Install dependencies (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y build-essential gcc valgrind

# Create a test run with smaller dataset for quick testing
test: $(TARGET)
	@echo "Running quick test..."
	@echo "Original TLB_SIZE = 8" > test_results.txt
	./$(TARGET) | tee -a test_results.txt

# Performance test with different configurations
perf-test: clean
	@echo "Performance testing with different TLB sizes..."
	@echo "Testing TLB_SIZE = 4"
	sed -i 's/#define TLB_SIZE.*/#define TLB_SIZE 4/' vm_memory.h
	make clean && make
	./$(TARGET) > results_tlb_4.txt
	@echo "Testing TLB_SIZE = 16"
	sed -i 's/#define TLB_SIZE.*/#define TLB_SIZE 16/' vm_memory.h
	make clean && make
	./$(TARGET) > results_tlb_16.txt
	@echo "Restoring original TLB_SIZE = 8"
	sed -i 's/#define TLB_SIZE.*/#define TLB_SIZE 8/' vm_memory.h
	@echo "Results saved in results_tlb_*.txt files"

# Help target
help:
	@echo "Available targets:"
	@echo "  all          - Build the simulator"
	@echo "  run          - Build and run the simulator"
	@echo "  test         - Quick test run"
	@echo "  memcheck     - Run with valgrind memory checking"
	@echo "  perf-test    - Performance test with different TLB sizes"
	@echo "  clean        - Clean build artifacts"
	@echo "  install-deps - Install build dependencies"
	@echo "  help         - Show this help message"

.PHONY: all run test memcheck clean install-deps perf-test help