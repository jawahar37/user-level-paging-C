#include "my_vm.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

// Display code
enum debug_levels {PUBLISH, LOG, DEBUG};
int debug_level = DEBUG;
//

void* physical_memory;
pde_t* page_dir;
char* physical_bitmap;
char* virtual_bitmap;
int physical_bitmap_size, virtual_bitmap_size;
int offset_bits, page_table_bits, page_dir_bits, tlb_hash_bits;

double tlb_misses;
double tlb_lookups;

int initial_call = 0;

tlb_entry *TLB;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {

    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating
    physical_memory = (void *) malloc(MEMSIZE);
    page_dir = (pde_t *) physical_memory;
    offset_bits = log2(PGSIZE);
    page_table_bits = offset_bits-2;
    page_dir_bits = 32 - page_table_bits - offset_bits;

    tlb_hash_bits = log2(TLB_ENTRIES);

    physical_bitmap_size = MEMSIZE/(PGSIZE*8) ;
    physical_bitmap = (char *)malloc( physical_bitmap_size );
    memset(physical_bitmap, 0, physical_bitmap_size );

    virtual_bitmap_size = MAX_MEMSIZE/(PGSIZE*8);
    virtual_bitmap = (char *)malloc( virtual_bitmap_size );
    memset(virtual_bitmap, 0, virtual_bitmap_size);
    
    // Setting the 0th Page Frame as for Page Directory
    set_bit_at_index(physical_bitmap, 0);

    // init TLB
    TLB = (tlb_entry*) malloc(TLB_ENTRIES * sizeof(tlb_entry));
    memset(TLB, 0, TLB_ENTRIES * sizeof(tlb_entry));
    tlb_misses = 0;
    tlb_lookups = 0;
    
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them

}


