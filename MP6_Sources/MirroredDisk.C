#include "MirroredDisk.H"
#include "assert.H"
#include "utils.H"
#include "console.H"
#include "machine.H"


Scheduler * SchedulerMirror;
 #ifdef THREAD_TEST
 // this logic is from https://courses.engr.illinois.edu/cs241/sp2012/lectures/23-inside_sem.pdf

bool MirroredDisk::testSetMux(bool * key) 
{
    int box = *(this->key);
    *(this->key) = true; 
    return box;

}

#endif 

MirroredDisk::MirroredDisk(DISK_ID _disk_id, unsigned int _size)
{
    this->master = new BlockingDisk(DISK_ID::MASTER, _size);
    this->slave = new BlockingDisk(DISK_ID::DEPENDENT, _size);
    *(this->key) = false;
}

void MirroredDisk::read(unsigned long _block_no, unsigned char *_buf)
{ 
    // Machine::disable_interrupts();
    #ifdef THREAD_TEST
        // lock the mux
        while(testSetMux(key));
    #endif
    issue_operation(DISK_OPERATION::READ, _block_no,  DISK_ID2::MASTER );
    issue_operation(DISK_OPERATION::READ, _block_no, DISK_ID2::DEPENDENT  );
    Console::puts(" I fail here 66? \n");
    ready();
    Console::puts(" I fail here 75? \n");

    int i;
    unsigned short tmpw;
    for (i = 0; i < 256; i++) {
        tmpw = Machine::inportw(0x1F0);
        _buf[i*2]   = (unsigned char)tmpw;
        _buf[i*2+1] = (unsigned char)(tmpw >> 8);
    }
    #ifdef THREAD_TEST
        // unlock the mux
        *(this->key) = false;
    #endif
    // Machine::enable_interrupts();
    // this->push(_block_no, _buf);
}

void MirroredDisk::write(unsigned long _block_no, unsigned char *_buf)
{
    // Machine::disable_interrupts();
    #ifdef THREAD_TEST
        // lock the mux
        while(testSetMux(key));
    #endif
    // issue_operation(DISK_OPERATION::WRITE, _block_no);    

    this->master->write(_block_no , _buf);
    this->slave->write(_block_no , _buf);
    // ready();
    #ifdef THREAD_TEST
        // unlock the mux
        *(this->key) = false;
    #endif
    // Machine::enable_interrupts();
}

void MirroredDisk::issue_operation(DISK_OPERATION _op, unsigned long _block_no , DISK_ID2 disk_id) {

  Machine::outportb(0x1F1, 0x00); /* send NULL to port 0x1F1         */
  Machine::outportb(0x1F2, 0x01); /* send sector count to port 0X1F2 */
  Machine::outportb(0x1F3, (unsigned char)_block_no);
                         /* send low 8 bits of block number */
  Machine::outportb(0x1F4, (unsigned char)(_block_no >> 8));
                         /* send next 8 bits of block number */
  Machine::outportb(0x1F5, (unsigned char)(_block_no >> 16));
                         /* send next 8 bits of block number */
  Machine::outportb(0x1F6, ((unsigned char)(_block_no >> 24)&0x0F) | 0xE0 | ((int) disk_id << 4));
                         /* send drive indicator, some bits, 
                            highest 4 bits of block no */

  Machine::outportb(0x1F7, (_op == DISK_OPERATION::READ) ? 0x20 : 0x30);

}

void MirroredDisk::ready()
{
  while ((!master->is_ready() || !slave->is_ready()))
  {


    SchedulerMirror->resume(Thread::CurrentThread()); 
    SchedulerMirror->yield();
  }
}

