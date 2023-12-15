#ifndef UTILS_H
#define UTILS_H

////////////////////////////////////////
// Helper functions for sfs operation //
////////////////////////////////////////

void write_to_directoryTable(directoryEntry *directoryTable2);
void write_to_inodeTable(inode *inodeTable2);
void write_to_superblock(superblock *sb2);
void write_to_freeBitMap(char *freeBitMap2);
void read_from_disk(int start_address, int num_blocks, void *buffer, int data_size);
int ceilingDivision(int a, int b);
int min(int a, int b);
int lastBlockRemainder(int rwptr);
void print_bit_map();
void print_directoryTable();
void print_inodeTable();
void print_superblock();

#endif // UTILS_H
