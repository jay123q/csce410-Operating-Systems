/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
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
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store
   inodeList from and to disk. */
// Inode * inodePointer;
SimpleDisk *Disk;
/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem()
{
    Console::puts("In file system constructor.\n");
    memset(inodeList, 0, block_size);
    // memset(freeList, 0, block_size);
}

FileSystem::~FileSystem()
{
    Console::puts("unmounting file system saving data \n");
    /* Make sure that the inode list and the free list are saved. */
    this->disk->write(1, freeList);
    memset(freeList, 'f', block_size);
    // clear freelist, read form thers
    this->disk->read(0, freeList);

    for (int i = 0; i < SYSTEM_BLOCKS; i++)
    {
        this->disk->write(i, freeList);
    }

    // read the data back to the blokcs now
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk *_disk)
{
    Console::puts("mounting file system from disk\n");
    this->disk = _disk;
    // /* Here you read the inode list and the free list into memory */
    return true;
    // assert(false);
}

bool FileSystem::Format(SimpleDisk *_disk, unsigned int _size)
{ // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodeList and for the free list
       are marked as used, otherwise they may get overwritten. */
    unsigned char veracityCheckerNodes[block_size]; // so this is for passing ablock side, I know the file descriptor is -1 block side
    memset(veracityCheckerNodes, 0, block_size);

    for (int i = 0; i < SYSTEM_BLOCKS; i++)
    {
        _disk->write(i, veracityCheckerNodes);
    }
    // read the data back to the blokcs now

    _disk->read(0, veracityCheckerNodes);

    // now return freelist back to its proper purpose

    for (int i = 0; i < MAX_INODES; i++)
    {

        inodeList[i]->blockNumberFile = -1; // where allocated on block
        inodeList[i]->fileId = 0;           // no disk id
    }
    _disk->write(0, veracityCheckerNodes);

    // clear free list

    memset(freeList, 'f', block_size);

    _disk->read(1, freeList);
    for (int i = 0; i < block_size; i++)
    {
        freeList[i] = 'f';
    }
    freeList[0] = 't';
    freeList[1] = 't';
    _disk->write(1, freeList);
    Console::puts(" disk format complete \n");
    return true;
}

// short FileSystem::GetFreeInode()
// {

// }

// int FileSystem::GetFreeBlock()
// {

// }

Inode *FileSystem::LookupFile(int _file_id)
{
    Console::puts(" looking up file with id = ");
    Console::puti(_file_id);
    Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    for (int i = 0; i < MAX_INODES; i++)
    {
        if (this->inodeList[i]->fileId == _file_id)
        {
            return inodeList[i];
        }
    }
    Console::puts(" Inode not found ith file id \n ");
    assert(false);
}

