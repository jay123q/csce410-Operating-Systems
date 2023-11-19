#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable *PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool *PageTable::kernel_mem_pool = NULL;
ContFramePool *PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;

void PageTable::init_paging(ContFramePool *_kernel_mem_pool,
                            ContFramePool *_process_mem_pool,
                            const unsigned long _shared_size)
{
   // unsigned long *page_table = (unsigned long *) 0x9D000;
   // http://www.osdever.net/tutorials/view/implementing-basic-paging
   //  assert(false);
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{

   page_directory = (unsigned long *)(kernel_mem_pool->get_frames(1) * PAGE_SIZE);
   unsigned long *page_table = (unsigned long *)(kernel_mem_pool->get_frames(1) * PAGE_SIZE);
   unsigned long address = 0;

   // http://www.osdever.net/tutorials/view/implementing-basic-paging
   // map the first 4MB of memory
   for (unsigned int i = 0; i < 1024; i++)
   {
      page_table[i] = address | 3; // set to supervisor
      address = address + 4096;    // 4096 = 4kb
   };

   page_directory[0] = (unsigned long)page_table;
   page_directory[0] = page_directory[0] | 3;

   for (unsigned int i = 1; i < 1024; i++)
   {
      page_directory[i] = 0 | 2;
   };

   Console::puts("Constructed Page Table object\n");
}

void PageTable::load()
{
   write_cr3(*page_directory);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   paging_enabled = 1;
   write_cr0(read_cr0() | 0x800000000);
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS *_r)
{

   // first find a free frame from the public common pool
   // two level paging, finding the table, and finding the board game inside of it

   unsigned long *page_dir = (unsigned long *)read_cr3(); // read from the reserved register
   unsigned long *page_table;
   unsigned long page_address;
   if (_r->err_code & 1)
   {
      assert(" AHHHHHHHHHHHHHHHHHHHHHHHH unhandled page fault 54 ")
   }
   else
   {
      // do I exist at 1 mb?
      page_address = read_cr2();
      unsigned long megaBytePageAddress = page_address >> 22;
      if ((page_dir[megaBytePageAddress]) & 1 == 0)
      {
         // this page table is now being loaded into memory at 1mb
         page_dir[megaBytePageAddress] = (PageTable::process_mem_pool->get_frames(1) * PAGE_SIZE);
         // set the bits pogging
         page_dir[megaBytePageAddress] |= 3;
      }
      page_table = (unsigned long *)(page_dir[megaBytePageAddress] & 0xFFFFF000); // 0xFFFF000 is 1 mb in hexa

      page_address = (page_address << 12) % 1024;
      if ((page_dir[page_address]) & 1 == 0)
      {
         page_dir[page_address] = (PageTable::process_mem_pool->get_frames(1) * PAGE_SIZE);
         page_dir[page_address] |= 2;
      }
   }

   Console::puts("handled page fault\n");
}
