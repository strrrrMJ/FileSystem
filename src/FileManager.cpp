#include "FileManager.h"
#include "Directory.h"
#include "FileSystem.h"
#include <iostream>
#include <time.h>
#include <cstdint>
#include <cstring>
#include <cmath>

using namespace std;

Directory g_cur_dir;
std::map<std::string, File *> FileManager::f_open_map;
extern SuperBlock g_superblock;
extern User g_user;

// >= 0: valid
// == -1: invalid, this file or directory does not exist
int Get_Inode_Num(vector<string> path, int begin_inode_num)
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
        BufferManager::Read(block_num, 0, (char *)&directory, sizeof(Directory));
        // BufferManager::Read(block_num * BLOCK_SIZE, (char *)&directory, sizeof(Directory));

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

void FileManager::Create_Dir(vector<string> path)
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
            inode.i_permission = 7 * 64 + 5 * 8 + 5;
            inode.i_uid = g_user.uid;
            inode.i_gid = g_user.gid;
            time_t t;
            time(&t);
            inode.i_time = t;

            Directory parent_directory;
            unsigned int block_num = parent_directory_inode.i_addr[0];
            BufferManager::Read(block_num, 0, (char *)&parent_directory, sizeof(Directory));
            // BufferManager::Read(block_num * BLOCK_SIZE, (char *)&parent_directory, sizeof(Directory));
            parent_directory.d_inode_num[parent_directory_inode.i_size] = dir_inode_num;
            strcpy(parent_directory.d_filename[parent_directory_inode.i_size], dir_name.c_str());
            BufferManager::Write(block_num, 0, (char *)&parent_directory, sizeof(Directory));
            // BufferManager::Write(block_num * BLOCK_SIZE, (char *)&parent_directory, sizeof(Directory));

            parent_directory_inode.i_size++;

            FileSystem::Store_Inode(parent_directory_inode, parent_directory_inode_num);

            inode.i_addr[0] = FileSystem::Allocate_Block();

            Directory dir;
            dir.d_inode_num[0] = dir_inode_num;
            dir.d_inode_num[1] = parent_directory_inode_num;
            strcpy(dir.d_filename[0], ".");
            strcpy(dir.d_filename[1], "..");
            BufferManager::Write(inode.i_addr[0], 0, (char *)&dir, sizeof(Directory));
            // BufferManager::Write(inode.i_addr[0] * BLOCK_SIZE, (char *)&dir, sizeof(Directory));
            FileSystem::Store_Inode(inode, dir_inode_num);
        }
    }
}

// Recursively Verify Whether We Can Remove_Dir
bool FileManager::Verify_Before_Rmdir(std::vector<std::string> path)
{

    int dir_inode_num = Get_Inode_Num(path);
    Inode dir_inode;
    FileSystem::Load_Inode(dir_inode, dir_inode_num);

    Directory dir;
    BufferManager::Read(dir_inode.i_addr[0], 0, (char *)&dir, sizeof(Directory));
    // BufferManager::Read(dir_inode.i_addr[0] * BLOCK_SIZE, (char *)&dir, sizeof(Directory));

    for (int i = 2; i < dir_inode.i_size; i++)
    {
        string entry_name = string(dir.d_filename[i]);
        Inode entry_inode;
        FileSystem::Load_Inode(entry_inode, dir.d_inode_num[i]);
        vector<string> path_temp(path);
        path_temp.push_back(entry_name);

        if (entry_inode.i_mode == 0)
        {
            if (!Verify_Before_Rm(path_temp))
            {
                return false;
            }
        }
        else
        {
            if (!Verify_Before_Rmdir(path_temp))
            {
                return false;
            }
        }
    }
    return true;
}

