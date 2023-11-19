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
    this->head = nullptr;
    this->tail = nullptr;
    size = 0;
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/


// bool BlockingDisk::block()
// {
//   return (Machine::inportb(0x1F7) & 0x08) != 0;
// }


void BlockingDisk::read(unsigned long _block_no, unsigned char *_buf)
{
  #ifdef INTERUPT_TEST
  Machine::disable_interrupts();
  #endif
  System_disk->issue_operation(DISK_OPERATION::READ, _block_no);
  while ((!is_ready()))
  {
      #ifdef INTERUPT_TEST
        this->push(Thread::CurrentThread());
      #else

    SchedulerFifo->resume(Thread::CurrentThread()); 
  Console::puts(" I fail here 66? \n");
       #endif
    SchedulerFifo->yield();
  Console::puts(" I fail here 75? \n");
  }



  int i;
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = Machine::inportw(0x1F0);
    _buf[i*2]   = (unsigned char)tmpw;
    _buf[i*2+1] = (unsigned char)(tmpw >> 8);
  }
  Console::puts(" I fail here 84? \n");
  #ifdef INTERUPT_TEST
   Machine::enable_interrupts();
   #endif
  // this->push(_block_no, _buf);
}


void BlockingDisk::write(unsigned long _block_no, unsigned char *_buf)
{
    #ifdef INTERUPT_TEST
     Machine::disable_interrupts();
     #endif
   System_disk->issue_operation(DISK_OPERATION::WRITE, _block_no);
  while ((!is_ready()))
  {
      #ifdef INTERUPT_TEST
        this->push(Thread::CurrentThread());
      #else

    SchedulerFifo->resume(Thread::CurrentThread()); 
       #endif
    SchedulerFifo->yield();
  }

  /* write data to port */
  int i; 
  unsigned short tmpw;
  for (i = 0; i < 256; i++) {
    tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
    Machine::outportw(0x1F0, tmpw);
  }
    #ifdef INTERUPT_TEST
    Machine::enable_interrupts();
    #endif
}

bool BlockingDisk::is_ready() {
   return ((Machine::inportb(0x1F7) & 0x08) != 0);
}


void BlockingDisk::ready()
{
  while ((!is_ready()))
  {
      #ifdef INTERUPT_TEST
        this->push(Thread::CurrentThread());
      #else

    SchedulerFifo->resume(Thread::CurrentThread()); 
       #endif
    SchedulerFifo->yield();
  }
}


void BlockingDisk::push( Thread * pushThread )
{
  linked_queue *new_thread = new linked_queue{ pushThread , nullptr };

  // check normal ll conditions
  if (head == nullptr)
  {
    head = new_thread;
    tail = new_thread;
  }
  else
  {
    tail->next = new_thread;
    tail = tail->next;
  }
  size++;
}


Thread *BlockingDisk::pop()
{
  // hey this is a copy from scheduler.C, I might ahve been able to just call yield
  // pop the tail
  if (size && head)
  { // if head exists and its not tail
    linked_queue *head_copy = head;
    Thread *threadReturn = head_copy->thread;
    Thread *current_thread = head->thread;
    head = head->next;
    size--;
    if(!size)
    {
      // head and tail is null
      tail = nullptr;
    }
    // leave the current to the next
    delete head_copy;
    return threadReturn;
  }
  else
  {
    Console::puts(" Your ability at incompetence is impressive Mr. Clapp, line 80, write to blocking pop function \n ");
    assert(false);
  }
}


  #ifdef INTERUPT_TEST
  #endif
  void BlockingDisk::handle_interrupt(REGS *_r)
  {
    Thread * nextThread = pop();
    SchedulerFifo->resume(nextThread->Thread::CurrentThread()); 
  }