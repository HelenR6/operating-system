/*
 * =====================================================================================
 *
 *	Filename:  		sma.c
 *
 *  Description:	Base code for Assignment 3 for ECSE-427 / COMP-310
 *
 *  Version:  		1.0
 *  Created:  		6/11/2020 9:30:00 AM
 *  Revised:  		-
 *  Compiler:  		gcc
 *
 *  Author:  		Mohammad Mushfiqur Rahman
 *      
 *  Instructions:   Please address all the "TODO"s in the code below and modify 
 * 					them accordingly. Feel free to modify the "PRIVATE" functions.
 * 					Don't modify the "PUBLIC" functions (except the TODO part), unless
 * 					you find a bug! Refer to the Assignment Handout for further info.
 * =====================================================================================
 */

/* Includes */
#include "sma.h" // Please add any libraries you plan to use inside this file
#include <stdio.h>      
#include <stdlib.h>  
#include <unistd.h>
#include <stdbool.h>
/* Definitions*/
#define MAX_TOP_FREE (128 * 1024) // Max top free block size = 128 Kbytes
//	TODO: Change the Header size if required
//#define FREE_BLOCK_HEADER_SIZE 2 * sizeof(char *) + sizeof(int) // Size of the Header in a free memory block
#define FREE_BLOCK_HEADER_SIZE sizeof(block)
//	TODO: Add constants here

typedef enum //	Policy type definition
{
	WORST,
	NEXT
} Policy;

char *sma_malloc_error;
void *freeListHead = NULL;			  //	The pointer to the HEAD of the doubly linked free memory list
void *freeListTail = NULL;			  //	The pointer to the TAIL of the doubly linked free memory list
unsigned long totalAllocatedSize = 0; //	Total Allocated memory in Bytes
unsigned long totalFreeSize = 0;	  //	Total Free memory in Bytes in the free memory list
Policy currentPolicy = WORST;		  //	Current Policy
//	TODO: Add any global variables here
int number_of_block = 0;
block *heap_start;
block *heap_end;
block * free_lst;
void *nextPtr = NULL;

/*
 * =====================================================================================
 *	Public Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: sma_malloc
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates a memory block of input size from the heap, and returns a 
 * 					pointer pointing to it. Returns NULL if failed and sets a global error.
 */

void *sma_malloc(int size)
{
	void *pMemory = NULL;

	// Checks if the free list is empty
	if (freeListHead == NULL)
	{
		// Allocate memory by increasing the Program Break
		pMemory = allocate_pBrk(size);
	}
	// If free list is not empty
	else
	{
		// Allocate memory from the free memory list
		pMemory = allocate_freeList(size);

		// If a valid memory could NOT be allocated from the free memory list
		if (pMemory == (void *)-2)
		{
			// Allocate memory by increasing the Program Break
			pMemory = allocate_pBrk(size);
		}
	}

	// Validates memory allocation
	if (pMemory < 0 || pMemory == NULL)
	{
		sma_malloc_error = "Error: Memory allocation failed!";
		return NULL;
	}

	// Updates SMA Info
	totalAllocatedSize += size;

	return pMemory;
}

/*
 *	Funcation Name: sma_free
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Deallocates the memory block pointed by the input pointer
 */
void sma_free(void *ptr)
{
	//	Checks if the ptr is NULL
	if (ptr == NULL)
	{
		puts("Error: Attempting to free NULL!");
	}
	//	Checks if the ptr is beyond Program Break
	else if (ptr > sbrk(0))
	{
		puts("Error: Attempting to free unallocated space!");
	}
	else
	{
		//	Adds the block to the free memory list
		add_block_freeList(ptr);
	}
}

/*
 *	Funcation Name: sma_mallopt
 *	Input type:		int
 * 	Output type:	void
 * 	Description:	Specifies the memory allocation policy
 */
void sma_mallopt(int policy)
{
	// Assigns the appropriate Policy
	if (policy == 1)
	{
		currentPolicy = WORST;
	}
	else if (policy == 2)
	{
		currentPolicy = NEXT;
	}
}