// Recursively delete
void FileManager::Remove_Dir(vector<string> path)
{
    if (path.size() < 2)
    {
        cout << "Wrong Instruction!" << endl;
        return;
    }
    vector<string> path_current_absolute = path;
    string dir_name = path[path.size() - 1];
    path.pop_back();
    int parent_directory_inode_num = Get_Inode_Num(path);
    path.push_back(dir_name);
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

            if (!Verify_Before_Rmdir(path))
            {
                cout << "You Cannot Remove This Directory!" << endl;
                return;
            }

            // Recursively remove the subdirectories and files contained by the directory which would be deleted
            // If We don't recursively remove these stuffs, we cannot free the space they occupy
            {
                Directory dir;
                BufferManager::Read(dir_inode.i_addr[0], 0, (char *)&dir, sizeof(Directory));
                // BufferManager::Read(dir_inode.i_addr[0] * BLOCK_SIZE, (char *)&dir, sizeof(Directory));

                for (int i = 2; i < dir_inode.i_size; i++)
                {
                    string entry_name = string(dir.d_filename[i]);
                    Inode entry_inode;
                    FileSystem::Load_Inode(entry_inode, dir.d_inode_num[i]);
                    vector<string> path_temp(path_current_absolute);
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
                BufferManager::Read(parent_directory_inode.i_addr[0], 0, (char *)&parent_dir, sizeof(Directory));
                // BufferManager::Read(parent_directory_inode.i_addr[0] * BLOCK_SIZE, (char *)&parent_dir, sizeof(Directory));
                {
                    int i = 0;
                    while (string(parent_dir.d_filename[i]) != dir_name)
                    {
                        i++;
                    }
                    while (i + 1 < parent_directory_inode.i_size)
                    {
                        strcpy(parent_dir.d_filename[i], parent_dir.d_filename[i + 1]);
                        parent_dir.d_inode_num[i] = parent_dir.d_inode_num[i + 1];
                        i++;
                    }
                }
                BufferManager::Write(parent_directory_inode.i_addr[0], 0, (char *)&parent_dir, sizeof(Directory));
                // BufferManager::Write(parent_directory_inode.i_addr[0] * BLOCK_SIZE, (char *)&parent_dir, sizeof(Directory));
                parent_directory_inode.i_size--;
            }
        }
        FileSystem::Store_Inode(dir_inode, dir_inode_num);
    }

    FileSystem::Store_Inode(parent_directory_inode, parent_directory_inode_num);
}

void FileManager::Create_File(vector<string> path)
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
            inode.i_permission = 6 * 64 + 4 * 8 + 4;
            inode.i_uid = g_user.uid;
            inode.i_gid = g_user.gid;
            time_t t;
            time(&t);
            inode.i_time = t;

            Directory parent_directory;
            unsigned int block_num = parent_directory_inode.i_addr[0];
            BufferManager::Read(block_num, 0, (char *)&parent_directory, sizeof(Directory));
            // BufferManager::Read(block_num * BLOCK_SIZE, (char *)&parent_directory, sizeof(Directory));
            parent_directory.d_inode_num[parent_directory_inode.i_size] = file_inode_num;
            strcpy(parent_directory.d_filename[parent_directory_inode.i_size], file_name.c_str());
            BufferManager::Write(block_num, 0, (char *)&parent_directory, sizeof(Directory));
            // BufferManager::Write(block_num * BLOCK_SIZE, (char *)&parent_directory, sizeof(Directory));

            parent_directory_inode.i_size++;

            FileSystem::Store_Inode(parent_directory_inode, parent_directory_inode_num);

            // inode.i_addr[0] = FileSystem::Allocate_Block();

            FileSystem::Store_Inode(inode, file_inode_num);
        }
    }
}

bool FileManager::Verify_Before_Rm(vector<string> path)
{
    int file_inode_num = Get_Inode_Num(path);
    Inode file_inode;
    FileSystem::Load_Inode(file_inode, file_inode_num);

    // merge path component into a string, as is convenient to judge whether this file was already opened
    string path_string = "";
    for (unsigned int i = 1; i < path.size(); i++)
    {
        path_string += "/" + path[i];
    }

    // Judge whether this file is already closed
    if (f_open_map.count(path_string))
    {
        // cout << "This File Hasn't Been Closed! You Should Close It Before Removing!" << endl;
        return false;
    }

    // Verify the permission

    if (file_inode.i_gid != g_user.gid && ((file_inode.i_permission & Inode::ELSE_W) == false) && g_user.uid != 0)
    {
        return false;
    }
    if (file_inode.i_uid != g_user.uid && ((file_inode.i_permission & Inode::GROUP_W) == false) && g_user.uid != 0)
    {
        return false;
    }
    if (((file_inode.i_permission & Inode::Owner_W) == false) && g_user.uid != 0)
    {
        return false;
    }

    return true;
}

