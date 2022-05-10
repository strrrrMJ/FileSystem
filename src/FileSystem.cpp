#include "FileSystem.h"

#include "iostream"

void FileSystem::Load_SuperBlock()
{
    DiskDriver::Read(SUPER_BLOCK_INDEX * BLOCK_SIZE, (char *)&(this->super_block), sizeof(SuperBlock));
}

void FileSystem::Store_SuperBlock()
{
    DiskDriver::Write(SUPER_BLOCK_INDEX * BLOCK_SIZE, (char *)&(this->super_block), sizeof(SuperBlock));
}

void FileSystem::Init_SuperBlock()
{

    super_block.s_inode_num = INODE_NUM;
    super_block.s_free_inode_num = INODE_NUM;

    super_block.s_total_block_num = TOTAL_BLOCK_NUM;
    super_block.s_free_block_num = TOTAL_BLOCK_NUM;

    super_block.s_nfree = 1; // Init with 1
    super_block.s_free[0] = -1;
}

void FileSystem::Init_All_Free_Blocks()
{
    unsigned int *stack = super_block.s_free;
    unsigned int &p_stk = super_block.s_nfree;
    int p_data = DATA_BLOCK_START_INDEX;
    std::cout << "Enter init all free blocks" << std::endl;
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
            super_block.s_nfree = 1;
        }
    }
    // std::cout << "p_data: " << p_data << std::endl;
    // std::cout << "s_nfree: " << p_stk << std::endl;
    // std::cout << TOTAL_BLOCK_NUM << std::endl;
    // std::cout << TOTAL_BLOCK_NUM - DATA_BLOCK_START_INDEX << std::endl;
    // std::cout << DATA_BLOCK_NUM << std::endl;
    // std::cout << DATA_BLOCK_NUM % BLOCK_SIZE << std::endl;
}

void FileSystem::Init_BitMap()
{
    unsigned int bitmap[INODE_NUM] = {0};
    DiskDriver::Write(BITMAP_START_INDEX * BLOCK_SIZE, (char *)bitmap, BITMAP_SIZE);
}

void FileSystem::Format_Disk()
{
    if (DiskDriver::Exists() == false)
    {
        throw "DISK DOES NOT EXIST";
    }
    this->Init_SuperBlock();

    this->Init_All_Free_Blocks();

    this->Init_BitMap();

    this->Store_SuperBlock();
}

unsigned int FileSystem::Allocate_Block()
{
    unsigned int res;

    unsigned int *stack = super_block.s_free;
    unsigned int &p_stk = super_block.s_nfree;

    if (p_stk > 1)
    {
        res = stack[--p_stk];
    }
    else
    {
        res = stack[0];
        DiskDriver::Read(res * BLOCK_SIZE, (char *)stack, sizeof(unsigned int) * MAX_NFREE);
    }

    this->Store_SuperBlock();

    return res;
}

void FileSystem::Free_Block(unsigned int block_num)
{
    unsigned int *stack = super_block.s_free;
    unsigned int &p_stk = super_block.s_nfree;

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
}

unsigned int FileSystem::Allocate_Inode()
{
    Load_SuperBlock();
    if (super_block.s_free_inode_num == 0)
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
                super_block.s_free_inode_num--;
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
    super_block.s_free_inode_num++;
    unsigned int tmp = 0;
    DiskDriver::Write(BITMAP_START_INDEX * BLOCK_SIZE + inode_num * sizeof(unsigned int), (char *)&tmp, sizeof(unsigned int));
    Store_SuperBlock();
}