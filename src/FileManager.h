#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "File.h"

#include <string>
#include <vector>
#include <map>

class FileManager
{

private:
    static std::map<std::string, File *> f_open_map;

public:
    static void Create_Dir(std::vector<std::string> &);

    static void Remove_Dir(std::vector<std::string> &);

    static void Create_File(std::vector<std::string> &);

    static void Remove_File(std::vector<std::string> &);

    static std::string Current_Dir();

    static File *Open_File(std::vector<std::string> &);

    static void Open_File_List();

    static void Close_File(std::vector<std::string> &);

    static void L_Seek(std::vector<std::string> &, unsigned int);

    static unsigned int Write_File(std::vector<std::string> &, const char *content);

    static unsigned int Read_File(std::vector<std::string> &, char *content, int length);
};
int Get_Inode_Num(std::vector<std::string> &, int = -1);
#endif