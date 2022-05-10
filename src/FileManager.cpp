#include "FileManager.h"
#include "Directory.h"
#include "FileSystem.h"

Directory g_cur_dir;

extern SuperBlock g_superblock;

using namespace std;

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

void FileManager::L_Seek(File &file, unsigned int pos)
{
    Inode inode;
    FileSystem::Load_Inode(inode,file.f_inode_id);

    

    FileSystem::Store_Inode(inode,file.f_inode_id);
}

unsigned int FileManager::Write_File(File &file, const char *content)
{
    Inode inode;
    FileSystem::Load_Inode(inode,file.f_inode_id);



    FileSystem::Store_Inode(inode,file.f_inode_id);

    return -1;
}
