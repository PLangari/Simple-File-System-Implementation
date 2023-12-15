#ifndef HELPER_H
#define HELPER_H

////////////////////////////////////////
// Functions that interact with disk. //
////////////////////////////////////////

void initialize_superblock();
void initialize_inodeTable();
void initialize_directoryTable();
void initialize_freeBitMap();
void write_to_lastBlock(inode inode, char *buf, int size, int inodeIndex);
void read_from_lastBlock(inode inode, char *buf, int length, int inodeIndex, int rdptr);

#endif // HELPER_H