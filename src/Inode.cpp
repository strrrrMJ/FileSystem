#include "Inode.h"
#include "FileSystem.h"
#include <cmath>
#include <iostream>
using namespace std;
unsigned int Inode::Offset_To_Index(unsigned int offset)
{
    // offset to a1 a2 a3 a4
    // calculate physical number of this block
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
        BufferManager::Read(i_addr[first_index], 0, (char *)index_table, BLOCK_SIZE);
        return index_table[second_index];
    }
    else if (block_cnt < DIRECT_PTR_NUM + SEC_PTR_NUM * PTR_IN_BLOCK_NUM + TER_PTR_NUM * PTR_IN_BLOCK_NUM * PTR_IN_BLOCK_NUM)
    {
        unsigned int first_index = (block_cnt - DIRECT_PTR_NUM - SEC_PTR_NUM * PTR_IN_BLOCK_NUM) / (PTR_IN_BLOCK_NUM * PTR_IN_BLOCK_NUM) + DIRECT_PTR_NUM + SEC_PTR_NUM;
        unsigned int second_index = (block_cnt - DIRECT_PTR_NUM - SEC_PTR_NUM * PTR_IN_BLOCK_NUM - PTR_IN_BLOCK_NUM * PTR_IN_BLOCK_NUM * (first_index - DIRECT_PTR_NUM - SEC_PTR_NUM)) / PTR_IN_BLOCK_NUM;
        unsigned int third_index = (block_cnt - DIRECT_PTR_NUM - SEC_PTR_NUM * PTR_IN_BLOCK_NUM) % PTR_IN_BLOCK_NUM;
        unsigned int index_table[PTR_IN_BLOCK_NUM];
        BufferManager::Read(i_addr[first_index], 0, (char *)index_table, BLOCK_SIZE);
        unsigned int second_block_no = index_table[second_index];
        BufferManager::Read(second_block_no, 0, (char *)index_table, BLOCK_SIZE);
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
    unsigned int block_cnt = ceil(float(i_size) / BLOCK_SIZE);
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
        BufferManager::Read(i_addr[first_index], 0, (char *)level2_index_table, BLOCK_SIZE);

        // change the index table
        level2_index_table[second_index] = physical_block_num;

        // store level2 index table
        BufferManager::Write(i_addr[first_index], 0, (char *)level2_index_table, BLOCK_SIZE);
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
            BufferManager::Read(i_addr[first_index], 0, (char *)level2_index_table, BLOCK_SIZE);
            level2_index_table[second_index] = new_level3_block_num;
            BufferManager::Write(i_addr[first_index], 0, (char *)level2_index_table, BLOCK_SIZE);
        }
        unsigned level2_index_table[PTR_IN_BLOCK_NUM];
        BufferManager::Read(i_addr[first_index], 0, (char *)level2_index_table, BLOCK_SIZE);

        unsigned level3_index_table[PTR_IN_BLOCK_NUM];
        BufferManager::Read(level2_index_table[second_index], 0, (char *)level3_index_table, BLOCK_SIZE);
        level3_index_table[third_index] = physical_block_num;
        BufferManager::Write(level2_index_table[second_index], 0, (char *)level3_index_table, BLOCK_SIZE);
    }
    else
    {
        // file exceeds its maximum size
        // it cannot be linked to any new physical block anymore
        throw "INVALID ACCESS!";
    }
}

void Inode::Free_All_Space()
{
    unsigned int block_cnt = ceil(float(i_size) / BLOCK_SIZE);
    for (unsigned int i = 0; i < block_cnt; i++)
    {
        unsigned int physical_block_num = Offset_To_Index(i * BLOCK_SIZE);
        // cout << "Level1: Free block: " << physical_block_num << endl;
        FileSystem::Free_Block(physical_block_num);
    }
    if (block_cnt > DIRECT_PTR_NUM)
    {
        unsigned int level2_index_table_num = ceil(float(block_cnt - DIRECT_PTR_NUM) / PTR_IN_BLOCK_NUM);
        if (level2_index_table_num <= SEC_PTR_NUM)
        {
            for (unsigned int i = 0; i < level2_index_table_num; i++)
            {
                // cout << "Level2: Free block: " << i_addr[DIRECT_PTR_NUM + i] << endl;
                FileSystem::Free_Block(i_addr[DIRECT_PTR_NUM + i]);
            }
        }
        else
        {
            // cout << "Level2: Free block: " << i_addr[DIRECT_PTR_NUM] << endl;
            // cout << "Level2: Free block: " << i_addr[DIRECT_PTR_NUM + 1] << endl;
            FileSystem::Free_Block(i_addr[DIRECT_PTR_NUM]);
            FileSystem::Free_Block(i_addr[DIRECT_PTR_NUM + 1]);
            unsigned int level2_index_table_num_remain = ceil(float(level2_index_table_num - SEC_PTR_NUM) / PTR_IN_BLOCK_NUM);
            unsigned int level3_index_table_num_remain = level2_index_table_num - SEC_PTR_NUM;
            for (unsigned int i = 0; i < level2_index_table_num_remain; i++)
            {
                unsigned int level2_index_table[PTR_IN_BLOCK_NUM];
                BufferManager::Read(i_addr[DIRECT_PTR_NUM + SEC_PTR_NUM + i], 0, (char *)level2_index_table, BLOCK_SIZE);
                for (unsigned int j = 0; j < PTR_IN_BLOCK_NUM && j < level3_index_table_num_remain; j++)
                {
                    // cout << "Level3: Free block: " << level2_index_table[j] << endl;
                    FileSystem::Free_Block(level2_index_table[j]);
                }
                level3_index_table_num_remain -= PTR_IN_BLOCK_NUM;
                // cout << "Level2: Free block: " << i_addr[DIRECT_PTR_NUM + SEC_PTR_NUM + i] << endl;
                FileSystem::Free_Block(i_addr[DIRECT_PTR_NUM + SEC_PTR_NUM + i]);
            }
        }
    }
}