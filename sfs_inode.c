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
#include "utils.h"
#include "sfs_dir.h"

// Functions that interact with disk. 

void initialize_superblock()
{
    sb.magic = 42096;
    sb.block_size = BLOCK_SIZE;
    sb.file_system_size = NUM_DATA_BLOCKS;
    sb.inode_table_length = NUM_INODES;
    sb.root_directory_inode = 0;

    // write the superblock to disk
    write_to_superblock(&sb);
}

void initialize_inodeTable()
{
    inodeTable[0].free = false;
    inodeTable[0].size = 3705;
    inodeTable[0].blocks[0] = 2;
    inodeTable[0].blocks[1] = 3;
    inodeTable[0].blocks[2] = 4;
    inodeTable[0].blocks[3] = 5;
    for (int i = 4; i < 12; ++i)
    {
        inodeTable[0].blocks[i] = -1;
    }
    inodeTable[0].indirect = -1;

    for (int i = 1; i < NUM_INODES; ++i)
    {
        init_inode(i);
    }

    // write the inode table to disk
    write_to_inodeTable(inodeTable);
}

void initialize_directoryTable()
{
    // initialize the root directory
    for (int i = 0; i < NUM_INODES; ++i)
    {
        directoryTable[i].inode = -1;
        strcpy(directoryTable[i].name, "");
    }

    directoryEntry root;
    root.inode = 0;
    // root.name = "/";
    strcpy(root.name, "/");
    // set first entry to root
    memcpy(&directoryTable[0], &root, sizeof(directoryEntry));

    // write the root directory to disk
    write_to_directoryTable(directoryTable);
}

void initialize_freeBitMap()
{
    // initialize the free bit map
    memset(freeBitMap, 0, NUM_DATA_BLOCKS);

    // update freebitmap
    freeBitMap[0] = 1;
    freeBitMap[1] = 1;

    // write the free bit map to disk
    write_to_freeBitMap(freeBitMap);
}

/**
 * @brief      Fills up the remaining bytes of last block of the file.
*/
void write_to_lastBlock(inode inode, char *buf, int size, int inodeIndex)
{
    // read last block
    char *lastBlock = (char *)malloc(BLOCK_SIZE);
    read_blocks(inode.blocks[inodeIndex], 1, lastBlock);

    // write to last block
    memcpy(&lastBlock[BLOCK_SIZE - size], buf, size);
    write_blocks(inode.blocks[inodeIndex], 1, lastBlock);

    // update buffer
    buf += size;
}

/**
 * @brief      Reads (copies into buffer) the remaining bytes of last block of the file.
*/
void read_from_lastBlock(inode inode, char *buf, int length, int inodeIndex, int rdptr)
{
    char *lastBlock = (char *)malloc(BLOCK_SIZE);
    read_blocks(inode.blocks[inodeIndex], 1, lastBlock);

    memcpy(buf, &lastBlock[BLOCK_SIZE - length], length);

    buf += length;
    rdptr -= length;
}