/*
 *	Funcation Name: sma_realloc
 *	Input type:		void*, int
 * 	Output type:	void*
 * 	Description:	Reallocates memory pointed to by the input pointer by resizing the
 * 					memory block according to the input size.
 */
void sma_mallinfo()
{
	//	Finds the largest Contiguous Free Space (should be the largest free block)
	int largestFreeBlock = get_largest_freeBlock();
	char str[60];

	//	Prints the SMA Stats
	sprintf(str, "Total number of bytes allocated: %lu", totalAllocatedSize);
	puts(str);
	sprintf(str, "Total free space: %lu", totalFreeSize);
	puts(str);
	sprintf(str, "Size of largest contigious free space (in bytes): %d", largestFreeBlock);
	puts(str);
}

/*
 *	Funcation Name: sma_realloc
 *	Input type:		void*, int
 * 	Output type:	void*
 * 	Description:	Reallocates memory pointed to by the input pointer by resizing the
 * 					memory block according to the input size.
 */
void *sma_realloc(void *ptr, int size)
{
	// TODO: 	Should be similar to sma_malloc, except you need to check if the pointer address
	//			had been previously allocated.
	// Hint:	Check if you need to expand or contract the memory. If new size is smaller, then
	//			chop off the current allocated memory and add to the free list. If new size is bigger
	//			then check if there is sufficient adjacent free space to expand, otherwise find a new block
	//			like sma_malloc.
	//			Should not accept a NULL pointer, and the size should be greater than 0.

	
	block* ob = ((block*) ptr)-1;
	if (size <= ob -> size){
		void* nptr = splitblock(ptr,size,ob->size-size);
		add_block_freeList(nptr);
		return ptr;
	}else{
		void* nptr = sma_malloc(size);
		strcpy((char*)nptr,(char*)ptr);
		sma_free(ptr);
		return nptr;
	}


}



/*
 * =====================================================================================
 *	Private Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: allocate_pBrk
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory by increasing the Program Break
 */
void *allocate_pBrk(int size)
{
	void *newBlock = NULL;
	int excessSize = 0;

	//	TODO: 	Allocate memory by incrementing the Program Break by calling sbrk() or brk()
	//	Hint:	Getting an exact "size" of memory might not be the best idea. Why?
	//			Also, if you are getting a larger memory, you need to put the excess in the free list

	//	Allocates the Memory Block
	newBlock = sbrk(size+FREE_BLOCK_HEADER_SIZE);
	allocate_block(getData(newBlock), size, excessSize, 0);
	return getData(newBlock);
}

/*
 *	Funcation Name: allocate_freeList
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory from the free memory list
 */
void *allocate_freeList(int size)
{
	void *pMemory = NULL;

	if (currentPolicy == WORST)
	{
		// Allocates memory using Worst Fit Policy
		pMemory = allocate_worst_fit(size);
	}
	else if (currentPolicy == NEXT)
	{
		// Allocates memory using Next Fit Policy
		pMemory = allocate_next_fit(size);
	}
	else
	{
		pMemory = NULL;
	}

	return pMemory;
}

/*
 *	Funcation Name: allocate_worst_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Worst Fit from the free memory list
 */
