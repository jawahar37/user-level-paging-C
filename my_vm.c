#include "my_vm.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Display code
enum debug_levels {PUBLISH, LOG, DEBUG};
int debug_level = DEBUG;
//

char* physical_memory;
pde_t* page_dir;
char* physical_bitmap;
char* virtual_bitmap;
int physical_bitmap_size, virtual_bitmap_size;
int offset_bits, page_table_bits, page_dir_bits;

int initial_call = 0;


/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating
    physical_memory = (char *) malloc(MEMSIZE);
    page_dir = (pde_t *) physical_memory;
    offset_bits = log2(PGSIZE);
    page_table_bits = offset_bits-2;
    page_dir_bits = 32 - page_table_bits - offset_bits;

    physical_bitmap_size = MEMSIZE/(PGSIZE*8) ;
    physical_bitmap = (char *)malloc( physical_bitmap_size );
    memset(physical_bitmap, 0, physical_bitmap_size );

    virtual_bitmap_size = MAX_MEMSIZE/(PGSIZE*8);
    virtual_bitmap = (char *)malloc( virtual_bitmap_size );
    memset(virtual_bitmap, 0, virtual_bitmap_size);
    
    // Setting the 0th Page Frame as for Page Directory
    set_bit_at_index(physical_bitmap, 0);
    
    // Print Statements
    // set_bit_at_index(virtual_bitmap, 1);
    // set_bit_at_index(virtual_bitmap, 2);
    // set_bit_at_index(virtual_bitmap, 3);
    // set_bit_at_index(virtual_bitmap, 4);
    // set_bit_at_index(virtual_bitmap, 5);
    // set_bit_at_index(virtual_bitmap, 6);
    // set_bit_at_index(virtual_bitmap, 7);

    // puts("virtual_bitmap[0]");
    // printBinary(physical_bitmap[0]); // 00000001
    // puts("virtual_bitmap[1]");
    // printBinary(physical_bitmap[1]);

    puts("set_physical_mem");
    print_bitmap("Virtual bitmap  ", virtual_bitmap,  2);
    print_bitmap("Physical bitmap ", physical_bitmap, 2);

    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them

}


/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
int
add_TLB(void *va, void *pa)
{

    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */

    return -1;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(void *va) {

    /* Part 2: TLB lookup code here */



   /*This function should return a pte_t pointer*/
}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = 0;	

    /*Part 2 Code here to calculate and print the TLB miss rate*/




    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}



/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t *translate(pde_t *pgdir, void *va) {
    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */


    //If translation not successful, then return NULL
    return NULL; 
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
page_map(pde_t *pgdir, void *va, void *pa)
{

    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */

    pde_t higher_bitmask = ((1<<page_dir_bits)-1)<<(32-page_dir_bits);
    pde_t page_dir_index = ((pde_t)va & higher_bitmask)>>offset_bits;

    pde_t page_table_pa_frame;
    if(pgdir[page_dir_index] == 0)
    {
        page_table_pa_frame = (pde_t)get_next_avail_physical();
        pgdir[page_dir_index] = page_table_pa_frame;
    }
    else
    {
        page_table_pa_frame = pgdir[page_dir_index];
    }
    pde_t* page_table_pa = pgdir + (page_table_pa_frame<<offset_bits);

    pte_t middle_bitmask = ((1<<page_table_bits)-1)<<(offset_bits);
    pte_t page_table_index = ((pte_t)va & middle_bitmask)>>offset_bits;

    page_table_pa[page_table_index] = (pte_t)pa;

    return -1;
}


/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {
    int counter = 0;
    pde_t first_index = 0;
    //Starting Virtual Address from 1
    for(int i=1;i<virtual_bitmap_size;i++)
    {
        // Checking for start of contiguous memory allocation       
        if(counter == 0){
            first_index = i;
        }

        if(get_bit_at_index(virtual_bitmap , i)==0){
            counter++;
        }
        else{
            counter=0;
        }
        
        if(counter==num_pages)
        {
            for (int j = first_index; j < num_pages+first_index; j++)
            {
                set_bit_at_index(virtual_bitmap, j);
            }
            return (void *)(first_index<<offset_bits); 
        }
    }
    return NULL;
    //Use virtual address bitmap to find the next free page
}


/* Function responsible for allocating pages
and used by the benchmark
*/
void *t_malloc(unsigned int num_bytes) {

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */

    if(initial_call == 0)
   {
    initial_call = 1;
    puts("Initial Call");
    set_physical_mem();
   }
    // puts("Subsequent Calls");
    // puts("Freeing Memory...");
    // free(page_dir);
    // puts("Freeing Physical Bitmap...");
    // free(physical_bitmap);
    // puts("Freeing Virtual Bitmap...");
    // free(virtual_bitmap);
    
    int num_pages = num_bytes/PGSIZE ;
    if(num_bytes%PGSIZE > 0) num_pages++;
    
    printf("Num_pages : %d\n",num_pages);
    void* va = get_next_avail(num_pages);
    

    for (pde_t i = 0; i < num_pages; i++)
    {
        void* pa = get_next_avail_physical();
        puts("Before page_map");
        print_bitmap("Virtual bitmap  ", virtual_bitmap,  2);
        print_bitmap("Physical bitmap ", physical_bitmap, 2);
        puts("");
        if(pa == NULL)
        {
            puts("No Physical Memory Available !");
            break;
        }
        page_map(page_dir,va+(i<<offset_bits),pa);
        puts("After page_map");
        print_bitmap("Virtual bitmap  ", virtual_bitmap,  2);
        print_bitmap("Physical bitmap ", physical_bitmap, 2);
        puts("");
    }

    puts("Page Directory :");
    print_page_table_entries(page_dir, 8, 8, 0);
    puts("");
    puts("Page Directory[0] Table :");
    print_page_table_entries(page_dir + (page_dir[0]<<offset_bits), 8, 8, 0);
    puts("------------------------------------ END OF MALLOC");
    

   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */
    return va;
    // return NULL;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void t_free(void *va, int size) {

    /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     *
     * Part 2: Also, remove the translation from the TLB
     */
    
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
 * The function returns 0 if the put is successfull and -1 otherwise.
*/
int put_value(void *va, void *val, int size) {

    /* HINT: Using the virtual address and translate(), find the physical page. Copy
     * the contents of "val" to a physical page. NOTE: The "size" value can be larger 
     * than one page. Therefore, you may have to find multiple pages using translate()
     * function.
     */


    /*return -1 if put_value failed and 0 if put is successfull*/

}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */


}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void mat_mult(void *mat1, void *mat2, int size, void *answer) {

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
     * matrix accessed. Similar to the code in test.c, you will use get_value() to
     * load each element and perform multiplication. Take a look at test.c! In addition to 
     * getting the values from two matrices, you will perform multiplication and 
     * store the result to the "answer array"
     */
    int x, y, val_size = sizeof(int);
    int i, j, k;
    for (i = 0; i < size; i++) {
        for(j = 0; j < size; j++) {
            unsigned int a, b, c = 0;
            for (k = 0; k < size; k++) {
                int address_a = (unsigned int)mat1 + ((i * size * sizeof(int))) + (k * sizeof(int));
                int address_b = (unsigned int)mat2 + ((k * size * sizeof(int))) + (j * sizeof(int));
                get_value( (void *)address_a, &a, sizeof(int));
                get_value( (void *)address_b, &b, sizeof(int));
                // printf("Values at the index: %d, %d, %d, %d, %d\n", 
                //     a, b, size, (i * size + k), (k * size + j));
                c += (a * b);
            }
            int address_c = (unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int));
            // printf("This is the c: %d, address: %x!\n", c, address_c);
            put_value((void *)address_c, (void *)&c, sizeof(int));
        }
    }
}

