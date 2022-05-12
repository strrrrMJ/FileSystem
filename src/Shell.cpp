#include "Shell.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <time.h>
#include <string.h>
#include <fstream>

using namespace std;
extern FileSystem g_filesystem;
extern User g_user;

string Shell::Get_Tree_Display_String(File_Tree *node, unsigned int n, std::vector<unsigned int> &nodeStatusList)
{
    string tree_string = "";
    if (n == 0)
    {
        // tree_string = "File Structure:\n";
        tree_string = "";
        tree_string += "  " + node->name + "\n";
    }
    else
    {
        tree_string = "";
        for (unsigned int i = 0; i < nodeStatusList.size(); i++)
        {
            if (nodeStatusList[i] == 1)
            {
                tree_string += "  └";
                nodeStatusList[i] = 2;
            }
            else if (nodeStatusList[i] == 0)
            {
                tree_string += "  │";
            }
            else
            {
                tree_string += "   ";
            }
        }
        tree_string += "──" + node->name + "\n";
    }
    if (!node->child.empty())
    {
        for (unsigned int i = 0; i < node->child.size(); i++)
        {
            unsigned int status = 0;
            if (i == node->child.size() - 1)
            {
                status = 1;
            }
            nodeStatusList.push_back(status);
            tree_string += Get_Tree_Display_String(node->child[i], n + 1, nodeStatusList);
            nodeStatusList.pop_back();
        }
    }
    return tree_string;
}

void Shell::Recursive_Helper_Of_Func_Tree(int depth, vector<string> &path)
{
    // Get the inode
    vector<string> path_temp = path;
    unsigned int inode_num = Get_Inode_Num(path_temp);
    Inode inode;
    FileSystem::Load_Inode(inode, inode_num); // No modifying, no storing

    // Get the directory's content
    Directory dir;
    unsigned int blkno = inode.i_addr[0];
    DiskDriver::Read(blkno * BLOCK_SIZE, (char *)&dir, sizeof(Directory)); // No modifying, no storing

    // Traverse subdirectories or files
    for (int i = 2; i < inode.i_size; i++)
    {
        for (int j = 0; j < depth; j++)
        {
            cout << "    ";
        }
        cout << "|___";

        string entry_name(dir.d_filename[i]);
        cout << entry_name << endl;

        if (inode.i_mode == 1)
        {
            path.push_back(entry_name);
            Recursive_Helper_Of_Func_Tree(depth + 1, path);
            path.pop_back();
        }
    }
}

File_Tree *Shell::Construct_Tree(string name, unsigned int parent_dir_inode_num)
{
    // get this file's(or directory's) inode
    vector<string> path;
    path.push_back(name);
    unsigned int inode_num = Get_Inode_Num(path, parent_dir_inode_num);
    Inode inode;
    FileSystem::Load_Inode(inode, inode_num);

    // new File_Tree node
    File_Tree *file_tree_node = new File_Tree;
    // store file's(or directory's) name
    file_tree_node->name = name;

    // it's a file
    if (inode.i_mode == 0)
    {
        return file_tree_node;
    }
    // it's a directory
    else
    {
        Directory dir;
        DiskDriver::Read(inode.i_addr[0] * BLOCK_SIZE, (char *)&dir, sizeof(Directory));
        for (unsigned int i = 2; i < inode.i_size; i++)
        {
            string sub_name = dir.d_filename[i];
            file_tree_node->child.push_back(Construct_Tree(sub_name, inode_num));
        }
        return file_tree_node;
    }
}

void Shell::Destroy_Tree(File_Tree *root)
{
    for (unsigned int i = 0; i < root->child.size(); i++)
    {
        Destroy_Tree(root->child[i]);
    }
    delete root;
}