void *allocate_worst_fit(int size)
{
	//puts("jello");
	void *worstBlock = NULL;
	int excessSize;
	int blockFound = 0;

	//	TODO: 	Allocate memory by using Worst Fit Policy
	//	Hint:	Start off with the freeListHead and iterate through the entire list to 
	//			get the largest block
	// block* cur_block=(block*)freeListHead;
	// int largestSize=get_largest_freeBlock();
	// while(cur_block!=NULL&& blockFound==0){
	// 	if(cur_block->isfree==1  &&cur_block->size -size>excessSize ){
	// 		blockFound=1;
    //         excessSize=cur_block->size -size;
	// 		worstBlock=getData(cur_block);
	// 	}cur_block=cur_block->next;
	// }

	block* curr = (block*)freeListHead;
	while (curr && blockFound == 0){
		if ((curr->isfree) && (curr->size >= size)){
			int new_excessSize = curr->size - size;
			if (new_excessSize > excessSize){
				worstBlock = getData(curr);
				excessSize = new_excessSize;
				blockFound = 1;
			}
		}
		curr = curr -> next;
	}
	//	Checks if appropriate block is found.
	if (blockFound)
	{
		//	Allocates the Memory Block
		allocate_block(worstBlock, size, excessSize, 1);
	}
	else
	{
		//	Assigns invalid address if appropriate block not found in free list
		worstBlock = (void *)-2;
	}

	return worstBlock;
}

/*
 *	Funcation Name: allocate_next_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Next Fit from the free memory list
 */
void *allocate_next_fit(int size)
{
	void *nextBlock = NULL;
	int excessSize;
	int blockFound = 0;

	//	TODO: 	Allocate memory by using Next Fit Policy
	//	Hint:	You should use a global pointer to keep track of your last allocated memory address, and 
	//			allocate free blocks that come after that address (i.e. on top of it). Once you reach 
	//			Program Break, you start from the beginning of your heap, as in with the free block with
	//			the smallest address)
	//if it is the first time to use next_fit, then the global variable nextPtr should point to freeListHead
	(nextPtr)==NULL ? (nextPtr=freeListHead):(nextPtr=NULL) ;
	block* track = (block*)freeListHead;
	while (track!=NULL){
		if (track == (block*) nextPtr){
			break;
		}else{
		nextPtr = freeListHead;
		track = track -> next;	
		}
		
	}
	
	block* cur_block = (block*) nextPtr;
	bool ifBreak=0;
	while (!ifBreak&&!blockFound ){
		//check if the current block size big enough to hold the blocks with given size. 
		if (cur_block->size >= size){
			//update the value of nextBlock to the next block
			nextBlock = (void*)(((block*)cur_block)+1);
			//update the excessSize to be allocated
			excessSize = cur_block->size - size;
			nextPtr = cur_block;
			blockFound = 1;
		}else{		
			cur_block = cur_block->next;
			if (cur_block == NULL){ cur_block = (block*)freeListHead;}


		}
		if ((char*)cur_block == (char*)nextPtr) {ifBreak=1;};
	}

	//	Checks if appropriate found is found.
	if (blockFound)
	{
		//	Allocates the Memory Block
		allocate_block(nextBlock, size, excessSize, 1);
	}
	else
	{
		//	Assigns invalid address if appropriate block not found in free list
		nextBlock = (void *)-2;
	}

	return nextBlock;
}

/*
 *	Funcation Name: allocate_block
 *	Input type:		void*, int, int, int
 * 	Output type:	void
 * 	Description:	Performs routine operations for allocating a memory block
 */
void allocate_block(void *newBlock, int size, int excessSize, int fromFreeList)
{
	//puts("allocate_block");
	void *excessFreeBlock; //	pointer for any excess free block
	int addFreeBlock;

	// 	Checks if excess free size is big enough to be added to the free memory list
	//	Helps to reduce external fragmentation

	//	TODO: Adjust the condition based on your Head and Tail size (depends on your TAG system)
	//	Hint: Might want to have a minimum size greater than the Head/Tail sizes
	addFreeBlock = excessSize > (FREE_BLOCK_HEADER_SIZE );
	block* nb = getBlock(newBlock);
	nb -> size = size;
	//	If excess free size is big enough
	if (addFreeBlock)
	{
		//	TODO: Create a free block using the excess memory size, then assign it to the Excess Free Block
		excessFreeBlock = splitblock(newBlock, size, excessSize);
		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes new block and adds the excess free block to the free list
			replace_block_freeList(newBlock, excessFreeBlock);
		}
		else
		{
			//	Adds excess free block to the free list
			add_block_freeList(excessFreeBlock);
		}
	}
	//	Otherwise add the excess memory to the new block
	else
	{
		//	TODO: Add excessSize to size and assign it to the new Block
		nb -> size = size + excessSize;
		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes the new block from the free list
			remove_block_freeList(newBlock);
		}
	}
}

