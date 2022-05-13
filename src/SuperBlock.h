#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

static const unsigned int MAX_NFREE = 100;

class SuperBlock
{
public:
    unsigned short s_inode_num; // Total number of inodes
    unsigned short s_free_inode_num;
    unsigned int s_total_block_num; // Total number of blocks
    unsigned int s_free_block_num;
    unsigned int s_nfree;           // The stack top pointer
    unsigned int s_free[MAX_NFREE]; // The stack, in other word, the free block index table
};

#endif