void Shell::Func_Tree()
{
    // construct file tree structure
    File_Tree *file_tree = new File_Tree;
    Inode inode;
    vector<string> path;
    Parse_Path(current_path, path);
    string name = path[path.size() - 1];
    int dir_inode_num = Get_Inode_Num(path);
    FileSystem::Load_Inode(inode, dir_inode_num);
    Directory dir;
    DiskDriver::Read(inode.i_addr[0] * BLOCK_SIZE, (char *)&dir, sizeof(Directory));

    file_tree->name = name;
    for (unsigned int i = 2; i < inode.i_size; i++)
    {
        string sub_name = dir.d_filename[i];
        file_tree->child.push_back(Construct_Tree(sub_name, dir_inode_num));
    }
    // cout << "." << endl;
    if (args.size() == 1)
    {
        // vector<string> cur_path_vec;
        // Parse_Path(current_path, cur_path_vec);
        // Recursive_Helper_Of_Func_Tree(0, cur_path_vec);
        vector<unsigned int> nodeStatusList;
        string output_string = Get_Tree_Display_String(file_tree, 0, nodeStatusList);
        cout << output_string;
    }
    else
    {
        cout << "Illegal Arguments! Please Read The Help Info!" << endl;
    }

    Destroy_Tree(file_tree);
}

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

    if (args[2] == "-f")
    {
        fstream fp(args[3], ios::binary | ios::in);
        if (!fp)
        {
            cout << "Origin File Doesn't Exist!" << endl;
        }
        else
        {
            fp.seekg(0, ios::end);
            unsigned int read_length = fp.tellg();
            fp.seekg(0, ios::beg);

            char *content = new char[read_length];

            fp.read(content, read_length);

            FileManager::Write_File(path, content, read_length);

            delete content;

            fp.close();
        }
    }
    else if (args[2] == "-s")
    {
        FileManager::Write_File(path, args[3].c_str(), args[3].length());
    }
}

void Shell::Func_Read()
{
    vector<string> path_t;
    Parse_Path(args[1], path_t);
    vector<string> path;
    Transform_Path(path_t, path);

    if (args[2] == "-f")
    {
        fstream fp;
        fp.open(args[3], ios::out | ios::binary);
        if (!fp)
        {
            throw "Failed To Create Target File!";
        }
        // Because we want to read the whole file
        FileManager::L_Seek(path, 0);

        // calculate how many bytes will be read and write
        int bytes_remained = FileManager::Get_File_Size(path);

        // allocate
        char *content = new char[bytes_remained];

        // read from MyDisk.img
        FileManager::Read_File(path, content, bytes_remained);

        // write to output file
        fp.write(content, bytes_remained);

        // release
        delete content;

        fp.close();
    }
    else if (args[2] == "-s")
    {
        const int MAX_CONTENT_LEN = 500;
        char content[MAX_CONTENT_LEN];
        FileManager::Read_File(path, content, atoi(args[3].c_str()));
        cout << content << endl;
    }
}
void Shell::Func_Seekg()
{
    vector<string> path_t;
    Parse_Path(args[1], path_t);
    vector<string> path;
    Transform_Path(path_t, path);
    FileManager::L_Seek(path, atoi(args[2].c_str()));
}

void Shell::Func_Logout()
{
    g_user.uid = (unsigned short)(-1);
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
    this->command_exec[string("help")] = &Shell::Func_Help;
    this->command_exec[string("open")] = &Shell::Func_Open;
    this->command_exec[string("openlist")] = &Shell::Func_Openlist;
    this->command_exec[string("close")] = &Shell::Func_Close;
    this->command_exec[string("seekg")] = &Shell::Func_Seekg;
    this->command_exec[string("tree")] = &Shell::Func_Tree;
    this->command_exec[string("logout")] = &Shell::Func_Logout;
}

void Shell::Prompt()
{
    // string work_directory = "/usr/local/bin";
    printf("\n# %s in %s\n$ ", g_user.username, current_path.c_str());
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

        // check if id matches password
        User user = FileSystem::Check_User(usr, psw);

        // wrong username or password
        if (user.uid == (unsigned short)(-1))
        {
            cout << "Please Check Your ID and Password, and Try Again!" << endl;
        }
        // log in successfully
        else
        {
            g_user = user;
            cout << "Log In Successfully";
            break;
        }
    }
}

void Shell::Run()
{
    if (!DiskDriver::Exists())
    {
        DiskDriver::Create_Disk();
    }
    g_user.uid = (unsigned short)(-1);

    cout << endl;
    cout << "Welcome To Virtual Unix File System!" << endl;
    cout << endl;

    current_path = "/";
    cout << "Erase All Data And Format The Disk?(y/n): ";
    string format;
    getline(cin, format);
    if (format == "y")
    {
        FileSystem::Format_Disk();
    }

    FileSystem::Boot();

    this->Init_Command_Exec();

    this->flag = true;

    while (1)
    {
        if (g_user.uid == (unsigned short)(-1))
        {
            this->Log_In();
            if (g_user.uid == 0)
            {
                current_path = "/";
            }
            else
            {
                current_path = "/Users/" + string(g_user.username);
            }
        }
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

void Shell::Func_Help()
{
}