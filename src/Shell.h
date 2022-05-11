#ifndef SHELL_H
#define SHELL_H

#include "Directory.h"
#include "User.h"
#include "FileSystem.h"
#include "FileManager.h"

#include <vector>
#include <string>
#include <map>

void Func_Exit();
class Shell
{
public:
    Directory current_dirrectory;
    std::vector<std::string> args;
    bool flag;
    std::string usr_name;
    std::map<std::string, void (Shell::*)(void)> command_exec;
    std::string current_path;

public:
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
    void Func_Create();
    void Func_Rmdir();
    void Func_Close();
    void Func_Open();
<<<<<<< HEAD
    void Func_help();
=======
    void Func_Openlist();
>>>>>>> 3061ef49dbf050a66a328c6536ad788f0ac930ee

    void Parse_Path(std::string, std::vector<std::string> &);
    static void Transform_Path(std::vector<std::string> &, std::vector<std::string> &);

public:
    void Run();
};

#endif