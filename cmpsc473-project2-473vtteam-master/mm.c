/*
 * mm.c
 *
 * Name: Trisha Mandal, Varun Jani
 * Email: tvm5513@psu.edu, vxj5053@psu.edu
 *
 * NOTE TO STUDENTS: Wrote code to implement malloc, realloc and free
 * using segreted lists.
 * Used the textbook: http://guanzhou.pub/files/Computer%20System_EN.pdf
 * Used online resources posted by github user amz27
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*
 * If you want debugging output, uncomment the following. Be sure not
 * to have debugging enabled in your final submission
 */
//#define DEBUG

#ifdef DEBUG
/* When debugging is enabled, the underlying functions get called */
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#else
/* When debugging is disabled, no code gets generated */
#define dbg_printf(...)
#define dbg_assert(...)
#endif /* DEBUG */

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#define memset mem_memset
#define memcpy mem_memcpy
#endif /* DRIVER */

/* What is the correct alignment? */
#define ALIGNMENT 16 /* Double word size (bytes) */
#define WORDSIZE 8 /* Word and header/footer size (bytes) */
#define CHUNKSIZE 4096 /* Extend heap by this amount (bytes) */
#define LISTSIZE 12 /* List size of segregated list */

// All the function prototypes declared here
static void* extend_heap(size_t size);
static void* coalesce(void *ptr);
static void* place(void* ptr, size_t asize);
static void delete(void *ptr);
static void allocate(void *ptr, size_t size);
static int search(size_t asize);

/* rounds up to the nearest multiple of ALIGNMENT */
static size_t align(size_t x)
{
    return ALIGNMENT * ((x+ALIGNMENT-1)/ALIGNMENT);
}

/*
 * Helper functions
 */
// compute the larger value given two values
static size_t max(size_t x, size_t y)
{ 
  return ((x > y) ? (x) : (y));
}

//computes the smaller value out of the two values
static size_t min(size_t x, size_t y)
{ 
   return ((x < y) ? (x) : (y));
}

//writes a word in the given pointer address
static void put(void* p, size_t val)
{
  *((size_t *)(p)) = val;
}

// pack the size and the allocated bit into a word
static size_t pack(size_t size, int alloc)
{
   return ((size) | (alloc & 0x1));
}

// read a word at the given pointer address
static size_t get(void* p)
{
  return (*(size_t *)(p));
}

// the address of bp points to ptr
static void putp(void* p, void* ptr)
{ 
   *(size_t *)(p) = (size_t)(ptr);
}

// return the size of block given by pointer
static size_t get_size(void* p)
{
   return (size_t)(get(p) & ~(0x7));
}

// return the allocated bit of given pointer
static size_t get_alloc(void* p)
{
  return (size_t)(get(p) & 0x1);
}

// hdrp and ftrp computes the address of header and footer given by pointer respectively
static size_t *hdrp(void *bp)
{
   return ((size_t *)(bp) - 1);
}
static size_t *ftrp(void *bp)
{
   return ((size_t *)((bp) + get_size(hdrp(bp)) - ALIGNMENT));
}

// prevblkp and next_blkp computes the address of previous and next block given by pointer
static size_t *next_blkp(void* ptr)
{
  return (size_t *)((ptr) + get_size(hdrp(ptr)));
}

static size_t *prev_blkp(void* ptr)
{
   return (size_t *)((ptr) - get_size((ptr) - ALIGNMENT));
}

// prev_free_blkp and next_free_blkp computes the address of previous and next free block given by pointer
static size_t *prev_free_blkp(void* ptr)
{
   return ((size_t *)(ptr));
}

static size_t *next_free_blkp(void* ptr)
{
   return ((size_t *)((ptr) + WORDSIZE));
}

// prev_list_blkp and next_list_blkp computes the address of previous and next block in the list given pointer
static size_t *prev_list_blkp(void* ptr)
{
   return (*(size_t **)(ptr));
}
static size_t *next_list_blkp(void* ptr)
{
   return (*(size_t **)(next_free_blkp(ptr)));
}

// declare segregated list array and heap pointer
void* segregated_list[LISTSIZE];
void* heap_listp = NULL;


/*
 * Initialize: return false on error, true on success.
 */
// function that initialize the heap and the segregated list 
//Creates prologue and epilogue blocks
bool mm_init(void)
{        
        // initialising the segregated list values as NULL
	for(int i = 0; i < LISTSIZE; i++){
		segregated_list[i] = NULL;
	}
		
	// create the prologue and epilogue
	if((heap_listp = mem_sbrk(4*WORDSIZE)) == NULL){
		return false;
	}

	put(heap_listp, 0); /* Alignment padding */
	put(heap_listp + (1 * WORDSIZE), pack(ALIGNMENT,1)); /* Prologue header */
	put(heap_listp + (2 * WORDSIZE), pack(ALIGNMENT, 1)); /* Prologue footer */ 
        put(heap_listp + (3 * WORDSIZE), pack(0, 1)); /* Epilogue header */

	// create free blocks in the heap
	if(extend_heap(CHUNKSIZE) == NULL){
		return false;
	}

	return true;
}

