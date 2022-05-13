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

    static bool Verify_Before_Rmdir(std::vector<std::string>);

    static bool Verify_Before_Rm(std::vector<std::string>);

public:
    static void Create_Dir(std::vector<std::string>);

    static void Remove_Dir(std::vector<std::string>);

    static void Create_File(std::vector<std::string>);

    static void Remove_File(std::vector<std::string>);

    static std::string Current_Dir();

    static File *Open_File(std::vector<std::string>);

    static void Open_File_List();

    static void Close_File(std::vector<std::string>);

    static void L_Seek(std::vector<std::string>, unsigned int);

    static unsigned int L_Seek_Pos(std::vector<std::string>);

    static unsigned int Write_File(std::vector<std::string>, const char *, unsigned int);

    static unsigned int Read_File(std::vector<std::string>, char *content, int length);

    // static unsigned int Read_Whole_File(std::vector<std::string> &, std::string);

    static unsigned int Get_File_Size(std::vector<std::string>);

    static void Empty_Open_File_Table();
};

int Get_Inode_Num(std::vector<std::string>, int = -1);

#endif