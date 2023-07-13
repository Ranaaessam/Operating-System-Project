#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//2022: NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
struct info{
	uint32 address;
	int pages_number;
			};
struct info free_array[100];
int index =0;
int last_aloocated_address = 0;
int taken_frames_array[40959];
uint32 allocated_size_array[40959];
uint32 return_va= 0;


uint32 Nextfit(int desired_pages)
 {
	int founded = 0;
	int counter = 0;
	int address = last_aloocated_address;
	uint32 start_virtual_address = (address*PAGE_SIZE) + KERNEL_HEAP_START;
	uint32 start_address = 0;
	do {
		//here we are Checking if the frame is empty
		if(taken_frames_array[address] == 0)
		{
			counter++;
			if(start_address == 0){
				start_address = (address*PAGE_SIZE) + KERNEL_HEAP_START;;
			}
			if(counter == desired_pages)
			{
				founded = 1;
				break;
			}
			address++;
		}
		else
		{
			address++;
			counter = 0;
			start_address = 0;
		}
		if((address*PAGE_SIZE+KERNEL_HEAP_START) == KERNEL_HEAP_MAX)
		{
			address  = 0;
			counter = 0;
			start_address = 0;
		}
	}
	while(!founded && (address*PAGE_SIZE+KERNEL_HEAP_START) != start_virtual_address);
		if(founded == 1)
		{
			last_aloocated_address = address;
			return start_address;
		}
		else {
			return 0;
	}
 }
void* BestFit(unsigned int size)
 {
	int desired_pages=ROUNDUP(size,PAGE_SIZE)/PAGE_SIZE;
	uint32 address_array[100];
	int available_pages[100];
	uint32 start_address = 0;
	uint32 * Page_Table_Ptr=NULL;
	int index=0;
	int segments=0;
	int h = 99999999;
	for(int i = KERNEL_HEAP_START; i<KERNEL_HEAP_MAX; i+=PAGE_SIZE)
	{
	 struct Frame_Info* ptr_frame_info;
	 ptr_frame_info= get_frame_info(ptr_page_directory, (void *)i, &Page_Table_Ptr);
		 if (ptr_frame_info==0)
		 {
			 if (start_address==0)
			 {
				 start_address=i;
				 address_array[index]=i;
				 available_pages[index]=1;
				 segments+=1;
			 }
			 else
			 {
				 available_pages[index]+=1;
			 }
		 }
		 else if (ptr_frame_info!=NULL && start_address!=0)
		 {
			 start_address=0;
			 index+=1;
		 }
	}

	if (segments == 0)
	{
		return NULL ;
	}

	for(int i = 0 ; i<segments;i++)
	{
		if (available_pages[i]>=desired_pages && available_pages[i]<h)
		{
			h=available_pages[i];
			start_address= address_array[i];
		}
	}

	if( h == 99999999)
	{
		return NULL ;
	}
	uint32 final_start_address = start_address ;
	for (int i =0 ; i<desired_pages;i++)
	 {
		struct Frame_Info* ptr_frame_info;
		allocate_frame(&ptr_frame_info);
		map_frame(ptr_page_directory, ptr_frame_info, (void*)start_address, PERM_PRESENT|PERM_WRITEABLE);
		start_address+=PAGE_SIZE;
	 }
	free_array[index].address=final_start_address;
	free_array[index].pages_number=desired_pages;
	index++;
	allocated_size_array[(final_start_address-KERNEL_HEAP_START)/PAGE_SIZE] = desired_pages;
	return (void *)final_start_address;
 }
void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT 2022 - [1] Kernel Heap] kmalloc()
	// Write your code here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");
	//NOTE: Allocation using NEXTFIT strategy
	//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	//refer to the project presentation and documentation for details
	uint32 neededSize = ROUNDUP(size,PAGE_SIZE);
	int desired_pages = neededSize/PAGE_SIZE;
	uint32 final_start_address = Nextfit(desired_pages);
	if(isKHeapPlacementStrategyBESTFIT())
	{
		return (void*)BestFit(size);
	}
	uint32 assign = final_start_address;
	if (final_start_address == 0)
	{
		return NULL;
	}
	else
	{
		for(int i = 0; i <desired_pages; i++)
		{
			struct Frame_Info* newFrame = NULL;
			int final_start_address = allocate_frame(&newFrame);
			if (final_start_address == E_NO_MEM)
			{
					return NULL;
			}
			else
			{
				int final_start_address = map_frame(ptr_page_directory,newFrame,(void*)assign, PERM_WRITEABLE|PERM_PRESENT);
				if (final_start_address == E_NO_MEM)
				{
						free_frame(newFrame);
						return NULL;
				}
				taken_frames_array[(assign-KERNEL_HEAP_START)/PAGE_SIZE] = 1;
				assign+=PAGE_SIZE;
			}
		}
	}
	return_va = assign;
	 allocated_size_array[(final_start_address-KERNEL_HEAP_START)/PAGE_SIZE] = desired_pages;
	 return (void*)final_start_address;
	//TODO: [PROJECT 2022 - BONUS1] Implement a Kernel allocation strategy
	// Instead of the Next allocation/deallocation, implement
	// BEST FIT strategy
	// use "isKHeapPlacementStrategyBESTFIT() ..."
	// and "isKHeapPlacementStrategyNEXTFIT() ..."
	//functions to check the current strategy
	//change this "return" according to your answer

}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT 2022 - [2] Kernel Heap] kfree()
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
	uint32 va = (uint32) virtual_address;
	int size = allocated_size_array[(va - KERNEL_HEAP_START)/PAGE_SIZE];
	allocated_size_array[(va - KERNEL_HEAP_START)/PAGE_SIZE] = 0;
	for (int i = 0; i < size; i ++)
	{
		unmap_frame(ptr_page_directory,(void*)va);
		taken_frames_array[(va-KERNEL_HEAP_START)/PAGE_SIZE] = 0;
		va= (va + PAGE_SIZE);
	}
	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details

}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	//TODO: [PROJECT 2022 - [3] Kernel Heap] kheap_virtual_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_virtual_address() is not implemented yet...!!");

	//return the virtual address corresponding to given physical_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer
	struct Frame_Info * physical_frame = to_frame_info(physical_address);
	unsigned int va;
	for(va= KERNEL_HEAP_START;va<return_va;va+=PAGE_SIZE)
	{
		uint32* ptr_page_table =NULL;
		struct Frame_Info * virtual_frame = get_frame_info(ptr_page_directory,(void*)va,&ptr_page_table);
		if(virtual_frame == physical_frame)
		{
			return va;
		}
	}


	return 0;
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT 2022 - [4] Kernel Heap] kheap_physical_address()
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//change this "return" according to your answer
	uint32 physical_address;
	uint32 *ptr_page_table = NULL;
	struct Frame_Info* ptr_frame_info = get_frame_info(ptr_page_directory, (void*)virtual_address, &ptr_page_table);
	physical_address = to_physical_address(ptr_frame_info);
	 uint32 present_bit=ptr_page_table[PTX(virtual_address)]& PERM_PRESENT;
	 if (present_bit==0)
		 return 0 ;
	return physical_address;

}

