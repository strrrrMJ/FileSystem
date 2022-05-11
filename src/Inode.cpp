#include "Inode.h"
#include "FileSystem.h"

unsigned int Inode::Offset_To_Index(unsigned int offset)
{
    // offset to a1 a2 a3 a4
    //  calculate physical number of this block
    unsigned int block_cnt = offset / BLOCK_SIZE;
    if (block_cnt < DIRECT_PTR_NUM)
    {
        return i_addr[block_cnt];
    }
    else if (block_cnt < DIRECT_PTR_NUM + SEC_PTR_NUM * PTR_IN_BLOCK_NUM)
    {
        unsigned int first_index = (block_cnt - DIRECT_PTR_NUM) / PTR_IN_BLOCK_NUM + DIRECT_PTR_NUM;
        unsigned int second_index = (block_cnt - DIRECT_PTR_NUM) % PTR_IN_BLOCK_NUM;
        unsigned int index_table[PTR_IN_BLOCK_NUM];
        DiskDriver::Read(i_addr[first_index] * BLOCK_SIZE, (char *)index_table, BLOCK_SIZE);
        return index_table[second_index];
    }
    else if (block_cnt < DIRECT_PTR_NUM + SEC_PTR_NUM * PTR_IN_BLOCK_NUM + TER_PTR_NUM * PTR_IN_BLOCK_NUM * PTR_IN_BLOCK_NUM)
    {
        unsigned int first_index = (block_cnt - DIRECT_PTR_NUM - SEC_PTR_NUM * PTR_IN_BLOCK_NUM) / (PTR_IN_BLOCK_NUM * PTR_IN_BLOCK_NUM) + DIRECT_PTR_NUM + SEC_PTR_NUM;
        unsigned int second_index = (block_cnt - DIRECT_PTR_NUM - SEC_PTR_NUM * PTR_IN_BLOCK_NUM - PTR_IN_BLOCK_NUM * PTR_IN_BLOCK_NUM * (first_index - DIRECT_PTR_NUM - SEC_PTR_NUM)) / PTR_IN_BLOCK_NUM;
        unsigned int third_index = (block_cnt - DIRECT_PTR_NUM - SEC_PTR_NUM * PTR_IN_BLOCK_NUM) % PTR_IN_BLOCK_NUM;
        unsigned int index_table[PTR_IN_BLOCK_NUM];
        DiskDriver::Read(i_addr[first_index] * BLOCK_SIZE, (char *)index_table, BLOCK_SIZE);
        unsigned int second_block_no = index_table[second_index];
        DiskDriver::Read(second_block_no * BLOCK_SIZE, (char *)index_table, BLOCK_SIZE);
        return index_table[third_index];
    }
    else
    {
        throw "INVALID ACCESS!";
        return -1;
    }
}

void Inode::link(unsigned int x)
{
}