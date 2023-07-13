
#include <inc/lib.h>

// malloc()
//	This function use NEXT FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.

struct Addr_Info
{
	uint32 Size;
	uint32 Var_Addr;
};

struct Addr_Info address_info[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE];
bool Tracker[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE] = { 0 };

uint32 start_address = USER_HEAP_START;
//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

void* malloc(uint32 size)
{
	//TODO: [PROJECT 2022 - [9] User Heap malloc()] [User Side]
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement NEXT FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_allocateMem to invoke the Kernel for allocation
	// 	4) Return pointer containing the virtual address of allocated space,
	//

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyNEXTFIT() and
	//sys_isUHeapPlacementStrategyBESTFIT() for the bonus
	//to check the current strategy

	size = ROUNDUP(size, PAGE_SIZE);
	uint32 Address;
	uint32 tmp;
	uint32 current_size = 0;
	uint32 X;
	bool Flag = 0;
	bool founded = 0;
	if (sys_isUHeapPlacementStrategyNEXTFIT())
	{
		X = (USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE;
		Address = start_address;
		while (X != 0)
		{
			if (Tracker[(Address - USER_HEAP_START) / PAGE_SIZE] == 0 && Flag == 1)
			{
				current_size += PAGE_SIZE;
				X--;
				Address += PAGE_SIZE;
			}
			else if (Tracker[(Address - USER_HEAP_START) / PAGE_SIZE] == 0 && Flag == 0)
			{
				Flag = 1;
				tmp = Address;
				current_size += PAGE_SIZE;
				X--;
				Address += PAGE_SIZE;
			}
			else
			{
				if (current_size >= size)
				{
					start_address = tmp;
					founded = 1;
					break;
				}
				X -= Tracker[(Address - USER_HEAP_START) / PAGE_SIZE];
				Address += PAGE_SIZE * Tracker[(Address - USER_HEAP_START) / PAGE_SIZE];
				Flag = 0;
				current_size = 0;
			}
			if (current_size >= size)
			{
				start_address = tmp;
				founded = 1;
				break;
			}
			if (Address >= USER_HEAP_MAX)
			{
				current_size = 0;
				Flag = 0;
				Address = USER_HEAP_START;
			}
		}
		if (founded == 0 && current_size >= size)
			start_address = tmp;
		else if (founded == 0)
			return NULL;
	}
	Address = (start_address - USER_HEAP_START) / PAGE_SIZE;
	Tracker[Address] = size / PAGE_SIZE;
	uint32 final_start_address = start_address;
	sys_allocateMem(start_address, size);
	address_info[(start_address - USER_HEAP_START) / PAGE_SIZE].Size = size;
	address_info[(start_address - USER_HEAP_START) / PAGE_SIZE].Var_Addr = start_address;
	if (start_address + size >= USER_HEAP_MAX)
		start_address = USER_HEAP_START;
	return (void*)final_start_address;
}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	panic("smalloc() is not required ..!!");
	return NULL;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	panic("sget() is not required ..!!");
	return 0;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
	//TODO: [PROJECT 2022 - [11] User Heap free()] [User Side]
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	//you shold get the size of the given allocation using its address
	//you need to call sys_freeMem()
	//refer to the project presentation and documentation for details
	uint32 tmp;
		uint32 Size;
		for (int i = 0; i < (USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE; i++)
		{
			if ((void*)address_info[i].Var_Addr == virtual_address)
			{
				tmp = (address_info[i].Var_Addr - USER_HEAP_START) / PAGE_SIZE;
				Tracker[tmp] = 0;
				Size=address_info[i].Size;
				address_info[i].Var_Addr = 0;
				address_info[i].Size = 0;
				break;
			}
		}
		sys_freeMem((uint32)virtual_address, Size);

}


void sfree(void* virtual_address)
{
	panic("sfree() is not requried ..!!");
}


//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2022 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

	return NULL;
}
