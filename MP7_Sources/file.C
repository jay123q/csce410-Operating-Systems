/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id)
{
    Console::puts("Opening file.\n");
    this->currentFileSystem = _fs;
    this->cursorPointer = 0;
    this->fileId = _id;
    this->currentBlock = _id+1; // allocating
    Console::puts(" current block print "); Console::puti(this->currentBlock);Console::puts(" dead space \n");
    // assert(false);
    
}

File::~File()
{
    Console::puts("Closing file.\n");

    Console::puts("update inodes \n");
    auto fileIdInode = this->currentFileSystem->inodeList;
    for (int i = 0; i < currentFileSystem->MAX_INODES; i++)
    {

        if (fileIdInode[i]->fileId == this->fileId )
        {
            fileIdInode[i]->fileId = this->fileId;             // no disk id
            fileIdInode[i]->blockPointer = this->cursorPointer;
            fileIdInode[i]->blockNumberFile = this->currentBlock;
            Console::puts( (const char * ) currentBlock );
        }
    }

    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf)
{
    Console::puts("reading from file\n");
    int offset = 0;
    // unsigned char block_cache[512];



    if (_n > 512)
    {
        _n = 512;
    }
    int countDown = _n;

    // while (countDown > 0)
    // {
        // clear the junk that may have not beed writtenm this logic is from csce 313
        memset(block_cache, 0, block_size);
        this->currentFileSystem->disk->read(this->currentBlock, block_cache);

    // deep copy required, test checks for pointers
    for ( int i = 0 ; i < _n ; i++)
    {
        _buf[i] = block_cache[i];
    }
    // Console::puts(" read value");
    Console::puts(_buf);

    return (int) _n;
}

int File::Write(unsigned int _n, const char *_buf)
{
    Console::puts("writing to file\n");
    if (_n > 512)
    {
        _n = 512;
    }
    int offset = 0;

    // int countDown = _n;
    memset(block_cache,0,block_size); //  clear the cache to avoid erronous writes
    Console::puts(" the block is "); Console::puti(currentBlock); Console::puts(" dead space \n");
    memcpy(block_cache + this->cursorPointer, _buf, _n);
    Console::puts(" the block is "); Console::puts((const char * )block_cache+this->cursorPointer); Console::puts(" dead space \n");
     this->currentFileSystem->disk->write(this->currentBlock, block_cache + this->cursorPointer);
    this->cursorPointer = this->cursorPointer + _n;
    Console::puts(" curosor positioning! ");Console::puti( this->cursorPointer );Console::puts(" dead space \n ");

     Console::puts(" clear cache? "); Console::puts((const char * )block_cache); Console::puts(" dead space \n");
    return (int) _n;
}

void File::Reset()
{
    Console::puts("resetting file\n");
    this->cursorPointer = 0;
    
}

bool File::EoF()
{

    Console::puts("checking for EoF\n");
    if(this->cursorPointer > block_size-1)
    {
        Console::puts(" handle adjusting pointer now  \n ");
        this->currentBlock++;
        this->cursorPointer = 0;
        return true;
    }
    else
    {
        return false;
    }
}
