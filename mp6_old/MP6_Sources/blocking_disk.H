/*
     File        : blocking_disk.H

     Author      : 

     Date        : 
     Description : 

*/

#ifndef _BLOCKING_DISK_H_
#define _BLOCKING_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "simple_disk.H"
#include "thread.H"
#include "scheduler.H"

// Task 2 uncomment to test


// #define INTERUPT_TEST

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* B l o c k i n g D i s k  */
/*--------------------------------------------------------------------------*/
struct linked_queue
{
   // unsigned long blockNum;
   // unsigned char * buffer;
   Thread *  thread;
   linked_queue * next;
};


class BlockingDisk : public SimpleDisk {
private:
linked_queue * head;
linked_queue *  tail;
int size;




public:
   BlockingDisk(DISK_ID _disk_id, unsigned int _size); 
   /* Creates a BlockingDisk device with the given size connected to the 
      MASTER or SLAVE slot of the primary ATA controller.
      NOTE: We are passing the _size argument out of laziness. 
      In a real system, we would infer this information from the 
      disk controller. */

   /* DISK OPERATIONS */

   virtual void read(unsigned long _block_no, unsigned char * _buf);
   /* Reads 512 Bytes from the given block of the disk and copies them 
      to the given buffer. No error check! */

   virtual void write(unsigned long _block_no, unsigned char * _buf);
   /* Writes 512 Bytes from the buffer to the given block on the disk. */
   // virtual void push(unsigned long _block_no, unsigned char * _buf);

   virtual bool is_ready();
   virtual void ready();
  #ifdef INTERUPT_TEST
   virtual Thread * pop();
   virtual void push(Thread * pushThread);
   // virtual bool block();
   virtual void handle_interrupt(REGS * _r);
#endif
};

#endif
