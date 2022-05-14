#include "FileSystem.h"
#include "FileManager.h"
#include "DiskDriver.h"

#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>

using namespace std;

SuperBlock g_superblock;

void FileSystem::Init_Some_Dir()
{
    vector<string> list = {"bin", "Users", "dev", "opt", "sbin", "tmp", "usr", "etc"};
    for (auto i : list)
    {
        vector<string> path = {"root"};
        path.push_back(i);
        FileManager::Create_Dir(path);
    }
}

void FileSystem::Load_SuperBlock()
{
    BufferManager::Read(SUPER_BLOCK_INDEX, 0, (char *)&g_superblock, sizeof(SuperBlock));
}

void FileSystem::Store_SuperBlock()
{
    BufferManager::Write(SUPER_BLOCK_INDEX, 0, (char *)&g_superblock, sizeof(SuperBlock));
}

void FileSystem::Init_SuperBlock()
{

    g_superblock.s_inode_num = INODE_NUM;
    g_superblock.s_free_inode_num = INODE_NUM - 1;

    g_superblock.s_total_block_num = TOTAL_BLOCK_NUM - 1;
    g_superblock.s_free_block_num = TOTAL_BLOCK_NUM;

    g_superblock.s_nfree = 1; // Init with 1
    g_superblock.s_free[0] = -1;
}

void FileSystem::Init_All_Free_Blocks()
{
    unsigned int *stack = g_superblock.s_free;
    unsigned int &p_stk = g_superblock.s_nfree;
    // int p_data = DATA_BLOCK_START_INDEX + 1;
    int p_data = TOTAL_BLOCK_NUM - 1;
    // std::cout << "Enter init all free blocks" << std::endl;
    while (p_data > DATA_BLOCK_START_INDEX)
    {
        if (p_stk < MAX_NFREE)
        {
            stack[p_stk++] = p_data--;
        }
        else
        {
            BufferManager::Write(p_data, 0, (char *)stack, sizeof(unsigned int) * MAX_NFREE);
            stack[0] = p_data--;
            p_stk = 1;
        }
    }
}

void FileSystem::Init_BitMap()
{
    unsigned int bitmap[INODE_NUM] = {1};
    BufferManager::Write(BITMAP_START_INDEX, 0, (char *)bitmap, BLOCK_SIZE);
    BufferManager::Write(BITMAP_START_INDEX + 1, 0, (char *)(bitmap + 128), BLOCK_SIZE);
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
    BufferManager::Write(inode.i_addr[0], 0, (char *)&dir, sizeof(Directory));

    FileSystem::Store_Inode(inode, 0);
}

void FileSystem::Init_Register()
{
    // create register file
    vector<string> register_file_path;
    register_file_path.push_back(string("root"));
    register_file_path.push_back(string("etc"));
    register_file_path.push_back(string("passwd"));
    FileManager::Create_File(register_file_path);

    // open it
    FileManager::Open_File(register_file_path);

    // initialize number of user to 3
    unsigned int usr_cnt = 3;
    FileManager::Write_File(register_file_path, (char *)&usr_cnt, sizeof(unsigned int));

    // add root into this file
    User root;
    root.gid = 0;
    root.uid = 0;
    strcpy(root.username, "root");
    strcpy(root.password, "root");
    FileManager::Write_File(register_file_path, (char *)&root, sizeof(User));

    // add Bob into this file
    User Mo;
    Mo.gid = 1;
    Mo.uid = 1;
    strcpy(Mo.username, "Mo");
    strcpy(Mo.password, "Mo");
    FileManager::Write_File(register_file_path, (char *)&Mo, sizeof(User));

    // add Tom into this file
    User Wang;
    Wang.gid = 1;
    Wang.uid = 2;
    strcpy(Wang.username, "Wang");
    strcpy(Wang.password, "Wang");
    FileManager::Write_File(register_file_path, (char *)&Wang, sizeof(User));

    // close it
    FileManager::Close_File(register_file_path);

    // create Bob's own user directory
    vector<string> Mo_directory_path;
    Mo_directory_path.push_back(string("root"));
    Mo_directory_path.push_back(string("Users"));
    Mo_directory_path.push_back(string("Mo"));
    FileManager::Create_Dir(Mo_directory_path);

    // create Tom's own user directory
    vector<string> Wang_directory_path;
    Wang_directory_path.push_back(string("root"));
    Wang_directory_path.push_back(string("Users"));
    Wang_directory_path.push_back(string("Wang"));
    FileManager::Create_Dir(Wang_directory_path);
}

