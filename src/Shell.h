#ifndef SHELL_H
#define SHELL_H

#include "Directory.h"
#include "User.h"
#include "FileSystem.h"

#include <vector>
#include <string>
#include <map>

void Func_Exit();
class Shell
{
private:
    Directory current_dirrectory;
    std::vector<std::string> args;
    bool flag;
    std::string usr_name;
    std::map<std::string, void (Shell::*)(void)> command_exec;

private:
    void Prompt();
    void Get_Command();
    void Execute();
    bool Is_Over();
    void Init_Command_Exec();
    void Log_In();

    void Func_Dir();
    void Func_Exit();
    void Func_Create();
    void Func_Close();
    void Func_Open();

public:
    void Run();
};

unsigned int Get_Inode_Num(std::vector<std::string>, unsigned int);

#endif