void FileManager::Remove_File(vector<string> path)
{
    vector<string> path_tmp(path);

    if (path.size() < 2)
    {
        cout << "Wrong Instruction!" << endl;
        return;
    }
    string file_name = path[path.size() - 1];
    path.pop_back();
    int parent_directory_inode_num = Get_Inode_Num(path);
    path.push_back(file_name);
    if (parent_directory_inode_num == -1)
    {
        cout << "This File Doesn't Exist!" << endl;
        return;
    }

    Inode parent_directory_inode;
    FileSystem::Load_Inode(parent_directory_inode, parent_directory_inode_num);

    vector<string> path0;
    path0.push_back(file_name);
    int file_inode_num = Get_Inode_Num(path0, parent_directory_inode_num);

    if (file_inode_num == -1)
    {
        cout << "This File Doesn't Exist!" << endl;
    }
    else
    {
        if (!Verify_Before_Rm(path))
        {
            cout << "You Cannot Delete This File, For You Haven't Close It Or You Haven't The Permission To Delete!" << endl;
            return;
        }

        Inode file_inode;
        FileSystem::Load_Inode(file_inode, file_inode_num);

        // release all space
        file_inode.Free_All_Space();

        FileSystem::Free_Inode(file_inode_num);

        // modify parent directory file
        Directory parent_dir;
        BufferManager::Read(parent_directory_inode.i_addr[0], 0, (char *)&parent_dir, sizeof(Directory));
        // BufferManager::Read(parent_directory_inode.i_addr[0] * BLOCK_SIZE, (char *)&parent_dir, sizeof(Directory));
        {
            int i = 0;
            while (string(parent_dir.d_filename[i]) != file_name)
            {
                i++;
            }
            while (i + 1 < parent_directory_inode.i_size)
            {
                strcpy(parent_dir.d_filename[i], parent_dir.d_filename[i + 1]);
                parent_dir.d_inode_num[i] = parent_dir.d_inode_num[i + 1];
                i++;
            }
        }
        BufferManager::Write(parent_directory_inode.i_addr[0], 0, (char *)&parent_dir, sizeof(Directory));
        // BufferManager::Write(parent_directory_inode.i_addr[0] * BLOCK_SIZE, (char *)&parent_dir, sizeof(Directory));

        parent_directory_inode.i_size--;
        FileSystem::Store_Inode(parent_directory_inode, parent_directory_inode_num);

        cout << "Successfully Removing This File!" << endl;
    }
}

File *FileManager::Open_File(vector<string> path)
{
    vector<string> path0 = path;
    // determine whether this file exists
    int inode_num = Get_Inode_Num(path);
    if (inode_num == -1)
    {
        // this file doesn't exist
        return NULL;
    }
    // determine whether it's a file
    Inode inode;
    FileSystem::Load_Inode(inode, inode_num);
    if (inode.i_mode == 1)
    {
        // it's directory
        return NULL;
    }

    // Verify the permission
    if (inode.i_gid != g_user.gid && ((inode.i_permission & Inode::ELSE_R) == false) && g_user.uid != 0)
    {
        // cout << "Current User Doesn't Have Permission To Open!" << endl;
        return NULL;
    }
    if (inode.i_uid != g_user.uid && ((inode.i_permission & Inode::GROUP_R) == false) && g_user.uid != 0)
    {
        // cout << "Current User Doesn't Have Permission To Open!" << endl;
        return NULL;
    }
    if (((inode.i_permission & Inode::Owner_R) == false) && g_user.uid != 0)
    {
        // cout << "Current User Doesn't Have Permission To Open!" << endl;
        return NULL;
    }

    // merge path component into a string, as is convenient to judge whether this file was already opened
    string path_string = "";
    for (unsigned int i = 1; i < path0.size(); i++)
    {
        path_string += "/" + path0[i];
    }
    // judge
    if (f_open_map.count(path_string) != 0)
    {
        // already opened
        return NULL;
    }

    // modify data of inode
    inode.i_count++;
    inode.i_time = time(NULL);
    FileSystem::Store_Inode(inode, inode_num);

    // add this file handler to map
    File *f = new File;
    f->f_inode_id = inode_num;
    f->f_offset = 0;
    f->f_offset = 0;
    f_open_map[path_string] = f;
    // cout << path_string << endl;
    return f;
}