bool FileSystem::CreateFile(int _file_id)
{

    Console::puts("creating file with id:");
    Console::puti(_file_id);
    Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    for (int i = 0; i < MAX_INODES; i++)
    {

        if (inodeList[i]->fileId == _file_id)
        {
            Console::puts(" same file id, file already exists ");
            assert(false);
        }
    }
    // now find the

    int indexFree = 0;
    // start i at 2 since its we know 0 and 1 full
    for (int i = 0; i < block_size; i++)
    {

        if (freeList[i] == 'f')
        {
            indexFree = i;
            // Console::puti( inodeList[i]->blockNumberFile );

            break;
        }
    }
    // Console::puts(" the ffree index is "); Console::puti(indexFree); Console::puts(" dead space \n ");
    freeList[indexFree] = 't'; // I think I am marking the block as full, I think I can simplify implementation to only have this as the only marking of state
    unsigned char freeListDup[block_size];
    for (int i = 0; i < block_size; i++)
    {
        // Console::puts(" i "); Console::puti(freeList[i]); Console::puts(" dead space \n ");

        freeListDup[i] = freeList[i];
    }
    disk->write(1, freeListDup); // update the freelist

    // find a inode to allocate
    unsigned char veracityCheckerNodes[block_size];
    memset(veracityCheckerNodes, 0, block_size); // this was a bugger of a error to debug, was overwritting freelist memory by not calling it here

    this->disk->read(0, veracityCheckerNodes);

    Console::puts(" disk update inodes \n ");

    for (int i = 0; i < MAX_INODES; i++)
    {
        int temp = veracityCheckerNodes[i];
        if (temp == 0)
        {
            // Console::puts(" do I exist ? \n");

            inodeList[i]->blockNumberFile = indexFree; // first block to start
            inodeList[i]->fileId = _file_id;
            veracityCheckerNodes[i] = inodeList[i]->blockNumberFile;

            //  Console::puts(" i "); Console::puti(inodeList[i]->fileId); Console::puts(" dead space ");
            break;
        }
        // inodeList[i]->fileId = 0;             // no disk id
        // veracityCheckerNodes[i] = inodeList[i]->blockNumberFile;
    }

    // for (int i = 0; i < block_size; i++)
    // {
    //     if(!veracityCheckerNodes[i])
    //     {
    //     //  Console::puts(" i "); Console::puti(veracityCheckerNodes[i]); Console::puts(" dead space \n ");
    //     }
        // Console::puts(" freeIndex "); Console::puti(freeList[i]); Console::puts(" dead space \n ");

    // }    
    this->disk->write(0, veracityCheckerNodes); // update directory
    Console::puts(" file created \n ");
    return true;
}

bool FileSystem::DeleteFile(int _file_id)
{
    // for(int i = 0 ; i < MAX_INODES ; i++)
    // {
    //          Console::puts(" i "); Console::puti(inodeList[i]->fileId); Console::puts(" dead space \n ");
    // }
    Console::puts("deleting file with id:");
    Console::puti(_file_id);
    Console::puts("\n");
    /* First, check if the file exists. If not, throw an error.
       Then free all blocks that belong to the file and delete/invalidate
       (depending on your implementation of the inode list) the inode. */

    // to avoid a annoyign write operation, I know the first two blocks are taken, and since I ddesigned the system to sequentially write
    // I can just check allocatedblocks(2) + fileIdent = taken, the block is taken

    unsigned char veracityCheckerNodes[block_size]; // so this is for passing ablock side, I know the file descriptor is -1 block side
    memset(veracityCheckerNodes, 0, block_size);
    memset(freeList, 'f', block_size);
    this->disk->read(0, veracityCheckerNodes);
    this->disk->read(1, freeList);
    bool deleteCheck = false;

    for (int i = 0; i < block_size; i++)
    {
        /* code */
        int temp = veracityCheckerNodes[i];
        if ( temp == _file_id+1 )
        {

            deleteCheck = true;
            veracityCheckerNodes[i] = 0; //  delete clear out
        }
    }
    if (deleteCheck == false)
    {
        Console::puts(" no file to delete breaking \n");
        assert(false);
    }
    // kill freelist postion

    // invalidate inode

    for (int i = 0; i < MAX_INODES; i++)
    {
        if (veracityCheckerNodes[i] - 1 == _file_id)
        {
            Console::puts(" do I exist ? \n");Console::puti(i); Console::puts(" dead spcae \n ");

            inodeList[i]->blockNumberFile = -1; // first block to start
            inodeList[i]->fileId = 0;
            veracityCheckerNodes[i] = inodeList[i]->blockNumberFile;
            break;
        }
        // veracityCheckerNodes[i] = inodeList[i]->blockNumberFile;
    }      


    freeList[_file_id+1] = 'f'; // this is the position where is is free.
    this->disk->write(0, veracityCheckerNodes); // update directory
    this->disk->write(1, freeList); // update directory

    // for (int i = 0; i < block_size; i++)
    // {

    //          Console::puts(" i "); Console::puti(freeList[i]); Console::puts(" dead space \n ");


    // }



    return true;
}
