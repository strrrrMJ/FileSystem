#ifndef SHELL_H
#define SHELL_H

#include "Directory.h"
#include "User.h"

#include <vector>
#include <string>

void Func_Dir();

class Shell
{
private:
    Directory current_dirrectory;
    std::vector<std::string> args;
    bool flag;
    std::string usr_name;

private:
    void Prompt();
    void Get_Command();
    void Execute();
    bool Is_Over();

public:
    void Run();
};

#endif