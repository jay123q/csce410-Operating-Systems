/*
     File        : blocking_disk.c

     Author      :
     Modified    :

     Description :

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "machine.H"


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/
SimpleDisk * System_disk;
Scheduler * SchedulerFifo;
BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size)
    : SimpleDisk(_disk_id, _size)
{


}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/


// void BlockingDisk::ready()
// {
//   while ((Machine::inportb(0x1F7) & 0x08) != 0)
//   {
//     // this->push()
//     SchedulerFifo->resume(Thread::CurrentThread()); 
//     SchedulerFifo->yield();
//   }

// }
// bool BlockingDisk::block()
// {
//   return (Machine::inportb(0x1F7) & 0x08) != 0;
// }


void BlockingDisk::read(unsigned long _block_no, unsigned char *_buf)
{
  // Machine::disable_interrupts();
  System_disk->issue_operation(DISK_OPERATION::READ, _block_no);
  Console::puts(" I fail here 66? \n");
  while (!is_ready())
  {
    // this->push()
    SchedulerFifo->resume(Thread::CurrentThread()); 
    SchedulerFifo->yield();
  }
  Console::puts(" I fail here 75? \n");

  int i;
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = Machine::inportw(0x1F0);
    _buf[i*2]   = (unsigned char)tmpw;
    _buf[i*2+1] = (unsigned char)(tmpw >> 8);
  }
  Console::puts(" I fail here 84? \n");
  // Machine::enable_interrupts();
  // this->push(_block_no, _buf);
}
// void BlockingDisk::push(unsigned long _block_no, unsigned char *_buf)
// {
//   linked_queue *new_thread = new linked_queue{_block_no, _buf};

//   // check normal ll conditions
//   if (head == nullptr)
//   {
//     head = new_thread;
//     tail = new_thread;
//   }
//   else
//   {
//     tail->next = new_thread;
//     tail = tail->next;
//   }
// }

void BlockingDisk::write(unsigned long _block_no, unsigned char *_buf)
{
    // Machine::disable_interrupts();
   System_disk->issue_operation(DISK_OPERATION::WRITE, _block_no);
  while (!is_ready())
  {
    // this->push()
    SchedulerFifo->resume(Thread::CurrentThread()); 
    SchedulerFifo->yield();
  }

  /* write data to port */
  int i; 
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
    Machine::outportw(0x1F0, tmpw);
  }
    // Machine::enable_interrupts();
}
bool BlockingDisk::is_ready() {
   return ((Machine::inportb(0x1F7) & 0x08) != 0);
}

// Thread *BlockingDisk::pop()
// {
//   // pop the tail
//    linked_queue *head_copy = head;
//   Thread *threadReturn = head_copy->thread;
//   if (head && head != tail)
//   { // if head exists and its not tail
//     Thread *current_thread = head->thread;
//     head = head->next;
//     // leave the current to the next
//     delete head_copy;
//   }
//   else
//   {
//     Console::puts(" Your ability at incompetence is impressive Mr. Clapp, line 80, write to blocking pop function \n ");
//     assert(false);
//   }
//   return threadReturn;
// }
