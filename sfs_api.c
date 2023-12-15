#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>

#include "disk_emu.h"
#include "sfs_api.h"
#include "types.h"
#include "sfs_inode.h"
#include "sfs_dir.h"
#include "test.h"
#include "utils.h"

// inode table
inode inodeTable[NUM_INODES];

// free bit map: blocks 7 - 17485
char freeBitMap[NUM_DATA_BLOCKS];

directoryEntry directoryTable[NUM_INODES];

fileDescriptor fileDescriptorTable[MAX_OPEN_FILES];

superblock sb;

void mksfs(int); // done

int sfs_getnextfilename(char *);

int sfs_getfilesize(const char *);

int sfs_fopen(char *); // done

int sfs_fclose(int); // done

int sfs_fwrite(int, const char *, int); // done (with bugs)

int sfs_fread(int, char *, int); // done (with bugs)

int sfs_fseek(int, int); // done (with little bugs hopefully)

int sfs_remove(char *); // need to do

void mksfs(int fresh)
{
    if (fresh == 1)
    {
        init_fresh_disk("sfs_disk.disk", BLOCK_SIZE, 17505);

        // initialize the disk
        initialize_superblock();
        initialize_inodeTable();
        initialize_directoryTable();
        initialize_fileDescriptorTable();

        // range of free data blocks:
        // 7 - 17485

        // test initialization (if it fails it would just print the failure)
        test_init_fs();
    }
    else
    {
        init_disk("sfs_disk.disk", BLOCK_SIZE, 17505);
        read_from_disk(0, 1, &sb, (int)sizeof(superblock));
        read_blocks(1, 4, inodeTable);
        read_blocks(5, 2, directoryTable);
        read_blocks(17486, 18, freeBitMap);
    }
}

int sfs_fopen(char *filename)
{
    // look for the file in the directory table
    int directoryEntry = find_directoryEntry(filename);

    // Create file if it does not exist
    if (directoryEntry == -1)
    {
        int freeInode = find_free_inode();
        directoryEntry = find_free_directoryEntry();

        // intialize inode and directory entry
        init_inode(freeInode);
        directoryTable[directoryEntry].inode = freeInode;
        strcpy(directoryTable[directoryEntry].name, filename);

        // update the disk
        write_to_inodeTable(inodeTable);
        write_to_directoryTable(directoryTable);
    }

    // find free file descriptor and add file to FDT
    int freeFileDescriptor = find_free_fileDescriptor();
    fileDescriptorTable[freeFileDescriptor].free = 0;
    fileDescriptorTable[freeFileDescriptor].inode = directoryTable[directoryEntry].inode;
    fileDescriptorTable[freeFileDescriptor].rwptr = inodeTable[directoryTable[directoryEntry].inode].size;

    return freeFileDescriptor;
}

int sfs_fclose(int fileID)
{
    // check if file is closed already
    if (fileDescriptorTable[fileID].free == 1)
    {
        return -1;
    }

    // remove file from FDT
    fileDescriptorTable[fileID].free = 1;
    fileDescriptorTable[fileID].inode = -1;
    fileDescriptorTable[fileID].rwptr = -1;
}

