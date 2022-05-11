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
int Get_Inode_Num(vector<string> &path, int begin_inode_num)
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
            if (path.size() == 0)
            {
                return begin_inode_num;
            }
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
                int res = Get_Inode_Num(path, directory.d_inode_num[i]);
                return res;
            }
        }
        return -1; // Does Not Exit
    }
}

void FileManager::Create_Dir(vector<string> &path)
{
    if (path.size() < 2)
    {
        throw -1;
    }
    string dir_name = path[path.size() - 1];
    path.pop_back();
    int parent_directory_inode_num = Get_Inode_Num(path);
    if (parent_directory_inode_num == -1)
    {
        cout << "Parent Directory Doesn't Exist!" << endl;
    }
    else
    {
        Inode parent_directory_inode;
        FileSystem::Load_Inode(parent_directory_inode, parent_directory_inode_num);
        if (parent_directory_inode.i_mode == 0)
        {
            cout << "Parent Directory Doesn't Exist!" << endl;
            return;
        }
        vector<string> path0;
        path0.push_back(dir_name);
        int dir_inode_num = Get_Inode_Num(path0, parent_directory_inode_num);
        if (dir_inode_num != -1)
        {
            cout << "This Directory or File Already Exists!" << endl;
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
            strcpy(parent_directory.d_filename[parent_directory_inode.i_size], dir_name.c_str());
            DiskDriver::Write(block_num * BLOCK_SIZE, (char *)&parent_directory, sizeof(Directory));

            parent_directory_inode.i_size++;

            FileSystem::Store_Inode(parent_directory_inode, parent_directory_inode_num);

            inode.i_addr[0] = FileSystem::Allocate_Block();

            Directory dir;
            dir.d_inode_num[0] = dir_inode_num;
            dir.d_inode_num[1] = parent_directory_inode_num;
            strcpy(dir.d_filename[0], ".");
            strcpy(dir.d_filename[1], "..");
            DiskDriver::Write(inode.i_addr[0] * BLOCK_SIZE, (char *)&dir, sizeof(Directory));
            FileSystem::Store_Inode(inode, dir_inode_num);
        }
    }
}

// Recursively delete
void FileManager::Remove_Dir(vector<string> &path)
{
    if (path.size() < 2)
    {
        cout << "Wrong Instruction!" << endl;
        return;
    }
    string dir_name = path[path.size() - 1];
    path.pop_back();
    int parent_directory_inode_num = Get_Inode_Num(path);
    if (parent_directory_inode_num == -1)
    {
        cout << "This Directory Doesn't Exist!" << endl;
        return;
    }

    Inode parent_directory_inode;
    FileSystem::Load_Inode(parent_directory_inode, parent_directory_inode_num);

    // Get the inode num of the directory which would be deleted
    vector<string> path0;
    path0.push_back(dir_name);
    int dir_inode_num = Get_Inode_Num(path0, parent_directory_inode_num);

    if (dir_inode_num == -1)
    {
        cout << "This Directory Doesn't Exist!" << endl;
    }
    else
    {

        Inode dir_inode;
        FileSystem::Load_Inode(dir_inode, dir_inode_num);
        if (dir_inode.i_mode == 0)
        {
            cout << "This Is a File, Not a Directory!'" << endl;
        }
        else
        {
            // Recursively remove the subdirectories and files contained by the directory which would be deleted
            // If We don't recursively remove these stuffs, we cannot free the space they occupy
            {
                Directory dir;
                DiskDriver::Read(dir_inode.i_addr[0] * BLOCK_SIZE, (char *)&dir, sizeof(Directory));

                for (int i = 0; i < dir_inode.i_size; i++)
                {
                    string entry_name = string(dir.d_filename[i]);
                    Inode entry_inode;
                    FileSystem::Load_Inode(entry_inode, dir.d_inode_num[i]);

                    vector<string> path_temp(path0);
                    path_temp.push_back(entry_name);
                    if (entry_inode.i_mode == 0)
                    {
                        Remove_File(path_temp);
                    }
                    else
                    {
                        Remove_Dir(path_temp);
                    }

                    FileSystem::Store_Inode(entry_inode, dir.d_inode_num[i]);
                }
            }

            // free the space occupied by this directory directly
            FileSystem::Free_Block(dir_inode.i_addr[0]);
            FileSystem::Free_Inode(dir_inode_num);

            // Change the record stored by the parent directory
            {
                // Remove the record in parent directory's data region, that the datablock pointed by i_data[0]
                Directory parent_dir;
                DiskDriver::Read(parent_directory_inode.i_addr[0] * BLOCK_SIZE, (char *)&parent_dir, sizeof(Directory));
                {
                    int i = 0;
                    while (string(parent_dir.d_filename[i]) != dir_name)
                    {
                        i++;
                    }
                    while (i + 1 < parent_directory_inode.i_size)
                    {
                        strcpy(parent_dir.d_filename[i], parent_dir.d_filename[i + 1]);
                        i++;
                    }
                }
                DiskDriver::Write(parent_directory_inode.i_addr[0] * BLOCK_SIZE, (char *)&parent_dir, sizeof(Directory));
                parent_directory_inode.i_size--;
            }
        }
        FileSystem::Store_Inode(dir_inode, dir_inode_num);
    }

    FileSystem::Store_Inode(parent_directory_inode, parent_directory_inode_num);
}

void FileManager::Create_File(vector<string> &path)
{
    if (path.size() < 2)
    {
        throw -1;
    }
    string file_name = path[path.size() - 1];
    path.pop_back();
    int parent_directory_inode_num = Get_Inode_Num(path);
    if (parent_directory_inode_num == -1)
    {
        cout << "Parent Directory Doesn't Exist!" << endl;
    }
    else
    {
        Inode parent_directory_inode;
        FileSystem::Load_Inode(parent_directory_inode, parent_directory_inode_num);
        if (parent_directory_inode.i_mode == 0)
        {
            cout << "Parent Directory Doesn't Exist!" << endl;
            return;
        }
        vector<string> path0;
        path0.push_back(file_name);
        int file_inode_num = Get_Inode_Num(path0, parent_directory_inode_num);
        if (file_inode_num != -1)
        {
            cout << "This Directory or File Already Exists!" << endl;
        }
        else
        {
            file_inode_num = FileSystem::Allocate_Inode();

            Inode inode;
            inode.i_size = 0;
            inode.i_mode = 0;
            inode.i_number = file_inode_num;
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
            parent_directory.d_inode_num[parent_directory_inode.i_size] = file_inode_num;
            strcpy(parent_directory.d_filename[parent_directory_inode.i_size], file_name.c_str());
            DiskDriver::Write(block_num * BLOCK_SIZE, (char *)&parent_directory, sizeof(Directory));

            parent_directory_inode.i_size++;

            FileSystem::Store_Inode(parent_directory_inode, parent_directory_inode_num);

            // inode.i_addr[0] = FileSystem::Allocate_Block();

            FileSystem::Store_Inode(inode, file_inode_num);
        }
    }
}

void FileManager::Remove_File(vector<string> &path){
    
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

    return -1;
}

unsigned int FileManager::Read_File(File &file, char *content, int length)
{
    return -1;
}