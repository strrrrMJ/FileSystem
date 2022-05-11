#include "Inode.h"
#include "FileSystem.h"
#include <cmath>
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

void Inode::Link(unsigned int physical_block_num)
{
    // calculate how many physical blocks this file will occupy
    unsigned int block_cnt = ceil(i_size / BLOCK_SIZE) + 1;
    if (block_cnt < DIRECT_PTR_NUM)
    {
        // level 1
        i_addr[block_cnt] = physical_block_num;
    }
    else if (block_cnt < DIRECT_PTR_NUM + SEC_PTR_NUM * PTR_IN_BLOCK_NUM)
    {
        // level 2
        unsigned int first_index = (block_cnt - DIRECT_PTR_NUM) / PTR_IN_BLOCK_NUM + DIRECT_PTR_NUM;
        unsigned int second_index = (block_cnt - DIRECT_PTR_NUM) % PTR_IN_BLOCK_NUM;

        // allocate new block to store level2 index table
        if (second_index == 0)
        {
            unsigned int new_level2_block_num = FileSystem::Allocate_Block();

            // link to level1 index table
            i_addr[first_index] = new_level2_block_num;
        }
        // read level2 index table from disk
        unsigned int level2_index_table[PTR_IN_BLOCK_NUM];
        DiskDriver::Read(i_addr[first_index] * BLOCK_SIZE, (char *)level2_index_table, BLOCK_SIZE);

        // change the index table
        level2_index_table[second_index] = physical_block_num;

        // store level2 index table
        DiskDriver::Write(i_addr[first_index] * BLOCK_SIZE, (char *)level2_index_table, BLOCK_SIZE);
    }
    else if (block_cnt < DIRECT_PTR_NUM + SEC_PTR_NUM * PTR_IN_BLOCK_NUM + TER_PTR_NUM * PTR_IN_BLOCK_NUM * PTR_IN_BLOCK_NUM)
    {
        // level 3
        unsigned int first_index = (block_cnt - DIRECT_PTR_NUM - SEC_PTR_NUM * PTR_IN_BLOCK_NUM) / (PTR_IN_BLOCK_NUM * PTR_IN_BLOCK_NUM) + DIRECT_PTR_NUM + SEC_PTR_NUM;
        unsigned int second_index = (block_cnt - DIRECT_PTR_NUM - SEC_PTR_NUM * PTR_IN_BLOCK_NUM - PTR_IN_BLOCK_NUM * PTR_IN_BLOCK_NUM * (first_index - DIRECT_PTR_NUM - SEC_PTR_NUM)) / PTR_IN_BLOCK_NUM;
        unsigned int third_index = (block_cnt - DIRECT_PTR_NUM - SEC_PTR_NUM * PTR_IN_BLOCK_NUM) % PTR_IN_BLOCK_NUM;

        if (third_index == 0)
        {
            unsigned int new_level3_block_num = FileSystem::Allocate_Block();

            // link to level2 index table
            if (second_index == 0)
            {
                unsigned int new_level2_block_num = FileSystem::Allocate_Block();

                // link to level1 index table
                i_addr[first_index] = new_level2_block_num;
            }
            unsigned level2_index_table[PTR_IN_BLOCK_NUM];
            DiskDriver::Read(i_addr[first_index] * BLOCK_SIZE, (char *)level2_index_table, BLOCK_SIZE);
            level2_index_table[second_index] = new_level3_block_num;
            DiskDriver::Write(i_addr[first_index] * BLOCK_SIZE, (char *)level2_index_table, BLOCK_SIZE);
        }
        unsigned level2_index_table[PTR_IN_BLOCK_NUM];
        DiskDriver::Read(i_addr[first_index] * BLOCK_SIZE, (char *)level2_index_table, BLOCK_SIZE);

        unsigned level3_index_table[PTR_IN_BLOCK_NUM];
        DiskDriver::Read(level2_index_table[second_index] * BLOCK_SIZE, (char *)level3_index_table, BLOCK_SIZE);
        level3_index_table[second_index] = physical_block_num;
        DiskDriver::Write(level2_index_table[second_index] * BLOCK_SIZE, (char *)level3_index_table, BLOCK_SIZE);
    }
    else
    {
        // file exceeds its maximum size
        // it cannot be linked to any new physical block anymore
        throw "INVALID ACCESS!";
    }
}