/*
 *	Funcation Name: replace_block_freeList
 *	Input type:		void*, void*
 * 	Output type:	void
 * 	Description:	Replaces old block with the new block in the free list
 */
void replace_block_freeList(void *oldBlock, void *newBlock)
{
	//	TODO: Replace the old block with the new block
	block* nb = getBlock(newBlock);
	block* ob = getBlock(oldBlock);
	nb -> prev = ob -> prev;
	nb -> next = ob -> next;
	if (ob -> prev)
		ob -> prev -> next = nb;
	if (ob -> next)
		ob -> next -> prev = nb;
	ob -> isfree = 0;
	nb -> isfree = 1;

	if (freeListHead == ob)
		freeListHead = nb;
	if (freeListTail == ob)
		freeListTail = nb;

	
	//	Updates SMA info
	totalAllocatedSize += (get_blockSize(oldBlock) - get_blockSize(newBlock));
	totalFreeSize += (get_blockSize(newBlock) - get_blockSize(oldBlock));
}

/*
 *	Funcation Name: add_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Adds a memory block to the the free memory list
 */
void add_block_freeList(void *freeblock)
{ 
	//	TODO: 	Add the block to the free list
	//	Hint: 	You could add the free block at the end of the list, but need to check if there
	//			exits a list. You need to add the TAG to the list.
	//			Also, you would need to check if merging with the "adjacent" blocks is possible or not.
	//			Merging would be tideous. Check adjacent blocks, then also check if the merged
	//			block is at the top and is bigger than the largest free block allowed (128kB).
	
	block* cb = getBlock(freeblock);
	cb -> isfree = 1;
	block* curr = (block*) freeListHead;

	cb -> next = NULL;
	if (freeListHead == NULL){
		freeListHead = cb;
		freeListTail = cb;
	}else{
		cb -> prev = freeListTail;
		((block*) freeListTail) -> next = cb;
		freeListTail = cb;
	}
	mergeBlock(freeblock);
	
	// }
	//	Updates SMA info
	//char buf[50];
	totalAllocatedSize -= get_blockSize(freeblock);
	// sprintf(buf,"totalAllocatedSize %lu\n",totalAllocatedSize);
	// puts(buf);
	totalFreeSize += get_blockSize(freeblock);
}

/*
 *	Funcation Name: remove_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Removes a memory block from the the free memory list
 */
void remove_block_freeList(void *block_b)
{
	//	TODO: 	Remove the block from the free list
	//	Hint: 	You need to update the pointers in the free blocks before and after this block.
	//			You also need to remove any TAG in the free block.
	block* cb = getBlock(block_b);
	cb -> isfree = 0;
	if (cb -> prev)
		cb -> prev -> next = cb -> next;
	if (cb -> next)
		cb -> next -> prev = cb -> prev;
	
	if (freeListHead == cb)
		freeListHead = cb -> next;
	if (freeListTail == cb)
		freeListTail = cb -> prev;
	//	Updates SMA info
	totalAllocatedSize += get_blockSize(block_b);
	totalFreeSize -= get_blockSize(block_b);
}

/*
 *	Funcation Name: get_blockSize
 *	Input type:		void*
 * 	Output type:	int
 * 	Description:	Extracts the Block Size
 */
int get_blockSize(void *ptr)
{
	//	Points to the address where the Length of the block is stored
	int pSize = getBlock(ptr)->size;

	//	Returns the deferenced size
	return pSize;



}

/*
 *	Funcation Name: get_largest_freeBlock
 *	Input type:		void
 * 	Output type:	int
 * 	Description:	Extracts the largest Block Size
 */
