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

#include "sfs_dir.h"
#include "disk_emu.h"
#include "types.h"
#include "sfs_inode.h"
#include "utils.h"
#include "sfs_api.h"

// Functions that handle cache stuff.

void initialize_fileDescriptorTable()
{
    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        fileDescriptorTable[i].free = 1;
        fileDescriptorTable[i].inode = -1;
        fileDescriptorTable[i].rwptr = -1;
    }
}

int find_free_fileDescriptor()
{
    for (int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (fileDescriptorTable[i].free == 1)
        {
            return i;
        }
    }

    return -1;
}

int find_free_directoryEntry()
{
    for (int i = 0; i < NUM_INODES; i++)
    {
        if (directoryTable[i].name == NULL || strcmp(directoryTable[i].name, "") == 0)
        {
            return i;
        }
    }

    return -1;
}

int find_directoryEntry(char* filename)
{
    for (int i = 0; i < NUM_INODES; i++)
    {
        if (strcmp(directoryTable[i].name, filename) == 0)
        {
            return i;
        }
    }

    return -1;
}

int find_free_inode()
{
    for (int i = 0; i < NUM_INODES; i++)
    {
        inode curr = inodeTable[i];
        if (curr.free == 1)
        {
            return i;
        }
    }

    return -1;
}

int find_free_dataBlock()
{
    for (int i = 0; i < NUM_DATA_BLOCKS; i++)
    {
        if (freeBitMap[i] == 0)
        {
            return i + 5;
        }
    }

    return -1;
}

void init_inode(int inode)
{
    inodeTable[inode].free = 1;
    inodeTable[inode].size = 0;
    inodeTable[inode].indirect = -1;
    for (int i = 0; i < 12; ++i)
    {
        inodeTable[inode].blocks[i] = -1;
    }
}