void FileManager::Open_File_List()
{
    cout << "File Already Opened:\n";
    for (auto &item : f_open_map)
    {
        cout << item.first << endl;
    }
}

void FileManager::Close_File(vector<string> path)
{
    // merge path component into a string
    string path_string = "";
    for (unsigned int i = 1; i < path.size(); i++)
    {
        path_string += "/" + path[i];
    }
    // if it hasn't been opened
    if (f_open_map.count(path_string) == 0)
    {
        cout << "This File Is Not Opened!" << endl;
    }
    else
    {
        // release File's memory
        delete f_open_map[path_string];

        f_open_map.erase(path_string);

        // cout << "Close File Successfully!" << endl;
    }
}

unsigned int FileManager::L_Seek_Pos(vector<string> path)
{
    string path_string = "";
    for (unsigned int i = 1; i < path.size(); i++)
    {
        path_string += "/" + path[i];
    }
    if (f_open_map.count(path_string) == 0)
    {
        return -1;
    }
    else
    {
        File *file = f_open_map[path_string];
        return file->f_offset;
    }
    return -1;
}

void FileManager::L_Seek(vector<string> path, unsigned int pos)
{
    string path_string = "";
    for (unsigned int i = 1; i < path.size(); i++)
    {
        path_string += "/" + path[i];
    }
    if (f_open_map.count(path_string) == 0)
    {
        cout << "This File Doesn't Exist Or You Didn't Open This File!" << endl;
    }
    else
    {
        File *file = f_open_map[path_string];

        Inode inode;
        FileSystem::Load_Inode(inode, file->f_inode_id);
        if (pos > inode.i_size)
        {
            cout << "Invalid Access!" << endl;
        }
        else
        {
            file->f_offset = pos;
        }
        FileSystem::Store_Inode(inode, file->f_inode_id);
    }
}

unsigned int FileManager::Write_File(vector<string> path, const char *content, unsigned int length)
{
    unsigned int total_wrriten_bytes = 0;

    // Get file structure
    string path_string = "";
    for (unsigned int i = 1; i < path.size(); i++)
    {
        path_string += "/" + path[i];
    }
    if (f_open_map.count(path_string) == 0)
    {
        cout << "This File Doesn't Exist Or You Didn't Open This File!" << endl;
    }
    else
    {
        File *file = f_open_map[path_string];
        unsigned int &offset = file->f_offset;

        // Judge whether this write is feasible
        unsigned int free_blocks_size = g_superblock.s_free_block_num * BLOCK_SIZE;
        unsigned int free_file_size = MAX_FILE_BLOCK_NUM * BLOCK_SIZE - offset;

        if (length > free_file_size)
        {
            cout << "Maximum File Limit Exceeded!" << endl;
        }
        else if (length > free_blocks_size)
        {
            cout << "Disk Space Isn't Enough!" << endl;
        }
        else
        {
            // Get the inode
            Inode inode;
            FileSystem::Load_Inode(inode, file->f_inode_id);

            // Verify the permission
            if (inode.i_gid != g_user.gid && ((inode.i_permission & Inode::ELSE_W) == false) && g_user.uid != 0)
            {
                // cout << "aaa" << endl;
                cout << "Current User Doesn't Have Permission To Write!" << endl;
                return 0;
            }
            if (inode.i_uid != g_user.uid && ((inode.i_permission & Inode::GROUP_W) == false) && g_user.uid != 0)
            {
                // cout << "bbb" << endl;
                cout << "Current User Doesn't Have Permission To Write!" << endl;
                return 0;
            }
            if (((inode.i_permission & Inode::Owner_W) == false) && g_user.uid != 0)
            {
                // cout << "ccc" << endl;
                cout << "Current User Doesn't Have Permission To Write!" << endl;
                return 0;
            }

            // Write
            char *ptr = (char *)content;
            while (length > 0)
            {
                // Caculate the size of blocks which are occupied by this file
                unsigned int num_of_blk_occupied_by_this_file = ceil(float(inode.i_size) / BLOCK_SIZE);
                unsigned int temp = num_of_blk_occupied_by_this_file * BLOCK_SIZE;

                // Determine which phisical block would be written
                unsigned int blkno;
                if (offset < temp)
                {
                    // Find the block (whick is already allocated)
                    blkno = inode.Offset_To_Index(offset);
                }
                else
                {
                    // Allocate a new block
                    blkno = FileSystem::Allocate_Block();
                    // Link this new block to the inode's index structure
                    inode.Link(blkno);
                }

                // Determine how many bytes would be write this cycle
                unsigned int offset_in_blk = offset % BLOCK_SIZE;
                unsigned int w_cnt_this_cycle = length < BLOCK_SIZE - offset_in_blk ? length : BLOCK_SIZE - offset_in_blk;

                // Write to disk
                BufferManager::Write(blkno, offset_in_blk, ptr, w_cnt_this_cycle);
                // BufferManager::Write(blkno * BLOCK_SIZE + offset_in_blk, ptr, w_cnt_this_cycle);

                // Update some variables
                length -= w_cnt_this_cycle;
                ptr += w_cnt_this_cycle;
                offset += w_cnt_this_cycle;
                total_wrriten_bytes += w_cnt_this_cycle;

                // Chage the file's size
                inode.i_size = (offset > inode.i_size) ? offset : inode.i_size;
            }

            // Change the last modified time
            inode.i_time = time(NULL);

            // Store the inode into disk
            FileSystem::Store_Inode(inode, file->f_inode_id);
        }
    }

    return total_wrriten_bytes;
}

