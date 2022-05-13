#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "SuperBlock.h"
#include "BufferManager.h"
#include "Inode.h"
#include "Directory.h"
#include "User.h"
#include <string>

// These const values discribe Disk Layout
static const unsigned int SUPER_BLOCK_INDEX = 0;
static const unsigned int SUPER_BLOCK_SIZE = 512;

static const unsigned int INODE_NUM = 256;
static const unsigned int INODE_SIZE = 64;

static const unsigned int BITMAP_START_INDEX = SUPER_BLOCK_INDEX + (SUPER_BLOCK_SIZE - 1) / BLOCK_SIZE + 1;
static const unsigned int BITMAP_SIZE = sizeof(int) * INODE_NUM;

static const unsigned int INODE_START_INDEX = BITMAP_START_INDEX + (BITMAP_SIZE - 1) / BLOCK_SIZE + 1;

static const unsigned int DATA_BLOCK_START_INDEX = INODE_START_INDEX + (INODE_NUM * INODE_SIZE - 1) / BLOCK_SIZE + 1;
static const unsigned int DATA_BLOCK_NUM = 2 * 1024 * 128;

static const unsigned int TOTAL_BLOCK_NUM = (DATA_BLOCK_NUM + DATA_BLOCK_START_INDEX); // Almost 128MB
static const unsigned int DISK_SIZE = (TOTAL_BLOCK_NUM * BLOCK_SIZE) / (1024 * 1024);

class FileSystem
{
public:
    static void Load_SuperBlock();

    static void Store_SuperBlock();

    static void Init_SuperBlock();

    static void Init_All_Free_Blocks();

    static void Init_BitMap();

    static void Free_Inode(unsigned int inode_num);

    static unsigned int Allocate_Inode();

    static void Format_Disk();

    // Return the block_num
    static unsigned int Allocate_Block();

    static void Free_Block(unsigned int block_num);

    static void Load_Inode(Inode &inode, unsigned int inode_pos);

    static void Store_Inode(Inode &inode, unsigned int inode_pos);

    static void Init_Root();

    static void Boot();

    static void Init_Some_Dir();

    static void Init_Register();

    static User Check_User(std::string, std::string);

public:
    friend class FileManager;
};

#endif