#include "Shell.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <time.h>
#include <string.h>
using namespace std;
extern FileSystem g_filesystem;
void Shell::Func_Ls()
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
    Inode inode;
    vector<string> path;
    Parse_Path(current_path, path);
    int dir_inode_num = Get_Inode_Num(path);
    FileSystem::Load_Inode(inode, dir_inode_num);
    Directory dir;
    DiskDriver::Read(inode.i_addr[0] * BLOCK_SIZE, (char *)&dir, sizeof(Directory));
    for (unsigned int i = 0; i < inode.i_size; i++)
    {
        Inode sub_dir_inode;
        unsigned int sub_dir_inode_num = dir.d_inode_num[i];
        FileSystem::Load_Inode(sub_dir_inode, sub_dir_inode_num);
        time_t time = sub_dir_inode.i_time;
        unsigned short mode = sub_dir_inode.i_mode;
        unsigned short size = sub_dir_inode.i_size;
        char *time_str = asctime(gmtime(&time));
        time_str[strlen(time_str) - 1] = 0;
        cout << setw(time_len) << time_str;
        cout << setw(type_len);
        if (mode == 1)
        {
            cout << "DIR" << setw(size_len) << "";
        }
        else
        {
            cout << "FILE" << setw(size_len) << size;
        }
        cout << setw(name_len) << dir.d_filename[i];
        cout << endl;
    }
}

void Shell::Func_Exit()
{
    cout << "Exit!" << endl;
    this->flag = false;
}

void Shell::Func_Mkdir()
{
    vector<string> path;
    Parse_Path(args[1], path);
    FileManager::Create_Dir(path);
}

void Shell::Init_Command_Exec()
{
    this->command_exec[string("ls")] = &Shell::Func_Ls;
    this->command_exec[string("exit")] = &Shell::Func_Exit;
    this->command_exec[string("mkdir")] = &Shell::Func_Mkdir;
}

void Shell::Prompt()
{
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
}

void Shell::Execute()
{
    if (this->args.size() < 1)
    {
        return;
    }
    else
    {
        string command_name = this->args[0];
        if (this->command_exec.count(command_name) == 0)
        {
            cout << "Wrong Instruction!" << endl;
        }
        else
        {
            void (Shell::*func_ptr)(void);
            func_ptr = this->command_exec[command_name];
            (this->*func_ptr)();
        }
    }
}

bool Shell::Is_Over()
{
    return !this->flag;
}

void Shell::Log_In()
{
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
                this->usr_name = usr;
                break;
            }
        }
        else if (usr == "Bob")
        {
            if (psw == "123456")
            {
                this->usr_name = usr;
                break;
            }
        }
        cout << "Please Check Your ID and Password, and Try Again!" << endl;
    }
}

void Shell::Run()
{
    current_path = "/";
    g_filesystem.Format_Disk();

    this->Log_In();
    this->flag = true;
    this->Init_Command_Exec();
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

// Make sure path_string.length() > 0
void Shell::Parse_Path(string path_string, vector<string> &path)
{
    // path example: /usr/psw
    string complete_path;
    if (path_string[0] != '/')
    {
        complete_path = this->current_path + "/" + path_string;
    }
    else
    {
        complete_path = path_string;
    }

    vector<string> res;
    res.push_back(string("root"));

    stringstream ss(complete_path);
    string item;
    while (getline(ss, item, '/'))
    {
        if (!item.empty())
        {
            res.push_back(item);
        }
    }
    path = res;
}