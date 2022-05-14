#ifndef SHELL_H
#define SHELL_H

#include "Directory.h"
#include "User.h"
#include "FileSystem.h"
#include "FileManager.h"

#include <vector>
#include <string>
#include <map>

struct File_Tree
{
    std::string name;
    std::vector<File_Tree *> child;
};

class Shell
{
private:
    void Recursive_Helper_Of_Func_Tree(int, std::vector<std::string> &);
    File_Tree *Construct_Tree(std::string, unsigned int);
    void Destroy_Tree(File_Tree *);
    std::string Get_Tree_Display_String(File_Tree *, unsigned int, std::vector<unsigned int> &);

private:
    Directory current_dirrectory;
    std::vector<std::string> args;
    bool flag;
    std::string usr_name;
    std::map<std::string, void (Shell::*)(void)> command_exec;
    std::string current_path;

private:
    void Prompt();
    void Get_Command();
    void Execute();
    bool Is_Over();
    void Init_Command_Exec();
    void Log_In();

    void Func_Ls();
    void Func_Exit();
    void Func_Mkdir();
    void Func_Cd();
    void Func_Touch();
    void Func_Rm();
    void Func_Rmdir();
    void Func_Close();
    void Func_Open();
    void Func_Write();
    void Func_Read();
    void Func_Lseek();
    void Func_Help();
    void Func_Tree();
    void Func_Logout();
    void Func_Chmod();
    void Func_Register();
    void Func_Userlist();

    void Func_Openlist();

public:
    void Parse_Path(std::string, std::vector<std::string> &);
    static void Transform_Path(std::vector<std::string> &, std::vector<std::string> &);

public:
    void Run();
};

#endif