int sfs_fwrite(int fileID, const char *buffer, int length)
{
    // get the opened file's inode
    fileDescriptor fd = fileDescriptorTable[fileID];
    inode node = inodeTable[fd.inode];

    // get the remainder of the last block
    int remainder = lastBlockRemainder(fd.rwptr);

    // get the number of blocks to allocate, as well as where to allocate them from
    int blocksToAllocate = ceilingDivision((length - remainder), BLOCK_SIZE);
    int startBlock = ceilingDivision(fd.rwptr, BLOCK_SIZE);
    int currBlock = startBlock;

    // fill up the last block if there's a remainder
    if (remainder != 0)
    {
        write_to_lastBlock(node, buffer, remainder, currBlock);
    }

    // if start block is less than 12: write to DIRECT blocks first
    if (startBlock < 12)
    {
        // We stop when (1) there's no more DIRECT blocks
        // OR (2) we are done allocating/writing into the blocks we needed to
        while (currBlock < min(startBlock + blocksToAllocate, 12))
        {
            // find free block
            int freeBlock = find_free_dataBlock();

            // update FBM and iNode's pointers
            freeBitMap[freeBlock] = 1;
            node.blocks[currBlock] = freeBlock;

            // write to disk
            write_blocks(freeBlock, 1, buffer);

            // advance the buffer pointer by a block
            buffer += BLOCK_SIZE;

            currBlock++;
        }

        blocksToAllocate = blocksToAllocate - (currBlock - startBlock);
    }

    // if there are still blocks to allocate: use indirect blocks
    if (blocksToAllocate > 0)
    {
        // get the indirect block
        int indirectBlock = node.indirect;

        // if there is no indirect block, allocate one
        if (indirectBlock == -1)
        {
            indirectBlock = find_free_dataBlock();
            freeBitMap[indirectBlock] = 1;
            node.indirect = indirectBlock;
        }

        // indirect block buffer: table of pointers to blocks
        int indirectBlockBuffer[BLOCK_SIZE / sizeof(int)];
        read_from_disk(indirectBlock, 1, indirectBlockBuffer, BLOCK_SIZE);

        for (int i = 0; i < min(blocksToAllocate, BLOCK_SIZE / sizeof(int)); i++)
        {
            int freeBlock = find_free_dataBlock();

            freeBitMap[freeBlock] = 1;
            indirectBlockBuffer[i] = freeBlock;

            // write to disk
            write_blocks(freeBlock, 1, buffer);

            // advance buffer pointer by a block size
            buffer += BLOCK_SIZE;
        }

        // update indirect block on the disk
        write_blocks(indirectBlock, 1, indirectBlockBuffer);

        // update inode's indirect block
        node.indirect = indirectBlock;
    }

    // update inode
    node.size += length;
    inodeTable[fd.inode] = node;
    fd.rwptr = node.size;

    // update disk
    write_to_inodeTable(inodeTable);
    write_to_freeBitMap(freeBitMap);

    return length;
}

int sfs_fread(int fileID, char *buffer, int length)
{
    // get the opened file
    fileDescriptor fd = fileDescriptorTable[fileID];
    inode node = inodeTable[fd.inode];
    int size = length;

    // start reading (copying into buffer) from remainder of last block
    int remainder = lastBlockRemainder(fd.rwptr);
    int blocksToRead = ceilingDivision((size - remainder), BLOCK_SIZE);
    if (remainder != 0)
    {
        // if the rd/wr pointer starts in the middle of a block, then increment the total number of reads required
        blocksToRead++;
    }
    int startBlock = ceilingDivision(fd.rwptr, BLOCK_SIZE);
    int currBlock = startBlock;

    // just like read: start from the remainder of last block
    if (remainder != 0)
    {
        read_from_lastBlock(node, buffer, remainder, currBlock, size);
    }

    if (startBlock < 12)
    {
        // We stop reading when (1) there's no more DIRECT blocks
        // OR (2) there's no more blocks to read
        while (currBlock < min(startBlock + blocksToRead, 12))
        {
            read_from_disk(
                node.blocks[currBlock],
                1,
                buffer,
                // this truncates reads into BLOCK_SIZEs (with last block smaller than BLOCK_SIZE => remainder)
                size > BLOCK_SIZE ? BLOCK_SIZE : size % BLOCK_SIZE);

            buffer += BLOCK_SIZE;
            size -= BLOCK_SIZE;
            currBlock++;
        }

        blocksToRead = blocksToRead - (currBlock - startBlock);
    }


    // if there are still blocks to read: read indirect block and read blocks
    if (blocksToRead > 0)
    {
        int indirectBlock = node.indirect;

        // indirect block buffer: table of pointers to blocks
        int indirectBlockBuffer[BLOCK_SIZE / sizeof(int)];
        read_from_disk(indirectBlock, 1, indirectBlockBuffer, BLOCK_SIZE);

        // read blocks
        for (int i = 0; i < min(blocksToRead, BLOCK_SIZE / sizeof(int)); i++)
        {
            read_from_disk(
                indirectBlockBuffer[i],
                1,
                buffer,
                size > BLOCK_SIZE ? BLOCK_SIZE : size % BLOCK_SIZE);

            buffer += BLOCK_SIZE;
            size -= BLOCK_SIZE;
        }
    }

    return length;
}

int sfs_fseek(int fileID, int pointer)
{
    // check if file is closed already
    if (fileDescriptorTable[fileID].free == 1)
    {
        return -1;
    }

    // check if pointer is valid (within file size)
    if (pointer > inodeTable[fileDescriptorTable[fileID].inode].size)
    {
        return -1;
    }

    fileDescriptorTable[fileID].rwptr = pointer;
    return 0;
}