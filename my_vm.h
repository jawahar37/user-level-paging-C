#ifndef MY_VM_H_INCLUDED
#define MY_VM_H_INCLUDED
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

//Assume the address space is 32 bits, so the max memory size is 4GB
//Page size is 4KB

//Add any important includes here which you may need

#define PGSIZE 4096

// Maximum size of virtual memory
#define MAX_MEMSIZE 4ULL*1024*1024*1024

// Size of "physcial memory"
#define MEMSIZE 1024*1024*1024

// Represents a page table entry
typedef unsigned long pte_t;

// Represents a page directory entry
typedef unsigned long pde_t;

#define TLB_ENTRIES 512

//Structure to represents TLB
struct tlb {
    /*Assume your TLB is a direct mapped TLB with number of entries as TLB_ENTRIES
    * Think about the size of each TLB entry that performs virtual to physical
    * address translation.
    */
   int a;

};
// struct tlb tlb_store;


void set_physical_mem();
pte_t* translate(pde_t *pgdir, void *va);
int page_map(pde_t *pgdir, void *va, void* pa);
bool check_in_tlb(void *va);
void put_in_tlb(void *va, void *pa);
void *t_malloc(unsigned int num_bytes);
void t_free(void *va, int size);
int put_value(void *va, void *val, int size);
void get_value(void *va, void *val, int size);
void mat_mult(void *mat1, void *mat2, int size, void *answer);
void print_TLB_missrate();

static unsigned int get_top_bits(unsigned int value,  int num_bits);
static void set_bit_at_index(char *bitmap, int index);
static void clear_bit_at_index(char *bitmap, int index);
static int get_bit_at_index(char *bitmap, int index);
void printBinary(char c);
void *get_next_avail_physical();

void text_color(int fg);
void text_color_bg(int fg, int bg);
void print_bitmap(char* name, char* bitmap, int num_bytes);
void printBinary(char c);
void print_page_table_entries(unsigned long* page_table_base, int entries_per_row, int num_rows, unsigned long display_offset);

#endif
