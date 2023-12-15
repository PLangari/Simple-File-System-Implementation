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
#include "types.h"
#include "sfs_inode.h"

int test_init_fs()
{
    superblock sb2;
    inode* inodeTable2 = (inode*) malloc(NUM_INODES * sizeof(inode));
    directoryEntry* directoryTable2 = (directoryEntry*) malloc(NUM_INODES * sizeof(directoryEntry));
    char* freeBitMap2 = (char*) malloc(NUM_DATA_BLOCKS * sizeof(char));
    
    read_from_disk(0, 1, &sb2, (int) sizeof(superblock));

    if (sb2.magic != sb.magic)
    {
        printf("magic number is wrong\n");
        return -1;
    }

    if (sb2.block_size != sb.block_size)
    {
        printf("block size is wrong\n");
        return -1;
    }

    if (sb2.file_system_size != sb.file_system_size)
    {
        printf("file system size is wrong\n");
        return -1;
    }

    if (sb2.inode_table_length != sb.inode_table_length)
    {
        printf("inode table length is wrong\n");
        return -1;
    }

    if (sb2.root_directory_inode != sb.root_directory_inode)
    {
        printf("root directory inode is wrong\n");
        return -1;
    }

    read_from_disk(1, 4, inodeTable2, (int) NUM_INODES * sizeof(inode));

    for (int i = 0; i < NUM_INODES; ++i)
    {
        if (inodeTable2[i].free != inodeTable[i].free)
        {
            printf("inode %d free is wrong\n", i);
            return -1;
        }
    }

    read_from_disk(5, 2, directoryTable2, (int) NUM_INODES * sizeof(directoryEntry));

    for (int i = 0; i < NUM_INODES; ++i)
    {
        if (directoryTable2[i].inode != directoryTable[i].inode)
        {
            printf("directory entry %d inode is wrong\n", i);
            return -1;
        }
    }

    read_from_disk(17486, 18, freeBitMap2, (int) NUM_DATA_BLOCKS * sizeof(char));

    for (int i = 0; i < NUM_DATA_BLOCKS; ++i)
    {
        if (freeBitMap2[i] != freeBitMap[i])
        {
            printf("free bit map %d is wrong\n", i);
            return -1;
        }
    }

    // free memory
    free(inodeTable2);
    free(directoryTable2);
    free(freeBitMap2);

    // all is good, memory is initialized correctly
    //printf("memory is initialized correctly\n");

    // DOCUMENTATION:
    // BLOCK 0: superblock
    // BLOCK 1-4: inode table
    // BLOCK 5-6: root directory
    // BLOCK 7-17485: data blocks
    // BLOCK 17486-17503: free bit map

    return 1;
}