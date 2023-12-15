#ifndef sfs_dir_H
#define sfs_dir_H

void initialize_fileDescriptorTable();
int find_free_fileDescriptor();
int find_free_directoryEntry();
int find_directoryEntry(char* filename);
int find_free_inode();
int find_free_dataBlock();
void init_inode(int inode);

#endif  // sfs_dir_H