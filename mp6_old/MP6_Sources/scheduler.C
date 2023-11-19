/*
 File: scheduler.C

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
#include "simple_timer.H"
#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

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
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler()
{
  // this implementation was inspired from CSCE 313 where we used locked to transfer TCP and passed into pairs that contained the needed info
  linked_queue_SS *head = nullptr;
  linked_queue_SS *tail = nullptr;
  head->thread = nullptr;
  head->next = nullptr;
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield()
{
  // stop you from screaming no default execpt handler
  // turn expections off while we are inserting to prevent a interupt that pulls us away while another thread can plug into a next area
  Machine::disable_interrupts();

  linked_queue_SS *copy_head = head;
  Thread *current_thread = head->thread;
  head = head->next;
  // leave the current to the next
  delete copy_head;
  Thread::dispatch_to(current_thread);

  Machine::enable_interrupts();
}

void Scheduler::resume(Thread *_thread)
{
  // stop you from screaming no default execpt handler

  Machine::disable_interrupts();
  // create the new thread to add
  linked_queue_SS *new_thread = new linked_queue_SS{_thread, nullptr};
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
  Machine::enable_interrupts();
}

void Scheduler::add(Thread *_thread)
{
  // I really think this add should have something more, but how do you prepare for a thread to be added?
  // the thread must exist, and I cannot think of any edge cases for a thread object
  resume(_thread);
}

void Scheduler::terminate(Thread *_thread)
{
  linked_queue_SS *copy_head = head;
  bool findThread = false;
  while (copy_head != nullptr)
  {
    if (copy_head->thread == _thread)
    {
      findThread == true;
    }
  }

  if (findThread)
  {
    // we found the proper thread and now can yield
    delete _thread;
    yield();
  }
  Console::puts(" the proper thread was not found \n ");
  assert(false);
}

RoundRobin::RoundRobin(int timer)
{
  // kill  the thread then thread = thread->next
  linked_queue_SS *RRhead = nullptr;
  linked_queue_SS *RRtail = nullptr;
  quantum = timer;

  // escape_button = (InterruptHandler * ) (InterruptHandler::register_handler(0,( InterruptHandler * ) ( 1000/timer  ) ));
  InterruptHandler::register_handler(0, (InterruptHandler *)(1000 / timer));
  Console::puts("Constructed Scheduler.\n");
}

void RoundRobin::RRyield()
{

  Thread *current_thread = RRhead->thread;
  linked_queue_SS *copy_head = RRhead;
  RRhead = RRhead->next;
  delete copy_head;
  Console::puts(" got past yield into dispath \n ");
  Thread::dispatch_to(current_thread);
}

void RoundRobin::RRresume(Thread *_thread)
{

  // create the new thread to add
  linked_queue_SS *new_thread = new linked_queue_SS{_thread, nullptr};
  // check normal ll conditions
  if (RRhead == nullptr)
  {
    RRhead = new_thread;
    RRtail = new_thread;
  }
  else
  {
    RRtail->next = new_thread;
    RRtail = RRtail->next;
  }
}

void RoundRobin::RRadd(Thread *_thread)
{
  // I really think this add should have something more, but how do you prepare for a thread to be added?
  // the thread must exist, and I cannot think of any edge cases for a thread object
  RRresume(_thread);
}

void RoundRobin::RRterminate(Thread *_thread)
{
  linked_queue_SS *copy_head = RRhead;
  bool findThread = false;
  while (copy_head != nullptr)
  {
    if (copy_head->thread == _thread)
    {
      findThread = true;
    }
  }

  if (findThread)
  {
    // we found the proper thread and now can yield
    delete _thread;
    RRyield();
  }
  Console::puts(" the proper thread was not found \n ");
  assert(false);
  // assert(false);
}