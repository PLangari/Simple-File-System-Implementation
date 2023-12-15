#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>    // Boolean data type
#include <stdio.h>      // Standard Input/Output functions
#include <stdlib.h>     // General purpose functions and memory management
#include <string.h>     // String manipulation functions

#define BLOCK_SIZE 1024
#define MAX_FILES 64
#define NUM_INODES 65
#define MAX_OPEN_FILES 10

#define MAXFILENAME 20
#define NUM_DATA_BLOCKS 17503

typedef struct {
    int magic;
    int block_size;
    int file_system_size;
    int inode_table_length;
    int root_directory_inode;
} superblock;

typedef struct {
    // index of inode in inode table
    int inode;
    char name[MAXFILENAME];
} directoryEntry;

// directory table
typedef struct {
    bool free;
    int size;
    int blocks[12];
    int indirect;
} inode;

typedef struct {
    bool free;
    // index of inode in inode table
    int inode;
    // read/write pointer
    int rwptr;
} fileDescriptor;

// inode table
extern inode inodeTable[NUM_INODES];

// free bit map
extern char freeBitMap[NUM_DATA_BLOCKS];

// directory table
extern directoryEntry directoryTable[NUM_INODES];

// superblock
extern superblock sb;

// file descriptor table
extern fileDescriptor fileDescriptorTable[MAX_OPEN_FILES];

#endif