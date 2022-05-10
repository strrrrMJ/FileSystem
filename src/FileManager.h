#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>

Directory g_cur_dir;

class FileManager
{
private:
    bool Dir_Name_Conflict(std::string dir_name);
public:
    static void Create_Dir(std::string dir_name);
    static void Romove_Dir(std::string dir_name);

    static std::string Current_Dir();
};

#endif