/*
 * malloc: return a pointer to the payload of the allocated block
 * search the segregated list for free block and extend the heap if 
 * more blocks are required.
 */
void* malloc(size_t size)
{
	size_t alignedsize; /* Adjusted block size */
	size_t extendsize; /* Amount to extend heap if no fit */
        void* bp = NULL;	
        int index = 0;
	
	// check if size is valid
	if(size == 0){
	    return NULL;
	}
	
	// Modify block size to include header 
	if(size <= ALIGNMENT)
        {
	    alignedsize = 2 * ALIGNMENT;
	}
	// alignment request
	else
        {
	    alignedsize = ALIGNMENT * ((size + (ALIGNMENT) + (ALIGNMENT-1)) / ALIGNMENT);
	}

	// find the right size class index
	index = search(alignedsize);
	// decide extended size for heap

	extendsize = max(alignedsize, CHUNKSIZE);
        bp = segregated_list[index];

	// find fit space to place the block
	if(index != LISTSIZE && bp!= NULL){
            int i = 0;
	    while(i < (2*ALIGNMENT))
	    {
                // In case block pointer is null we don't need to iterate anymore
		if(bp == NULL)
		   break;
	        // Place the block correctly in case of the condition
	        if(alignedsize <= get_size(hdrp(bp)))
                {
	      		place(bp,alignedsize);
			return bp;
		}
		bp = next_list_blkp(bp);
		i++;
	    }

	}

	index++;
	bp = NULL;
	
	// if there is no space in the list, allocate more memory in heap
	while((index < LISTSIZE) && (bp == NULL))
        {
	    bp = segregated_list[index];
	    index++;
	}
	/* Get more memory and place the block */
	if(bp == NULL)
        {
	    bp = extend_heap(extendsize);
	}
        
	bp = place(bp,alignedsize);
	return bp;
}

/*
 * free: free the block and add to the list
 */
void free(void* ptr)
{
        // No point in freeing if the pointer is NULL
   	if(ptr == NULL)
	    return;

	size_t size = get(hdrp(ptr));
	// set header and footer of the block to unallocated
	put(hdrp(ptr),pack(size, 0));
	put(ftrp(ptr),pack(size, 0));

	// add to the list
	allocate(ptr, size);
	coalesce(ptr);
}

/*
 * realloc: return a new pointer that has the size and content of the old pointer
 * malloc space for the new pointer
 */
// Used pseudo code from the malloc lab pdf given to us
void* realloc(void* oldptr, size_t size)
{
	void* ptr;
        size_t csize = min(size, get_size(hdrp(oldptr)) - WORDSIZE);
  
        //if size of oldptr is 0 then free oldptr 
        if(size == 0)
        {
            free(oldptr);
	    return NULL;
	}

	// call equivalent to malloc(size)
	if(oldptr == NULL)
        {
	    return malloc(size);
	}
	// malloc space for pointer
	ptr = malloc(size);
	//copy the contents of old pointer
	memcpy(ptr,oldptr,csize);
        // Once copied, we can free the old pointer now
	free(oldptr);

	return ptr;
}

// function that combines adjacent free blocks into one larger free block
// and remove the appropriate free blocks from the segregated list
// Code snippet taken from the textbook pg 833
static void* coalesce(void *bp){
	// maintain information of previous and next block
	size_t palloc = get_alloc(hdrp(prev_blkp(bp)));
	size_t nalloc = get_alloc(hdrp(next_blkp(bp)));
	size_t size = get_size(hdrp(bp));
	
	// case where previous and next block allocated
	if(palloc && nalloc){ /* Case 1 */
		return bp;
	}
	// case where next block is free
	else if(palloc && !nalloc){ /* Case 2 */
		// get next block size
		size += get_size(hdrp(next_blkp(bp)));

		// delete current free block and next free block from the segregated list
		delete(bp);
		delete(next_blkp(bp));

		// update block 
		put(hdrp(bp),pack(size, 0));
		put(ftrp(bp),pack(size, 0));
	}
        // case where previous block is free
        else if(!palloc && nalloc){ /* Case 3 */
                // get previous block size
                size += get_size(hdrp(prev_blkp(bp)));

                // delete current free block and previous free block from the segregated list
                delete(bp);
                delete(prev_blkp(bp));

                // update block
                put(ftrp(bp),pack(size,0));
                put(hdrp(prev_blkp(bp)),pack(size,0));
                bp = prev_blkp(bp);
        }
	// case where both of previous and next blocks are free
	else{ /* Case 4 */
		// get size of blocks
		size += get_size(ftrp(next_blkp(bp))) + get_size(hdrp(prev_blkp(bp)));

		// delete current, previous and next block from the seg list
		delete(bp);
		delete(next_blkp(bp));
		delete(prev_blkp(bp));

		// update block
		put(ftrp(next_blkp(bp)),pack(size,0));
		put(hdrp(prev_blkp(bp)),pack(size,0));
		bp = prev_blkp(bp);
	}

	// add new free block to segregated list
	allocate(bp,size);
	return bp;
}