unsigned int FileManager::Get_File_Size(vector<string> path)
{
    unsigned int inode_num = Get_Inode_Num(path);
    Inode inode;
    FileSystem::Load_Inode(inode, inode_num);
    return inode.i_size;
}

unsigned int FileManager::Read_File(vector<string> path, char *content, int length)
{
    unsigned int total_read_bytes = 0;

    // Get file structure
    string path_string = "";
    for (unsigned int i = 1; i < path.size(); i++)
    {
        path_string += "/" + path[i];
    }
    if (f_open_map.count(path_string) == 0)
    {
        cout << "This File Doesn't Exist Or You Didn't Open This File!" << endl;
    }
    else
    {
        File *file = f_open_map[path_string];
        unsigned int &offset = file->f_offset;

        // Get the inode
        Inode inode;
        FileSystem::Load_Inode(inode, file->f_inode_id);

        // Verify the permission
        if (inode.i_gid != g_user.gid && ((inode.i_permission & Inode::ELSE_R) == false) && g_user.uid != 0)
        {
            cout << "Current User Doesn't Have Permission To Read!" << endl;
            return 0;
        }
        if (inode.i_uid != g_user.uid && ((inode.i_permission & Inode::GROUP_R) == false) && g_user.uid != 0)
        {
            cout << "Current User Doesn't Have Permission To Read!" << endl;
            return 0;
        }
        if (((inode.i_permission & Inode::Owner_R) == false) && g_user.uid != 0)
        {
            cout << "Current User Doesn't Have Permission To Read!" << endl;
            return 0;
        }

        // Judge whether this read is feasible
        unsigned int free_file_size = inode.i_size - offset;
        if (length > free_file_size)
        {
            cout << "Don't Try To Read More Than The File!" << endl;
            return 0;
        }

        // Read
        char *ptr = (char *)content;
        while (length > 0)
        {
            // Get the physical blkno
            unsigned int blkno;
            blkno = inode.Offset_To_Index(offset);

            // Determine how many bytes would be read this cycle
            unsigned int offset_in_blk = offset % BLOCK_SIZE;
            unsigned int r_cnt_this_cycle = length < BLOCK_SIZE - offset_in_blk ? length : BLOCK_SIZE - offset_in_blk;

            // Read from disk
            BufferManager::Read(blkno, offset_in_blk, ptr, r_cnt_this_cycle);
            // BufferManager::Read(blkno * BLOCK_SIZE + offset_in_blk, ptr, r_cnt_this_cycle);

            // Update some variables
            length -= r_cnt_this_cycle;
            ptr += r_cnt_this_cycle;
            offset += r_cnt_this_cycle;
            total_read_bytes += r_cnt_this_cycle;
        }

        // Store the inode into disk
        FileSystem::Store_Inode(inode, file->f_inode_id);
    }

    return total_read_bytes;
}

void FileManager::Empty_Open_File_Table()
{
    f_open_map.erase(f_open_map.begin(), f_open_map.end());
}