User FileSystem::Check_User(std::string username, std::string password)
{
    // open file
    vector<string> register_file_path;
    register_file_path.push_back(string("root"));
    register_file_path.push_back(string("etc"));
    register_file_path.push_back(string("passwd"));
    FileManager::Open_File(register_file_path);

    // read out how many user this system have now
    unsigned int usr_num;
    FileManager::Read_File(register_file_path, (char *)&usr_num, sizeof(unsigned int));

    User res;
    res.uid = -1;

    // read out every record and compare with given username and password
    for (unsigned int i = 0; i < usr_num; i++)
    {
        // read out
        User read_usr_tmp;
        FileManager::Read_File(register_file_path, (char *)&read_usr_tmp, sizeof(User));

        // if both match
        if ((string(read_usr_tmp.username) == username) && (string(read_usr_tmp.password) == password))
        {
            res = read_usr_tmp;
            break;
        }
    }

    // close file
    FileManager::Close_File(register_file_path);

    return res;
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

    Init_Register();
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
        BufferManager::Read(res, 0, (char *)stack, sizeof(unsigned int) * MAX_NFREE);
        p_stk = MAX_NFREE;
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
        BufferManager::Write(block_num, 0, (char *)stack, sizeof(unsigned int) * MAX_NFREE);
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
        BufferManager::Read(BITMAP_START_INDEX, 0, (char *)bitmap, BLOCK_SIZE);
        BufferManager::Read(BITMAP_START_INDEX + 1, 0, (char *)(bitmap + 128), BLOCK_SIZE);
        for (unsigned int i = 0; i < INODE_NUM; i++)
        {
            if (bitmap[i] == 0)
            {
                g_superblock.s_free_inode_num--;
                unsigned int tmp = 1;
                BufferManager::Write(BITMAP_START_INDEX, i * sizeof(unsigned int), (char *)&tmp, sizeof(unsigned int));
                Store_SuperBlock();
                return i;
            }
        }
    }
    return -1; // It won't return here under normal circumtances.
}

void FileSystem::Free_Inode(unsigned int inode_num)
{
    g_superblock.s_free_inode_num++;
    unsigned int tmp = 0;
    BufferManager::Write(BITMAP_START_INDEX, inode_num * sizeof(unsigned int), (char *)&tmp, sizeof(unsigned int));
    Store_SuperBlock();
}

void FileSystem::Load_Inode(Inode &inode, unsigned int inode_pos)
{
    // BufferManager::Read(INODE_START_INDEX, inode_pos * INODE_SIZE, (char *)&inode, sizeof(Inode));
    int tmp = inode_pos / 8;
    BufferManager::Read(INODE_START_INDEX + tmp, (inode_pos % 8) * INODE_SIZE, (char *)&inode, sizeof(Inode));
}

void FileSystem::Store_Inode(Inode &inode, unsigned int inode_pos)
{
    // BufferManager::Write(INODE_START_INDEX, inode_pos * INODE_SIZE, (char *)&inode, sizeof(Inode));
    int tmp = inode_pos / 8;
    BufferManager::Write(INODE_START_INDEX + tmp, (inode_pos % 8) * INODE_SIZE, (char *)&inode, sizeof(Inode));
}

void FileSystem::Boot()
{
    Load_SuperBlock();
}