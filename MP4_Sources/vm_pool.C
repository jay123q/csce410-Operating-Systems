/*
 File: vm_pool.C

 Author:
 Date  :

 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/
int maxAllocate = 512;
VMPool::VMPool(unsigned long _base_address,
               unsigned long _size,
               ContFramePool *_frame_pool,
               PageTable *_page_table)
{

    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;

    vmList = (VmStruct *)frame_pool->get_frames(1);
    allocatedVM = 0;

    page_table->register_pool(this);
    Console::puts("Vm pool is constructed .\n");
}

unsigned long VMPool::allocate(unsigned long _size)
{
    unsigned long address;
    // if not allocated then reallocate at base
    if (allocatedVM == 0)
    {
        address = base_address;
    }
    else
    {
        VmStruct &oldRegion = vmList[allocatedVM - 1];
        address = oldRegion.start_address + oldRegion.size;
    }

    vmList[allocatedVM] = {address, _size};
    allocatedVM++;
    Console::puts(" ALLOCATE ALLOCATED region of memory.\n");
    return address;
}

void VMPool::release(unsigned long start_address)
{
    // Find the region with the specified start address
    int checkIndexVm = -1;
    for (int i = 0; i < allocatedVM; i++)
    {
        if (vmList[i].start_address == start_address)
        {
            checkIndexVm = i;
            break;
        }
    }
    if (checkIndexVm == -1)
    {
        Console::puts(" RELEASE HUH HELP MA \n");
        return;
    }

    // Free all pages in the region
    unsigned long pageAddy = start_address;
    unsigned long end_address = start_address + vmList[checkIndexVm].size;
    while (pageAddy < end_address)
    {
        page_table->free_page(pageAddy);
        pageAddy += PageTable::PAGE_SIZE;
    }

    // Shift all vmList after the freed one down by one
    for (int i = checkIndexVm; i < allocatedVM - 1; i++)
    {
        vmList[i] = vmList[i + 1];
    }
    allocatedVM--;

    // reloadd the page table
    page_table->load();
    Console::puts(" Memory is free baby bird fly! RELEASE COMPLETE \n");
}

bool VMPool::is_legitimate(unsigned long _address)
{
    // Check if the address is within the range of any of the allocated virtual memory regions
    for (unsigned int i = 0; i < allocatedVM; i++)
    {
        unsigned long end_address = vmList[i].start_address + vmList[i].size - 1;
        if (_address >= vmList[i].start_address && _address <= end_address)
        {
            Console::puts("Address is part of an allocated region.\n");
            return true;
        }
    }
    Console::puts("Address is not part of an allocated region.\n");
    return false;
}