#include "FileManager.h"
#include "Directory.h"
#include "FileSystem.h"
#include <iostream>
#include <time.h>
#include <cstdint>
#include <cstring>
using namespace std;
Directory g_cur_dir;

extern SuperBlock g_superblock;

using namespace std;

// >= 0: vliad
// == -1: invalid, this file or directory does not exist
int Get_Inode_Num(vector<string> &path, int begin_inode_num = -1)
{
    if (path.size() == 0)
    {
        return begin_inode_num;
    }
    else
    {
        if (begin_inode_num == -1)
        {
            vector<string>::iterator k = path.begin();
            path.erase(k);
            begin_inode_num = 0;
        }
        Inode inode;
        FileSystem::Load_Inode(inode, begin_inode_num);
        unsigned int block_num = inode.i_addr[0];
        Directory directory;
        DiskDriver::Read(block_num * BLOCK_SIZE, (char *)&directory, sizeof(Directory));
        for (unsigned int i = 0; i < inode.i_size; i++)
        {
            if (string(directory.d_filename[i]) == path[0])
            {
                vector<string>::iterator k = path.begin();
                path.erase(k);
                unsigned int res = Get_Inode_Num(path, directory.d_inode_num[i]);
                return res;
            }
        }
        return -1; // Does Not Exit
    }
}
// void FileManager::Create_Dir(string dir_name)
// {
//     // Check if the dir_name illegal
//     if (dir_name.empty() || dir_name.length() > MAX_FILE_NAME)
//     {
//         throw "ILLEGAL FILE NAME";
//     }
//     // Load superblock
//     FileSystem::Load_SuperBlock();
//     // Check if there is enough space
//     if (g_superblock.s_free_block_num <= 0 || g_superblock.s_free_inode_num <= 0)
//     {
//         throw "OUT OF SPACE";
//     }
// }

// void FileManager::Romove_Dir(string dir_name)
// {
//     if (dir_name.empty() || dir_name.length() > MAX_FILE_NAME)
//     {
//         throw "ILEGAL FILE NAME";
//     }
//     FileSystem::Load_SuperBlock();
// }
void FileManager::Create_Dir(vector<string> &path)
{
    if (path.size() < 2)
    {
        throw -1;
    }
    for (auto item : path)
    {
        cout << item << ' ';
    }
    cout << endl;
    string dir_name = path[path.size() - 1];
    path.pop_back();
    for (auto item : path)
    {
        cout << item << ' ';
    }
    unsigned int parent_directory_inode_num = Get_Inode_Num(path);
    if (parent_directory_inode_num == -1)
    {
        cout << "Parent Directory Doesn't Exist!" << endl;
    }
    else
    {
        Inode parent_directory_inode;
        FileSystem::Load_Inode(parent_directory_inode, parent_directory_inode_num);
        vector<string> path0;
        path0.push_back(dir_name);
        unsigned int dir_inode_num = Get_Inode_Num(path0, parent_directory_inode_num);
        if (dir_inode_num != -1)
        {
            cout << "This Directory Already Exists!" << endl;
        }
        else
        {
            dir_inode_num = FileSystem::Allocate_Inode();
            Inode inode;
            inode.i_size = 2;
            inode.i_mode = 1;
            inode.i_number = dir_inode_num;
            inode.i_count = 0;
            inode.i_permission = 0;
            inode.i_uid = 0;
            inode.i_gid = 0;
            time_t t;
            time(&t);
            inode.i_time = t;
            Directory parent_directory;
            unsigned int block_num = parent_directory_inode.i_addr[0];
            DiskDriver::Read(block_num * BLOCK_SIZE, (char *)&parent_directory, sizeof(Directory));
            parent_directory.d_inode_num[parent_directory_inode.i_size] = dir_inode_num;
            memcpy(parent_directory.d_filename[parent_directory_inode.i_size], dir_name.c_str(), dir_name.length());
            DiskDriver::Write(block_num * BLOCK_SIZE, (char *)&parent_directory, sizeof(Directory));

            parent_directory_inode.i_size++;

            FileSystem::Store_Inode(parent_directory_inode, parent_directory_inode_num);

            inode.i_addr[0] = FileSystem::Allocate_Block();

            Directory dir;
            dir.d_inode_num[0] = dir_inode_num;
            dir.d_inode_num[1] = parent_directory_inode_num;
            memcpy(dir.d_filename[0], ".", 1);
            memcpy(dir.d_filename[1], "..", 2);
            DiskDriver::Write(inode.i_addr[0] * BLOCK_SIZE, (char *)&dir, sizeof(Directory));
            FileSystem::Store_Inode(inode, dir_inode_num);
        }
    }
}

void FileManager::L_Seek(File &file, unsigned int pos)
{
    Inode inode;
    FileSystem::Load_Inode(inode, file.f_inode_id);

    FileSystem::Store_Inode(inode, file.f_inode_id);
}

unsigned int FileManager::Write_File(File &file, const char *content)
{
    Inode inode;
    FileSystem::Load_Inode(inode, file.f_inode_id);

    FileSystem::Store_Inode(inode, file.f_inode_id);

    return -1;
}