/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
void
add_TLB(void *va, pte_t pa)
{

    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */
    // find slot to place TLB entry.
    // empty the slot if an entry exists already
    
    pde_t hash_bitmask = ((1<<tlb_hash_bits)-1)<<offset_bits;
    pte_t hash_value = ((pte_t)(va) & hash_bitmask)>>offset_bits;
    hash_value = hash_value % TLB_ENTRIES;

    pte_t virtual_value = ((pte_t)va)>>(offset_bits);

    TLB[hash_value].virtual_tag = virtual_value;
    TLB[hash_value].physical_frame = pa;
    tlb_lookups++;
    tlb_misses++;
        // puts("\n------------------------------------ END OF ADD TLB");
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t 
check_TLB(void *va) {
    /*This function should return a pte_t pointer*/
    /* Part 2: TLB lookup code here */

    tlb_lookups++;
    pte_t hash_bitmask = ((1<<tlb_hash_bits)-1)<<offset_bits;
    pte_t hash_value = ((pte_t)(va) & hash_bitmask)>>offset_bits;
    hash_value = hash_value % TLB_ENTRIES;

    pte_t virtual_value = ((pte_t)va)>>(offset_bits);
    
    if (TLB[hash_value].virtual_tag == virtual_value) {
        return TLB[hash_value].physical_frame;
    }
    else {
        tlb_misses++;
        return 0;
    }
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

    miss_rate = tlb_misses/tlb_lookups;

    printf("\nTLB MISSES  : %lf",tlb_misses);
    printf("\nTLB LOOKUPS : %lf\n",tlb_lookups);


    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}

void 
remove_TLB(void *va) {
    /*This function should return a pte_t pointer*/
    /* Part 2: TLB lookup code here */

    pte_t hash_bitmask = ((1<<tlb_hash_bits)-1)<<offset_bits;
    pte_t hash_value = ((pte_t)(va) & hash_bitmask)>>offset_bits;
    hash_value = hash_value % TLB_ENTRIES;

    pte_t virtual_value = ((pte_t)va)>>(offset_bits);

    if (TLB[hash_value].virtual_tag == virtual_value) {
        TLB[hash_value].virtual_tag = 0;
        TLB[hash_value].physical_frame = 0;
    }
    else {
        return;
    }
    // puts("\n------------------------------------ END OF REMOVE TLB");
}



/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t translate(pde_t *pgdir, void *va) {
    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */
    pte_t cached_translation = check_TLB(va);
    if(cached_translation != 0)
    {
        return cached_translation;
    }

    pde_t virtual_address_index = ((pde_t)va)>>offset_bits;
    if(get_bit_at_index(virtual_bitmap , virtual_address_index)==0){
        //If translation not successful, then return NULL
        printf("Invalid Virtual Address");
        return 0;
    }

    pde_t higher_bitmask = ((1<<page_dir_bits)-1)<<(32-page_dir_bits);
    pde_t page_dir_index = ((pde_t)va & higher_bitmask)>>(32-page_dir_bits);
    
    
    pte_t page_table_pa_frame = pgdir[page_dir_index];


    pde_t* page_table_pa = pgdir + (page_table_pa_frame<<offset_bits);

    pte_t middle_bitmask = ((1<<page_table_bits)-1)<<(offset_bits);
    pte_t page_table_index = ((pte_t)va & middle_bitmask)>>offset_bits;

    // TLB TODO
    // Adding to TLB
    add_TLB(va, page_table_pa[page_table_index]);

    // Returns Physical Frame
    return (page_table_pa[page_table_index]);
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
    pde_t page_dir_index = ((pde_t)va & higher_bitmask)>>(32-page_dir_bits);

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

    add_TLB(va, page_table_pa[page_table_index]);

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
    pthread_mutex_lock(&lock);
    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */
    
    if(initial_call == 0)
    {
        printf("\t- Initialize all.\n");
        initial_call = 1;
        set_physical_mem();
    }
    
    int num_pages = num_bytes/PGSIZE ;
    if(num_bytes%PGSIZE > 0) num_pages++;
    
    void* va = get_next_avail(num_pages);
    

    for (pde_t i = 0; i < num_pages; i++)
    {
        void* pa = get_next_avail_physical();
        if(pa == NULL)
        {
            puts("No Physical Memory Available !");
            break;
        }
        page_map(page_dir,va+(i<<offset_bits),pa);
    }

   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */
    pthread_mutex_unlock(&lock);
    return va;
    // return NULL;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void t_free(void *va, int size) {
    pthread_mutex_lock(&lock);
    /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     *
     * Part 2: Also, remove the translation from the TLB
     */
    pde_t virtual_address_index = ((pde_t)va)>>offset_bits;
    if(get_bit_at_index(virtual_bitmap , virtual_address_index)==0){
        //If translation not successful, then return NULL
        printf("Invalid Virtual Address");
        pthread_mutex_unlock(&lock);
        return ;
    }

    int num_pages = size/PGSIZE ;
    if(size%PGSIZE > 0) num_pages++;

    for(int i=0;i<num_pages;i++){
        pde_t virtual_address_index = ((pde_t)(va+(i<<offset_bits)))>>offset_bits;
        if(get_bit_at_index(virtual_bitmap , virtual_address_index)==0){
            //If translation not successful, then return NULL
            printf("Invalid Virtual Address");
            pthread_mutex_unlock(&lock);
            return ;
        }
    }


    for(int i=0;i<num_pages;i++){
        pte_t physical_frame = translate(page_dir,va+(i<<offset_bits));
        pde_t virtual_frame = ((pde_t)(va+(i<<offset_bits)))>>offset_bits;


        clear_bit_at_index(physical_bitmap, physical_frame);
        clear_bit_at_index(virtual_bitmap, virtual_frame);
        memset(page_dir + (physical_frame<<offset_bits), 0, PGSIZE);

        remove_TLB(va+(i<<offset_bits));
    }

    pthread_mutex_unlock(&lock);
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
    pthread_mutex_lock(&lock);
    pte_t address;

    pte_t remaining_size = size;
    pte_t va_slider = 0;
    while(remaining_size>0){
        pde_t offset_bitmask = ((1<<offset_bits)-1);
        pde_t offset_value = ((pte_t)(va+va_slider) & offset_bitmask);
        pde_t remaining_page = PGSIZE - offset_value;

        pde_t virtual_address_index = ((pde_t)(va+va_slider))>>offset_bits;
        if(get_bit_at_index(virtual_bitmap , virtual_address_index)==0){
            //If translation not successful, then return NULL
            printf("Invalid Virtual Address");
            pthread_mutex_unlock(&lock);
            return -1;
        }
        if(remaining_size <= remaining_page){
            remaining_size = 0;
            va_slider += remaining_size; 
        }
        else{
            remaining_size -= remaining_page;
            va_slider += remaining_page;
        }
    }

    remaining_size = size;
    va_slider = 0;
    while(remaining_size>0){
        pde_t offset_bitmask = ((1<<offset_bits)-1);
        pde_t offset_value = ((pte_t)(va+va_slider) & offset_bitmask);
        pde_t remaining_page = PGSIZE - offset_value;
        address = translate(page_dir, va+va_slider);

        if(remaining_size <= remaining_page){
            memcpy((page_dir+(address<<offset_bits) + offset_value ), (val + (size - remaining_size)), remaining_size);
            remaining_size = 0;
            va_slider += remaining_size; 
        }
        else{
            memcpy((page_dir+(address<<offset_bits) + offset_value ), (val + (size - remaining_size)), remaining_page);
            remaining_size -= remaining_page;
            va_slider += remaining_page;
        }
    }

    /*return -1 if put_value failed and 0 if put is successfull*/
    pthread_mutex_unlock(&lock);
    return 0;
}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */
    pthread_mutex_lock(&lock);
    pte_t address;

    pte_t remaining_size = size;
    pte_t va_slider = 0;
    while(remaining_size>0){
        pde_t offset_bitmask = ((1<<offset_bits)-1);
        pde_t offset_value = ((pte_t)(va+va_slider) & offset_bitmask);
        pde_t remaining_page = PGSIZE - offset_value;
        
        pde_t virtual_address_index = ((pde_t)(va+va_slider))>>offset_bits;
        if(get_bit_at_index(virtual_bitmap , virtual_address_index)==0){
            //If translation not successful, then return NULL
            printf("Invalid Virtual Address");
            pthread_mutex_unlock(&lock);
            return ;
        }
        if(remaining_size <= remaining_page){
            remaining_size = 0;
            va_slider += remaining_size; 
        }
        else{
            remaining_size -= remaining_page;
            va_slider += remaining_page;
        }
    }

    remaining_size = size;
    va_slider = 0;
    while(remaining_size>0){
        pde_t offset_bitmask = ((1<<offset_bits)-1);
        pde_t offset_value = ((pte_t)(va+va_slider) & offset_bitmask);
        pde_t remaining_page = PGSIZE - offset_value;
        address = translate(page_dir, va+va_slider);

        if(remaining_size <= remaining_page){
            memcpy((val + (size - remaining_size)), (page_dir+(address<<offset_bits) + offset_value ), remaining_size);
            remaining_size = 0;
            va_slider += remaining_size; 
        }
        else{
            memcpy((val + (size - remaining_size)), (page_dir+(address<<offset_bits) + offset_value ), remaining_page);
            remaining_size -= remaining_page;
            va_slider += remaining_page;
        }
    }

    /*return -1 if put_value failed and 0 if put is successfull*/
    pthread_mutex_unlock(&lock);
    return;
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
                c += (a * b);
            }
            int address_c = (unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int));
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
    //Implement your code here  
    unsigned int bitmask = 1 << (index % 8);
    bitmap[index / 8] |= bitmask;
    return;
}

static void clear_bit_at_index(char *bitmap, int index)
{
    unsigned int bitmask = 1 << (index % 8);
    bitmap[index / 8] &= ~bitmask;
}

static int get_bit_at_index(char *bitmap, int index)
{
    //Get to the location in the character bitmap array
    //Implement your code here
    return (bitmap[index / 8] >> (index % 8)) & 1;
}

static unsigned int get_top_bits(unsigned int value,  int num_bits)
{
    //Implement your code here
    return value >> (32-num_bits);
    
}