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
        unsigned int size = sub_dir_inode.i_size;
        char *time_str = asctime(gmtime(&time));
        time_str[strlen(time_str) - 1] = 0;
        cout << setw(time_len) << time_str;
        cout << setw(type_len);
        if (mode == 1)
        {
            cout << "<DIR>" << setw(size_len) << "";
        }
        else
        {
            cout << "<FILE>" << setw(size_len) << size;
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
    vector<string> path_t;
    Parse_Path(args[1], path_t);
    vector<string> path;
    Transform_Path(path_t, path);
    FileManager::Create_Dir(path);
}

void Shell::Func_Create()
{
    vector<string> path_t;
    Parse_Path(args[1], path_t);
    vector<string> path;
    Transform_Path(path_t, path);
    FileManager::Create_File(path);
}

void Shell::Func_Rmdir()
{
    vector<string> path_t;
    Parse_Path(args[1], path_t);
    vector<string> path;
    Transform_Path(path_t, path);
    vector<string> path_now_t, path_now;
    Parse_Path(current_path, path_now_t);
    Transform_Path(path_now_t, path_now);
    // determine whether this path is current path's parent path
    unsigned int i = 0;
    while (i < path_now.size() && i < path.size())
    {
        if (path_now[i] != path[i])
        {
            break;
        }
        i++;
    }

    if (i == path.size())
    {
        cout << "You Can't Delete This Directory Now!" << endl;
    }
    else
    {
        FileManager::Remove_Dir(path);
    }
}

void Shell::Func_Cd()
{
    string complete_path;
    if (args[1][0] != '/')
    {
        complete_path = current_path + "/" + args[1];
    }
    else
    {
        complete_path = args[1];
    }
    vector<string> path_component_t, path_component_t0;
    vector<string> path_component;
    Parse_Path(complete_path, path_component_t);
    path_component_t0 = path_component_t;
    int inode_num = Get_Inode_Num(path_component_t0);
    if (inode_num == -1)
    {
        cout << "This Directory Doesn't Exist!" << endl;
        return;
    }
    Inode inode;
    FileSystem::Load_Inode(inode, inode_num);
    if (inode.i_mode == 0)
    {
        cout << "Wrong Instruction!" << endl;
        return;
    }

    Transform_Path(path_component_t, path_component);

    if (path_component.size() == 1)
    {
        current_path = "/";
    }
    else
    {

        current_path = "";
        for (unsigned int i = 1; i < path_component.size(); i++)
        {
            current_path += "/" + path_component[i];
        }
    }
}

void Shell::Func_Open()
{
    vector<string> path_t;
    Parse_Path(args[1], path_t);
    vector<string> path;
    Transform_Path(path_t, path);
    File *f = FileManager::Open_File(path);
    if (!f)
    {
        cout << "failed to open it, for you could opened it already or it does not exist!" << endl;
    }
    else
    {
        cout << "file successfully opened!" << endl;
    }
}

void Shell::Func_Openlist()
{
    FileManager::Open_File_List();
}

void Shell::Func_Close()
{
    vector<string> path_t;
    Parse_Path(args[1], path_t);
    vector<string> path;
    Transform_Path(path_t, path);
    FileManager::Close_File(path);
}

void Shell::Func_Rm()
{
    vector<string> path_t;
    Parse_Path(args[1], path_t);
    vector<string> path;
    Transform_Path(path_t, path);
    FileManager::Remove_File(path);
}

void Shell::Func_Write()
{
    vector<string> path_t;
    Parse_Path(args[1], path_t);
    vector<string> path;
    Transform_Path(path_t, path);
    FileManager::Write_File(path, args[2].c_str());
}

void Shell::Func_Read()
{
    vector<string> path_t;
    Parse_Path(args[1], path_t);
    vector<string> path;
    Transform_Path(path_t, path);
    const int MAX_CONTENT_LEN = 500;
    char content[MAX_CONTENT_LEN];
    FileManager::Read_File(path, content, atoi(args[2].c_str()));
    cout << content << endl;
}
void Shell::Func_Seekg()
{
    vector<string> path_t;
    Parse_Path(args[1], path_t);
    vector<string> path;
    Transform_Path(path_t, path);
    FileManager::L_Seek(path, atoi(args[2].c_str()));
}
void Shell::Init_Command_Exec()
{
    this->command_exec[string("ls")] = &Shell::Func_Ls;
    this->command_exec[string("exit")] = &Shell::Func_Exit;
    this->command_exec[string("mkdir")] = &Shell::Func_Mkdir;
    this->command_exec[string("cd")] = &Shell::Func_Cd;
    this->command_exec[string("create")] = &Shell::Func_Create;
    this->command_exec[string("rmdir")] = &Shell::Func_Rmdir;
    this->command_exec[string("rm")] = &Shell::Func_Rm;
    this->command_exec[string("write")] = &Shell::Func_Write;
    this->command_exec[string("read")] = &Shell::Func_Read;

    this->command_exec[string("help")] = &Shell::Func_help;

    this->command_exec[string("open")] = &Shell::Func_Open;
    this->command_exec[string("openlist")] = &Shell::Func_Openlist;
    this->command_exec[string("close")] = &Shell::Func_Close;
    this->command_exec[string("seekg")] = &Shell::Func_Seekg;
}

void Shell::Prompt()
{
    // string work_directory = "/usr/local/bin";
    printf("\n# %s in %s\n$ ", this->usr_name.c_str(), current_path.c_str());
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
    int count = 0;
    while (s >> buf)
    {
        if (count++ == 0)
        {
            transform(buf.begin(), buf.end(), buf.begin(), ::tolower);
        }
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
        cout << endl;
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
    if (!DiskDriver::Exists())
    {
        DiskDriver::Create_Disk();
    }

    cout << endl;
    cout << "Welcome to virtual unix file system!" << endl;

    current_path = "/";
    cout << "Erase All Data And Format The Disk?(y/n):";
    string format;
    getline(cin, format);
    if (format == "y")
    {
        FileSystem::Format_Disk();
    }
    FileSystem::Boot();

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

void Shell::Transform_Path(vector<string> &path_t, vector<string> &path)
{
    for (unsigned int i = 0; i < path_t.size(); i++)
    {
        if (path_t[i] == ".")
        {
            continue;
        }
        else if (path_t[i] == "..")
        {
            if (path.size() > 1)
            {
                path.pop_back();
            }
        }
        else
        {
            path.push_back(path_t[i]);
        }
    }
}

void Shell::Func_help()
{
}