void *get_next_avail_physical() {
    //Starting Virtual Address from 1
    for(int i=1;i<physical_bitmap_size;i++)
    {

        if(get_bit_at_index(physical_bitmap , i)==0){
            set_bit_at_index(physical_bitmap, i);
            return (void *)(i);
        }
    }
    return NULL;
    //Use virtual address bitmap to find the next free page
}

static void set_bit_at_index(char *bitmap, int index)
{
    unsigned int bitmask = 1 << (index % 8);
    bitmap[index / 8] |= bitmask;
}

static void clear_bit_at_index(char *bitmap, int index)
{
    unsigned int bitmask = 1 << (index % 8);
    bitmap[index / 8] &= ~bitmask;
}

static int get_bit_at_index(char *bitmap, int index)
{
    return (bitmap[index / 8] >> (index % 8)) & 1;
}

static unsigned int get_top_bits(unsigned int value,  int num_bits)
{
    return value >> (32-num_bits);
	
}

void printBinary(char c) {
    for (int i = 7; i >= 0; i--) {
        putchar((c & (1 << i)) ? '1' : '0');
    }
    putchar('\n');
}

// Display implementations

#define BLACK 		0
#define RED 		1
#define YELLOW		3
#define BLUE		4
#define MAGENTA		5
#define	WHITE		7

#define CYAN		87
#define LIME        82
#define ORANGE      214

void text_color(int fg) {
	printf("%c[38;5;%dm", 0x1B, fg);
}
void text_color_bg(int fg, int bg) {
	printf("%c[38;5;%d;48;5;%dm", 0x1B, fg, bg);
}

void reset_color() {
    printf("\033[0m");
}


#define BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

void print_byte(char byte)
{
    printf(BINARY_PATTERN, BYTE_TO_BINARY(byte));
}


void print_bitmap(char* name, char* bitmap, int num_bytes) {
    printf("\t%s: ", name);

    text_color_bg(BLACK, YELLOW);
    for(int i = num_bytes-1; i >= 0; i--) {
        print_byte(bitmap[i]);
        if(i != 0)
            printf("  ");
    }
    reset_color();
    printf("\n");
}

void print_page_table_entries(unsigned long* page_table, int entries_per_row, int num_rows, unsigned long entry_offset) {
    unsigned long entry = entry_offset;

    for (int i = 0; i < num_rows; i++) {
        if(i%4 < 2)
            text_color(CYAN);
        else
            text_color(ORANGE);
        
        for (int j = 0; j < entries_per_row; j++) {
            printf("%lu  ", page_table[entry++]);
        }
        printf("\n");
    }

    reset_color();
}
