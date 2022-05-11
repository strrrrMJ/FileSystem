#include "FileSystem.h"
#include "FileManager.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>

using namespace std;

SuperBlock g_superblock;

void FileSystem::Init_Some_Dir()
{
    vector<string> list = {"Users", "bin", "dev", "opt", "sbin", "tmp", "usr", "var"};
    for (auto i : list)
    {
        vector<string> path = {"root"};
        path.push_back(i);
        FileManager::Create_Dir(path);
    }
}

void FileSystem::Load_SuperBlock()
{
    DiskDriver::Read(SUPER_BLOCK_INDEX * BLOCK_SIZE, (char *)&g_superblock, sizeof(SuperBlock));
}

void FileSystem::Store_SuperBlock()
{
    DiskDriver::Write(SUPER_BLOCK_INDEX * BLOCK_SIZE, (char *)&g_superblock, sizeof(SuperBlock));
}

void FileSystem::Init_SuperBlock()
{

    g_superblock.s_inode_num = INODE_NUM;
    g_superblock.s_free_inode_num = INODE_NUM;

    g_superblock.s_total_block_num = TOTAL_BLOCK_NUM;
    g_superblock.s_free_block_num = TOTAL_BLOCK_NUM;

    g_superblock.s_nfree = 1; // Init with 1
    g_superblock.s_free[0] = -1;
}

void FileSystem::Init_All_Free_Blocks()
{
    unsigned int *stack = g_superblock.s_free;
    unsigned int &p_stk = g_superblock.s_nfree;
    int p_data = DATA_BLOCK_START_INDEX + 1;
    // std::cout << "Enter init all free blocks" << std::endl;
    while (p_data < TOTAL_BLOCK_NUM)
    {
        if (p_stk < MAX_NFREE)
        {
            stack[p_stk++] = p_data++;
        }
        else
        {
            DiskDriver::Write(p_data * BLOCK_SIZE, (char *)stack, sizeof(unsigned int) * MAX_NFREE);
            stack[0] = p_data++;
            g_superblock.s_nfree = 1;
        }
    }
}

void FileSystem::Init_BitMap()
{
    unsigned int bitmap[INODE_NUM] = {1};
    DiskDriver::Write(BITMAP_START_INDEX * BLOCK_SIZE, (char *)bitmap, BITMAP_SIZE);
}

void FileSystem::Init_Root()
{
    Inode inode;
    inode.i_size = 2;
    inode.i_mode = 1;
    inode.i_number = 0;
    inode.i_count = 0;
    inode.i_permission = 0;
    inode.i_uid = 0;
    inode.i_gid = 0;
    time_t t;
    time(&t);
    inode.i_time = t;

    inode.i_addr[0] = DATA_BLOCK_START_INDEX;

    Directory dir;
    dir.d_inode_num[0] = 0;
    dir.d_inode_num[1] = 0;
    strcpy(dir.d_filename[0], ".");
    strcpy(dir.d_filename[1], "..");
    DiskDriver::Write(inode.i_addr[0] * BLOCK_SIZE, (char *)&dir, sizeof(Directory));

    FileSystem::Store_Inode(inode, 0);
}

void FileSystem::Format_Disk()
{
    if (DiskDriver::Exists() == false)
    {
        throw "DISK DOES NOT EXIST";
    }
    Init_SuperBlock();

    Init_All_Free_Blocks();

    Init_BitMap();

    Init_Root();

    Store_SuperBlock();

    Init_Some_Dir();
}

unsigned int FileSystem::Allocate_Block()
{
    unsigned int res;

    unsigned int *stack = g_superblock.s_free;
    unsigned int &p_stk = g_superblock.s_nfree;

    if (p_stk > 1)
    {
        res = stack[--p_stk];
    }
    else
    {
        res = stack[0];
        DiskDriver::Read(res * BLOCK_SIZE, (char *)stack, sizeof(unsigned int) * MAX_NFREE);
    }

    Store_SuperBlock();

    return res;
}

void FileSystem::Free_Block(unsigned int block_num)
{
    unsigned int *stack = g_superblock.s_free;
    unsigned int &p_stk = g_superblock.s_nfree;

    if (p_stk < MAX_NFREE)
    {
        stack[p_stk++] = block_num;
    }
    else
    {
        DiskDriver::Write(block_num * BLOCK_SIZE, (char *)stack, sizeof(unsigned int) * MAX_NFREE);
        stack[0] = block_num;
        p_stk = 1;
    }

    Store_SuperBlock();
}

unsigned int FileSystem::Allocate_Inode()
{
    Load_SuperBlock();
    if (g_superblock.s_free_inode_num == 0)
    {
        throw "OUT OF INODE!";
    }
    else
    {
        unsigned int bitmap[INODE_NUM];
        DiskDriver::Read(BITMAP_START_INDEX * BLOCK_SIZE, (char *)bitmap, BITMAP_SIZE);
        for (unsigned int i = 0; i < INODE_NUM; i++)
        {
            if (bitmap[i] == 0)
            {
                g_superblock.s_free_inode_num--;
                unsigned int tmp = 1;
                DiskDriver::Write(BITMAP_START_INDEX * BLOCK_SIZE + i * sizeof(unsigned int), (char *)&tmp, sizeof(unsigned int));
                Store_SuperBlock();
                return i;
            }
        }
    }
    return -1; // 正常情况下, 不会在这里返回
}

void FileSystem::Free_Inode(unsigned int inode_num)
{
    g_superblock.s_free_inode_num++;
    unsigned int tmp = 0;
    DiskDriver::Write(BITMAP_START_INDEX * BLOCK_SIZE + inode_num * sizeof(unsigned int), (char *)&tmp, sizeof(unsigned int));
    Store_SuperBlock();
}

void FileSystem::Load_Inode(Inode &inode, unsigned int inode_pos)
{
    DiskDriver::Read(INODE_START_INDEX * BLOCK_SIZE + inode_pos * INODE_SIZE, (char *)&inode, sizeof(Inode));
}

void FileSystem::Store_Inode(Inode &inode, unsigned int inode_pos)
{
    DiskDriver::Write(INODE_START_INDEX * BLOCK_SIZE + inode_pos * INODE_SIZE, (char *)&inode, sizeof(Inode));
}

void FileSystem::Boot()
{
    Load_SuperBlock();
}