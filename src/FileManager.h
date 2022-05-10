#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "File.h"

#include <string>
#include <vector>

class FileManager
{
private:
    bool Dir_Name_Conflict(std::string dir_name);

public:
    static void Create_Dir(std::vector<std::string> &);

    static void Remove_Dir(std::string dir_name);

    static std::string Current_Dir();

    static File &Open_File();

    static void Close_File(File &file);

    static void L_Seek(File &file, unsigned int pos);

    static unsigned int Write_File(File &file, const char *content);

    static unsigned int Read_File(File &file, char *content, int length);

    // void Edit_File_Permission();
};
int Get_Inode_Num(std::vector<std::string> &, int);

#endif