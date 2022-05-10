#include "Shell.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <time.h>
using namespace std;
extern FileSystem g_filesystem;
void Shell::Func_Dir()
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

void Shell::Func_Exit()
{
    cout << "Exit!" << endl;
    this->flag = false;
}
void Shell::Init_Command_Exec()
{
    this->command_exec[string("dir")] = &Shell::Func_Dir;
    this->command_exec[string("exit")] = &Shell::Func_Exit;
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

// >= 0: vliad
// == -1: invalid, this file or directory does not exist
unsigned int Get_Inode_Num(vector<string> &path, unsigned int begin_inode_num = 0)
{
    if (path.size() == 0)
    {
        return begin_inode_num;
    }
    else
    {
        Inode inode;
        DiskDriver::Read(INODE_START_INDEX * BLOCK_SIZE + begin_inode_num * INODE_SIZE, (char *)&inode, sizeof(Inode));
        unsigned int block_num = inode.i_addr[0];
        Directory directory;
        DiskDriver::Read(DATA_BLOCK_START_INDEX * BLOCK_SIZE + block_num * BLOCK_SIZE, (char *)&directory, sizeof(Directory));
        for (unsigned int i = 0; i < inode.i_size; i++)
        {
            if (string(directory.d_filename[i]) == path[0])
            {
                vector<string>::iterator k = path.begin();
                path.erase(k);
                unsigned int res = Get_Inode_Num(path, directory.d_inode_num[i]);
                return res;
            }
        }
        return -1; // Does Not Exit
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

// parse
// mkdir, open