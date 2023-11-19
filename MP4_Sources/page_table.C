#include "assert.H"
#include "console.H"
#include "exceptions.H"
#include "page_table.H"
#include "paging_low.H"

PageTable *PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool *PageTable::kernel_mem_pool = NULL;
ContFramePool *PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;

void PageTable::init_paging(ContFramePool *_kernel_mem_pool,
                            ContFramePool *_process_mem_pool,
                            const unsigned long _shared_size)
{
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;

   Console::puts("Initialized Paging \n");
}

/* Initializes a page table with a given location for the directory and the
     page table proper.
     NOTE: The PageTable object still needs to be stored somewhere!
     Probably it is best to have it on the stack, as there is no
     memory manager yet...
     NOTE2: It may also be simpler to create the first page table *before*
     paging has been enabled.
  */
PageTable::PageTable()
{
   page_directory = (unsigned long *)(process_mem_pool->get_frames(1) * PAGE_SIZE);
   unsigned long *page_table = (unsigned long *)(process_mem_pool->get_frames(1) * PAGE_SIZE);
   unsigned long address = 0;

   for (unsigned int i = 0; i < 1024; i++)
   {
      page_table[i] = address | 3;
      address += PAGE_SIZE;
   }

   page_directory[0] = (unsigned long)page_table | 3;
   for (unsigned int i = 1; i < 1024; i++)
   {
      page_directory[i] = 0 | 2;
   }

   page_directory[1023] = (unsigned long)page_directory | 3;

   // Set up the virtual memory pool
   for (unsigned int i = 0; i < 512; i++)
   {
      vmArray[i] = NULL;
   }

   Console::puts("Construction of Page Table Complete \n");
}

/* Makes the given page table the current table. This must be done once during
     system startup and whenever the address space is switched (e.g. during
     process switching). */
void PageTable::load()
{
   current_page_table = this;
   write_cr3((unsigned long) page_directory);
   Console::puts(" Page Table is loaded\n");
}

void PageTable::enable_paging()
{
   paging_enabled = 1;

   write_cr3((unsigned long)current_page_table->page_directory);
   write_cr0(read_cr0() | 0x80000000);
   Console::puts(" Paging is on\n");
}
unsigned long *PageTable::Create_Directory_Handle(unsigned long pageDirectoryIndex, unsigned long *directory, unsigned long *table)
{
   directory[pageDirectoryIndex] = (unsigned long)(process_mem_pool->get_frames(1) * PAGE_SIZE | 3);

   // Get the virtual address of the page table
   unsigned long virtPageAddy = pageDirectoryIndex * PAGE_SIZE;
   table = (unsigned long *)(0xFFC00000 + virtPageAddy);

   // Initialize the new page table
   for (int i = 0; i < 1024; i++)
   {
      table[i] = 0 | 2;
   }
   return table;
}
void PageTable::handle_fault(REGS *_r)
{
   // unsigned long* directory = (unsigned long*)read_cr3();
   // The directory is located at the last entry in virtual memory
   unsigned long *directory = (unsigned long *)(0xFFFFF000);
   unsigned long faultAddy = read_cr2();
   unsigned long pageDirectVir = faultAddy >> 22; // 2^22 = 4gb
   unsigned long pageTableAddy = faultAddy >> 12; // 2^12 = 4kb
   unsigned long virtPageAddy = pageDirectVir * PAGE_SIZE;
   unsigned long *table = NULL;
   bool gateCheck = true;
   if ((_r->err_code & 1) == 1)
   {
      Console::puts(" AHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH i DONT EXIST PANIC 97 ");
   }
   else
   {
      // get the active vm pool we have into vm pool
      VMPool **vmPoolsCheck = current_page_table->vmArray;
      // Iterate through the vm_pools
      for (int i = 0; i < current_page_table->vm_count; i++)
      {
         // if we are empty move on
         gateCheck = true;
         if (vmPoolsCheck[i] == NULL)
         {
            gateCheck = false;
         }
         // check if we exist as a legitimatte storage box
         if (gateCheck)
         {
            if (vmPoolsCheck[i]->is_legitimate(faultAddy))
            {

               // check if we have a faulty directory
               if ((directory[pageDirectVir] & 1) == 0)
               {
                  // here we are going to find the table like we did before, then allocate a frame, then allocate the table to 2^12
                  table = Create_Directory_Handle(pageDirectVir, directory, table);
               }

               else
               {
                  // create a cirtual page table
                  table = (unsigned long *)(0xFFC00000 + virtPageAddy);
               }
               table[pageTableAddy % 1024] =
                   (unsigned long)process_mem_pool->get_frames(1) * PAGE_SIZE | 3;
            }
         }
      }
   }

   Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool *_vm_pool)
{
   if (vm_count >= 512)
   {
      Console::puts("vm pool has many kids inside and no room for you 153 page table c \n");
   }
   else
   {
      vmArray[vm_count] = _vm_pool;
      vm_count++;
      Console::puts(" licessned  VM Pool\n");
   }
}

void PageTable::free_page(unsigned long page_no) {
    // Calculate the indexes for the page directory and page table
    unsigned long pageDirectoryIndex = page_no >> 22;
    unsigned long pageTableIndex = page_no >> 12;

    // Calculate the virtual and physical addresses of the page table
   // virtual is the pageDirctIndex *  PageSize
    unsigned long* table = (unsigned long*)(0xFFC00000 + pageDirectoryIndex * PAGE_SIZE);
    unsigned long pageFrame = table[pageTableIndex % 1024];

    // Free the page frame
    process_mem_pool->release_frames(pageFrame / PAGE_SIZE);

    // Mark the page as invalid
    table[pageTableIndex % 1024] &= ~1;

    // Invalidate the TLB
    write_cr3((unsigned long)current_page_table->page_directory);

    // Print a message to indicate that the page has been freed
    Console::puts(" freed bird freed the page fly baby fly \n");
}

