# User-level Paging in C

## Description

This project focuses on building a user-level page table that translates virtual addresses to physical addresses using a multi-level page table and implements a translation lookaside buffer (TLB) cache to reduce translation cost. The project emulates memory mechanisms and policies using C, providing a detailed simulation of virtual memory management. The implementation is tested across different page sizes to ensure the correctness of the page table, allocations, and TLB state.

## Features
- **User-level page table:** Translates virtual addresses to physical addresses.
- **Multi-level page table:** Efficiently handles address translation.
- **Translation Lookaside Buffer (TLB):** Reduces translation cost.
- **Support for multiple page sizes:** Tests the implementation across various page sizes.

## Installation

Clone the repository and navigate to the project directory:

```sh
git clone https://github.com/jawahar37/user-level-paging-C
cd user-level-paging-C
```
Compile the project using the provided Makefile:
```sh
make
```

### Usage
Part 1: Implementation of Virtual Memory System
- set_physical_mem(): Allocates memory buffer using mmap or malloc to create an illusion of physical memory.
- translate(): Takes the address of the outer page directory and a virtual address, returning the corresponding physical address.
- page_map(): Checks if there is an existing mapping for a virtual address and adds a new entry if necessary.
- t_malloc(): Takes the bytes to allocate and returns a virtual address. Manages a 32-bit address space supporting a 4GB address space. Allocates one or more pages per call and handles internal fragmentation.
Part 2: Implementation of a TLB
- add_TLB(): Adds a new virtual to physical page translation in the TLB.
- check_TLB(): Checks the presence of a translation in the TLB.
- print_TLB_missrate(): Reports the TLB miss rate.
Additional Functions:
t_free(): Frees pages starting from the page representing the virtual address.
put_value(): Copies values to physical pages using a virtual address.
get_value(): Reads data from a virtual address to a value buffer.

## Example Commands
Compile the benchmark tests using the provided Makefile:

```sh
make -C benchmark
```

Run the benchmark tests:
```sh
./benchmark/multi_test
```
