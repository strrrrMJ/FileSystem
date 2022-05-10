#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>

class FileManager
{
public:
    static void Create_Dir(std::string dir_name);
    static void Romove_Dir(std::string dir_name);
    
    static std::string Current_dir();
};

#endif