//code snippet from textbook pg 831
//extends heap when required
static void* extend_heap(size_t words){
	
        //maintains alignment 
        size_t alignedsize = align(words);
	size_t *bp;
	
	if((long)(bp = mem_sbrk(alignedsize)) == -1){
		return NULL;
	}
	
	// add to segregated list
	allocate(bp,alignedsize);
	
	/* Initialize free block header/footer and the epilogue header */
	put(hdrp(bp), pack(alignedsize, 0)); /* Free block header */
	put(ftrp(bp), pack(alignedsize, 0)); /* Free block footer */
	put(hdrp(next_blkp(bp)), pack(0,1)); /* New epilogue header */
	return coalesce(bp); /* Coalesce if the previous block was free */
}

/* function that places the required block into free block
  and computes the remaining size of the free block, if it is less 
  or equal to the minimum free block size(32 bytes), then allocate free block
  and add to the list. If the remaining size of the block is greater
  than minimum block size then split the block.
  code snippet from texbook page 857
*/
static void* place(void* bp, size_t asize){
	// delete the free block from seg list
	delete(bp);

	size_t csize = get_size(hdrp(bp));

	// compare the remaining block size to the minimum free block size
	// if large than or equal to the minimum free block size(32 bytes), then split the block
	if((csize - asize) >= (4*WORDSIZE)){ 
		put(hdrp(bp), pack(asize,1));
		put(ftrp(bp), pack(asize,1));
		put(hdrp(next_blkp(bp)), pack(csize-asize,0));
		put(ftrp(next_blkp(bp)), pack(csize-asize,0));
		allocate(next_blkp(bp), csize-asize);
	}

	// if the remaining size is not larger than min block size, 
	// then assign free block to allocated
	else{
		put(hdrp(bp), pack(csize, 1));
		put(ftrp(bp), pack(csize, 1));
	}
	return bp;
}


// function that search the segregated list to find the appropriate size class
// and return the size class index of the segregated list
static int search(size_t size){

        // Logic is explained in the video
        size_t remainder = size/32;
        if(remainder == 0) return 0;
        else if(remainder == 1) return 1;
	else if(remainder == 2) return 2;
	else if(remainder == 3) return 3;
	else if(remainder == 4) return 4;
	else if(remainder == 5) return 5;
	else if(remainder == 6) return 6;
	else if(remainder == 7) return 7;
	else if(remainder == 8) return 8;
	else if(remainder == 9) return 9;
        else if(remainder == 10) return 10;
        else return 11;
	        
}

// function that remove the block from the segregated list
static void delete(void* bp){
	
	// use the size of the block and search for segregated list size class index
	int index = search(get_size(hdrp(bp))); 
        
 	if(prev_list_blkp(bp)){
                // update previous free block and next free pointer
                if(next_list_blkp(bp) != NULL){
                        putp(prev_free_blkp(next_list_blkp(bp)), prev_list_blkp(bp));
                        putp(next_free_blkp(prev_list_blkp(bp)), next_list_blkp(bp));
                }
                else{
                        putp(next_free_blkp(prev_list_blkp(bp)), NULL);
                }
        }
	if(!prev_list_blkp(bp)){
		// update previous free block pointer 
		if(next_list_blkp(bp) != NULL)
                {
			putp(prev_free_blkp(next_list_blkp(bp)), NULL);
			segregated_list[index] = next_list_blkp(bp);
		}
		else
	        {
			segregated_list[index] = NULL;
		}
	}
}

// function that insert the free block into segregated list
static void allocate(void* bp, size_t size){
    
    // use the size of the block and search for segregated list size class index
    int index = search(size);

    //set head of the size class list
    void *head;
    head = segregated_list[index]; 

    // case that there are  blocks in the size list
    if (head != NULL) { 
	// set bp to list head and update the previous and next block info
	segregated_list[index] = bp;
        putp(prev_free_blkp(bp), NULL);
        putp(next_free_blkp(bp), head);
        putp(prev_free_blkp(head), bp);
        
    }
    // case that there are no blocks in the size list
    if (head == NULL){ 
	// set bp to the list head and set previous and next free block to NULL
	segregated_list[index] = bp;
	putp(prev_free_blkp(bp), NULL);
        putp(next_free_blkp(bp), NULL);  
    }
}

/*
 * calloc
 * This function is not tested by mdriver, and has been implemented for you.
 */
void* calloc(size_t nmemb, size_t size)
{
    void* ret;
    size *= nmemb;
    ret = malloc(size);
    if (ret) {
        memset(ret, 0, size);
    }
    return ret;
}

/*
 * Return whether the pointer is in the heap.
 * May be useful for debugging.
 */
static bool in_heap(const void* p)
{
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}

/*
 * Return whether the pointer is aligned.
 * May be useful for debugging.
 */
static bool aligned(const void* p)
{
    size_t ip = (size_t) p;
    return align(ip) == ip;
}

/*
 * commented out for submission. 
 * checks both the heap and the free list
 */

bool mm_checkheap(int lineno)
{   
    #ifdef DEBUG
    /* Write code to check heap invariants here */
    /* IMPLEMENT THIS */
#endif /* DEBUG */
    return true;
}