int get_largest_freeBlock()
{
	int largestBlockSize = 0;

	//	TODO: Iterate through the Free Block List to find the largest free block and return its size
	block* cur_block = (block*) freeListHead;
	while (cur_block){
		if (cur_block-> size >= largestBlockSize){
			largestBlockSize =  cur_block->size;
		}
		cur_block = cur_block -> next;
	}
	return largestBlockSize;
}
void*getData(void*b){
	return (void*)(((block*)b)+1);
}
block* getBlock(void* ptr){
	return((block*)ptr)-1;
}
void* splitblock(void* cbptr, size_t l_size, size_t excessSize){
	block* cb = ((block*) cbptr)-1;
	size_t r_size = excessSize - FREE_BLOCK_HEADER_SIZE;
	cb -> size = l_size;
    cb -> isfree = 1;

	void* nbptr = (void*)(((char*)(cb+2))+l_size);
	block* nb = ((block*) nbptr)-1;
	nb -> size = r_size;
	nb -> isfree = 1;
    totalAllocatedSize -= FREE_BLOCK_HEADER_SIZE;
	totalFreeSize -= FREE_BLOCK_HEADER_SIZE;
	 return nbptr;
}
void mergeBlock(void*b){
	block* cb = getBlock(b);
	void* pbrk = sbrk(0);
	block* cur_block = (block*) freeListHead;
	while (cur_block){
		if (next_block_size(cur_block) ==(char*)cb){
			// curr block end at cb head
			cur_block -> size += cb -> size + FREE_BLOCK_HEADER_SIZE;
			//removeBlock(cb);
					if (cb -> next){
            cb -> next -> prev = cb -> prev;
        }
		    
	    if (cb -> prev){
            cb -> prev -> next = cb -> next;
        }
		    
	    if (freeListHead == cb)
		   { freeListHead = cb -> next;}
	    if (freeListTail == cb)
		    {freeListTail = cb -> prev;}
			cb = cur_block;
			totalFreeSize+=FREE_BLOCK_HEADER_SIZE;
			totalAllocatedSize += FREE_BLOCK_HEADER_SIZE;
		}else if (next_block_size(cb)== (char*)(cur_block)){
			// cb end at curr block
			cb -> size += cur_block -> size + FREE_BLOCK_HEADER_SIZE;
			//removeBlock(curr);
			if (cur_block -> next)
		   { cur_block -> next -> prev = cb -> prev;}
	    if (cur_block -> prev)
		    {cur_block -> prev -> next = cur_block -> next;}
	    if (freeListHead == cur_block)
		    {freeListHead = cur_block -> next;}
	    if (freeListTail == cur_block)
		    {freeListTail = cur_block -> prev;}
			totalFreeSize+=FREE_BLOCK_HEADER_SIZE;
			totalAllocatedSize += FREE_BLOCK_HEADER_SIZE;
		}
		cur_block = cur_block -> next;
	}

	if (next_block_size(cb)== (char*)(pbrk)&&sbrk(-(FREE_BLOCK_HEADER_SIZE + cb->size))!=0){
		if (cb -> next)
			{cb -> next -> prev = cb -> prev;}
		if (cb -> prev)
			{cb -> prev -> next = cb -> next;}
		if (freeListHead == cb)
			{freeListHead = cb -> next;}
		if (freeListTail == cb)
			{freeListTail = cb -> prev;}

	}

}

void removeBlock(block* cb){
	if(cb->next){
		cb -> next -> prev = cb -> prev;
	}
	if(cb->prev){
		cb -> prev -> next = cb -> next;
	}
	if (freeListHead == cb)
		freeListHead = cb -> next;
	if (freeListTail == cb)
		freeListTail = cb -> prev;
}
void replace_block(block*ob, block*nb){
    nb -> prev = ob -> prev;
	nb -> next = ob -> next;
	if (ob -> prev)
        ob -> prev -> next = nb;
    
	if (ob -> next)
    ob -> next -> prev = nb;	
}
char* next_block_size(block* b){
    return (char*)(b + 1)+b->size ;
}