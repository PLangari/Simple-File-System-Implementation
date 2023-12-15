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

// Helper functions for sfs operation

void write_to_directoryTable(directoryEntry *directoryTable2)
{
    write_blocks(5, 2, directoryTable2);
}

void write_to_inodeTable(inode *inodeTable2)
{
    write_blocks(1, 4, inodeTable2);
}

void write_to_superblock(superblock *sb2)
{
    write_blocks(0, 1, sb2);
}

void write_to_freeBitMap(char *freeBitMap2)
{
    write_blocks(17486, 18, freeBitMap2);
}

/**
 * @brief      Allows read in a proper way (prevent buffer overflow).
*/
void read_from_disk(int start_address, int num_blocks, void *buffer, int data_size)
{
    char *temp = (char *)malloc(num_blocks * BLOCK_SIZE);
    memset(temp, 0, num_blocks * BLOCK_SIZE);
    read_blocks(start_address, num_blocks, temp);
    memcpy(buffer, temp, data_size);
    free(temp);
}

int ceilingDivision(int a, int b)
{
    return (a + b - 1) / b;
}

int min(int a, int b)
{
    return a < b ? a : b;
}

/**
 * @brief      Finds the amount of remainder bytes in the last block.
*/
int lastBlockRemainder(int rwptr)
{
    return BLOCK_SIZE - (rwptr % BLOCK_SIZE == 0 ? BLOCK_SIZE : rwptr % BLOCK_SIZE);
}

void print_bit_map()
{
    char *freeBitMap2 = (char *)malloc(NUM_DATA_BLOCKS * sizeof(char));
    read_from_disk(17486, 18, freeBitMap2, NUM_DATA_BLOCKS * sizeof(char));

    for (int i = 0; i < NUM_DATA_BLOCKS; ++i)
    {
        printf("%d", freeBitMap2[i]);
    }

    free(freeBitMap2);
}

void print_directoryTable()
{
    directoryEntry *directoryTable2 = (directoryEntry *)malloc(NUM_INODES * sizeof(directoryEntry));
    read_from_disk(5, 2, directoryTable2, NUM_INODES * sizeof(directoryEntry));
    for (int i = 0; i < NUM_INODES / 6; ++i)
    {
        printf("File %d:  iNode %d  name %s\n", i, directoryTable2[i].inode, directoryTable2[i].name);
    }

    free(directoryTable2);
}

void print_inodeTable()
{
    inode *inodeTable2 = (inode *)malloc(NUM_INODES * sizeof(inode));
    read_from_disk(1, 4, inodeTable2, (int)sizeof(inode) * NUM_INODES);
    for (int i = 0; i < NUM_INODES / 6; ++i)
    {
        printf("inode %d:  free %d  size %d  blocks %d %d %d %d %d %d %d %d %d %d %d  indirect %d\n", i, inodeTable2[i].free, inodeTable2[i].size, inodeTable2[i].blocks[0], inodeTable2[i].blocks[1], inodeTable2[i].blocks[2], inodeTable2[i].blocks[3], inodeTable2[i].blocks[4], inodeTable2[i].blocks[5], inodeTable2[i].blocks[6], inodeTable2[i].blocks[7], inodeTable2[i].blocks[8], inodeTable2[i].blocks[9], inodeTable2[i].blocks[10], inodeTable2[i].blocks[11], inodeTable2[i].indirect);
    }

    free(inodeTable2); // HERE'S THE PROBLEM
}

void print_superblock()
{
    superblock sb;
    read_from_disk(0, 1, &sb, (int)sizeof(superblock));
    printf("magic: %d\n", sb.magic);
    printf("block_size: %d\n", sb.block_size);
    printf("file_system_size: %d\n", sb.file_system_size);
    printf("inode_table_length: %d\n", sb.inode_table_length);
    printf("root_directory_inode: %d\n", sb.root_directory_inode);
}