#ifndef _MIRROREDDISK_H_
#define _MIRROREDDISK_H_

#include "simple_disk.H"
#include "thread.H"
#include "scheduler.H"
#include "blocking_disk.H"
#include "frame_pool.H"


// Task 4 uncomment to thread

#define THREAD_TEST
enum class DISK_ID2 {MASTER = 0, DEPENDENT = 1};
class MirroredDisk
{
    private:
        bool * key;
        BlockingDisk * master;
        BlockingDisk * slave;


    public:
        MirroredDisk(DISK_ID _disk_id, unsigned int _size);
        /* Creates a BlockingDisk device with the given size connected to the
        MASTER or SLAVE slot of the primary ATA controller.
        NOTE: We are passing the _size argument out of laziness.
        In a real system, we would infer this information from the
        disk controller. */

        /* DISK OPERATIONS */

        virtual void read(unsigned long _block_no, unsigned char *_buf);
        /* Reads 512 Bytes from the given block of the disk and copies them
        to the given buffer. No error check! */

        virtual void write(unsigned long _block_no, unsigned char *_buf);
        /* Writes 512 Bytes from the buffer to the given block on the disk. */
        // virtual void push(unsigned long _block_no, unsigned char * _buf);

        // virtual Thread * pop();

        // virtual bool block();
         virtual void ready();
        virtual  void issue_operation(DISK_OPERATION _op, unsigned long _block_no, DISK_ID2 discID);
        // virtual bool is_ready();

        #ifdef THREAD_TEST
        bool testSetMux(bool * key);
    
        #endif
};
#endif
