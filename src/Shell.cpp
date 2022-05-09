#include "Shell.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
using namespace std;

void Func_Dir()
{
    const unsigned int time_len = 25;
    const unsigned int type_len = 15;
    const unsigned int size_len = 15;
    const unsigned int name_len = 25;
    cout << setw(time_len) << "Edit Time";
    cout << setw(type_len) << "Type";
    cout << setw(size_len) << "Size(Byte)";
    cout << setw(name_len) << "Name";
    cout << endl;
    // 每行输出文件/目录信息
    // ......
}

void Shell::Prompt()
{
    // string user_name = "vw";
    string work_directory = "/usr/local/bin";
    printf("# %s in %s\n$ ", this->usr_name.c_str(), work_directory.c_str());
}

void Shell::Get_Command()
{
    unsigned int len = this->args.size();
    while (len > 0)
    {
        this->args.pop_back();
        len--;
    }
    string command;
    getline(cin, command);
    string buf;
    stringstream s(command);
    while (s >> buf)
    {
        transform(buf.begin(), buf.end(), buf.begin(), ::tolower);
        this->args.push_back(buf);
    }
    // for (auto arg : this->args)
    // {
    //     cout << arg << endl;
    // }
}

void Shell::Execute()
{
    if (this->args[0] == "exit")
    {
        cout << "Exit!" << endl;
        this->flag = false;
    }
    else if (this->args[0] == "dir")
    {
        Func_Dir();
    }
}

bool Shell::Is_Over()
{
    return !this->flag;
}

void Shell::Run()
{
    bool login = false;
    while (1)
    {
        string usr;
        string psw;
        cout << "Input Your ID: ";
        getline(cin, usr);
        cout << "Input Password: ";
        getline(cin, psw);
        if (usr == "root")
        {
            if (psw == "root")
            {
                login = true;
            }
        }
        else if (usr == "Bob")
        {
            if (psw == "123456")
            {
                login = true;
            }
        }
        if (login)
        {
            this->usr_name = usr;
            break;
        }
        else
        {
            cout << "Please Check Your ID and Password, and Try Again!" << endl;
        }
    }

    this->flag = true;
    while (1)
    {
        this->Prompt();
        this->Get_Command();
        this->Execute();
        if (this->Is_Over())
        {
            break;
        }
    }
}

// 2022/5/9 21.01
