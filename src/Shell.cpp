#include "Shell.h"
#include "DiskDriver.h"

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
    BufferManager::Read(blkno, 0, (char *)&dir, sizeof(Directory)); // No modifying, no storing

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
        BufferManager::Read(inode.i_addr[0], 0, (char *)&dir, sizeof(Directory));
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
    BufferManager::Read(inode.i_addr[0], 0, (char *)&dir, sizeof(Directory));

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
    if (args.size() == 1)
    {
        Inode inode;
        vector<string> path;
        Parse_Path(current_path, path);
        int dir_inode_num = Get_Inode_Num(path);
        FileSystem::Load_Inode(inode, dir_inode_num);
        Directory dir;
        BufferManager::Read(inode.i_addr[0], 0, (char *)&dir, sizeof(Directory));
        for (unsigned int i = 0; i < inode.i_size; i++)
        {
            Inode sub_dir_inode;
            unsigned int sub_dir_inode_num = dir.d_inode_num[i];
            FileSystem::Load_Inode(sub_dir_inode, sub_dir_inode_num);
            unsigned short mode = sub_dir_inode.i_mode;
            cout << dir.d_filename[i];
            cout << "  ";
        }
        cout << endl;
    }
    else if (args.size() > 2 || args[1] != "-l")
    {
        cout << "Incorrect Arguments For This Command" << endl;
    }
    else
    {
        const unsigned int time_len = 25;
        const unsigned int type_len = 15;
        const unsigned int size_len = 15;
        const unsigned int name_len = 25;
        const unsigned int owner_len = 25;
        const unsigned int permission_len = 25;
        cout << setw(time_len) << "Edit Time";
        cout << setw(type_len) << "Type";
        cout << setw(size_len) << "Size(Byte)";
        cout << setw(name_len) << "Name";
        cout << setw(owner_len) << "Owner";
        cout << setw(permission_len) << "Permission";
        cout << endl;
        Inode inode;
        vector<string> path;
        Parse_Path(current_path, path);
        int dir_inode_num = Get_Inode_Num(path);
        FileSystem::Load_Inode(inode, dir_inode_num);
        Directory dir;
        BufferManager::Read(inode.i_addr[0], 0, (char *)&dir, sizeof(Directory));
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
                cout << "<DIR>" << setw(size_len) << "-";
                cout << setw(name_len) << dir.d_filename[i];
                cout << setw(owner_len) << "-";
                cout << setw(permission_len - 8);
                cout << (sub_dir_inode.i_permission & Inode::Owner_R ? 'R' : '-');
                cout << (sub_dir_inode.i_permission & Inode::Owner_W ? 'W' : '-');
                cout << (sub_dir_inode.i_permission & Inode::Owner_E ? 'X' : '-');

                cout << (sub_dir_inode.i_permission & Inode::GROUP_R ? 'R' : '-');
                cout << (sub_dir_inode.i_permission & Inode::GROUP_W ? 'W' : '-');
                cout << (sub_dir_inode.i_permission & Inode::GROUP_E ? 'X' : '-');

                cout << (sub_dir_inode.i_permission & Inode::ELSE_R ? 'R' : '-');
                cout << (sub_dir_inode.i_permission & Inode::ELSE_W ? 'W' : '-');
                cout << (sub_dir_inode.i_permission & Inode::ELSE_E ? 'X' : '-');
            }
            else
            {
                cout << "<FILE>" << setw(size_len) << size;
                cout << setw(name_len) << dir.d_filename[i];
                // cout << endl

                User user;
                vector<string> register_file_path;
                register_file_path.push_back("root");
                register_file_path.push_back("etc");
                register_file_path.push_back("passwd");
                FileManager::Open_File(register_file_path);
                unsigned int usr_num;
                FileManager::Read_File(register_file_path, (char *)&usr_num, sizeof(unsigned int));
                for (unsigned int i = 0; i < usr_num; i++)
                {
                    // read out
                    User read_usr_tmp;
                    FileManager::Read_File(register_file_path, (char *)&read_usr_tmp, sizeof(User));

                    // if both match
                    if (read_usr_tmp.uid == sub_dir_inode.i_uid)
                    {
                        cout << setw(owner_len) << read_usr_tmp.username;
                        break;
                    }
                }
                FileManager::Close_File(register_file_path);
                cout << setw(permission_len - 8);
                cout << (sub_dir_inode.i_permission & Inode::Owner_R ? 'R' : '-');
                cout << (sub_dir_inode.i_permission & Inode::Owner_W ? 'W' : '-');
                cout << (sub_dir_inode.i_permission & Inode::Owner_E ? 'X' : '-');

                cout << (sub_dir_inode.i_permission & Inode::GROUP_R ? 'R' : '-');
                cout << (sub_dir_inode.i_permission & Inode::GROUP_W ? 'W' : '-');
                cout << (sub_dir_inode.i_permission & Inode::GROUP_E ? 'X' : '-');

                cout << (sub_dir_inode.i_permission & Inode::ELSE_R ? 'R' : '-');
                cout << (sub_dir_inode.i_permission & Inode::ELSE_W ? 'W' : '-');
                cout << (sub_dir_inode.i_permission & Inode::ELSE_E ? 'X' : '-');
            }
            cout << endl;
        }
    }
}

void Shell::Func_Exit()
{
    BufferManager::Flush();
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

void Shell::Func_Touch()
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
        cout << "You Cannot Delete This Directory!" << endl;
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
        cout << "Failed To Open It, For You Could Opened It Already Or It Does Not Exist";
        cout << " Or You Don't Have The Permission!" << endl;
    }
    else
    {
        cout << "File Successfully Opened!" << endl;
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

            delete[] content;

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
        delete[] content;

        fp.close();
    }
    else if (args[2] == "-s")
    {
        const int MAX_CONTENT_LEN = 500;
        char content[MAX_CONTENT_LEN];
        unsigned int tmp = FileManager::Read_File(path, content, atoi(args[3].c_str()));
        if (tmp > 0)
            cout << content << endl;
    }
}

void Shell::Func_Lseek()
{
    vector<string> path_t;
    Parse_Path(args[1], path_t);
    vector<string> path;
    Transform_Path(path_t, path);

    if (args.size() == 3)
    {
        FileManager::L_Seek(path, atoi(args[2].c_str()));
    }
    else
    {
        int tmp = FileManager::L_Seek_Pos(path);
        if (tmp == -1)
        {
            cout << "This File Doesn't Exist Or You Didn't Open This File!" << endl;
        }
        else
        {
            cout << "The File's R/W Pointer: " << tmp << endl;
        }
    }
}

void Shell::Func_Logout()
{
    FileManager::Empty_Open_File_Table();
    g_user.uid = (unsigned short)(-1);
}

void Shell::Func_Chmod()
{
    // Get the path vector
    string complete_path;
    if (args[1][0] != '/')
    {
        complete_path = current_path + "/" + args[1];
    }
    else
    {
        complete_path = args[1];
    }
    vector<string> path_vector;
    Parse_Path(complete_path, path_vector);

    // Get the inode
    unsigned int inode_num = Get_Inode_Num(path_vector);
    Inode inode;
    FileSystem::Load_Inode(inode, inode_num);

    // print the permission
    if (args.size() == 2)
    {
        // cout << "This File's Permission: ";
        cout << "Permission: ";

        cout << (inode.i_permission & Inode::Owner_R ? 'r' : '-');
        cout << (inode.i_permission & Inode::Owner_W ? 'w' : '-');
        cout << (inode.i_permission & Inode::Owner_E ? 'x' : '-');

        cout << ' ';

        cout << (inode.i_permission & Inode::GROUP_R ? 'r' : '-');
        cout << (inode.i_permission & Inode::GROUP_W ? 'w' : '-');
        cout << (inode.i_permission & Inode::GROUP_E ? 'x' : '-');

        cout << ' ';

        cout << (inode.i_permission & Inode::ELSE_R ? 'r' : '-');
        cout << (inode.i_permission & Inode::ELSE_W ? 'w' : '-');
        cout << (inode.i_permission & Inode::ELSE_E ? 'x' : '-');

        cout << endl;
        return;
    }
    else if (args[2].length() != 3)
    {
        cout << "Illegal Arguments!" << endl;
        return;
    }
    else if (g_user.uid != inode.i_uid && g_user.uid != 0)
    {
        cout << "Only The File's Owner Can Modify Its Permissions!" << endl;
        return;
    }
    else
    {

        unsigned short o = args[2][0] - '0';
        unsigned short g = args[2][1] - '0';
        unsigned short e = args[2][2] - '0';

        if (o<0 | g<0 | e<0 | o> 7 | g> 7 | e> 7)
        {
            cout << "Illegal Arguments" << endl;
            return;
        }

        unsigned short new_permission = o * 64 + g * 8 + e;

        inode.i_permission = new_permission;

        FileSystem::Store_Inode(inode, inode_num);
    }
}

void Shell::Func_Register()
{

    // read passwd file
    vector<string> register_file_path;
    register_file_path.push_back(string("root"));
    register_file_path.push_back(string("etc"));
    register_file_path.push_back(string("passwd"));
    FileManager::Open_File(register_file_path);
    FileManager::L_Seek(register_file_path, 0);

    // read out how many user this system have now
    unsigned int usr_num;
    FileManager::Read_File(register_file_path, (char *)&usr_num, sizeof(unsigned int));

    // new_usr.uid = usr_num;

    unsigned int i = 0;
    for (; i < usr_num; i++)
    {
        // read out a record
        User read_usr_tmp;
        FileManager::Read_File(register_file_path, (char *)&read_usr_tmp, sizeof(User));

        // if new username has been already created
        // register new user failed
        if (string(read_usr_tmp.username) == args[1])
        {
            cout << "User Already Exists!" << endl;
            break;
        }
    }

    if (i == usr_num)
    {

        if (args[1].length() > 14 || args[2].length() > 14)
        {
            cout << "Username Or Password Too Long!" << endl;
        }
        else
        {
            // create new User
            User new_usr;
            new_usr.gid = 1;
            new_usr.uid = usr_num++;
            strcpy(new_usr.username, args[1].c_str());
            strcpy(new_usr.password, args[2].c_str());

            // write number of user
            FileManager::L_Seek(register_file_path, 0);
            FileManager::Write_File(register_file_path, (char *)&usr_num, sizeof(unsigned int));

            // write new record of new user
            FileManager::L_Seek(register_file_path, sizeof(unsigned int) + (usr_num - 1) * sizeof(User));
            FileManager::Write_File(register_file_path, (char *)&new_usr, sizeof(User));

            // create new user directory
            vector<string> new_directory_path;
            new_directory_path.push_back(string("root"));
            new_directory_path.push_back(string("Users"));
            new_directory_path.push_back(args[1]);
            FileManager::Create_Dir(new_directory_path);
        }
    }

    FileManager::Close_File(register_file_path);

    //
}

void Shell::Func_Userlist()
{
    vector<string> register_file_path;
    register_file_path.push_back(string("root"));
    register_file_path.push_back(string("etc"));
    register_file_path.push_back(string("passwd"));
    FileManager::Open_File(register_file_path);
    FileManager::L_Seek(register_file_path, 0);

    unsigned int usr_num;
    FileManager::Read_File(register_file_path, (char *)&usr_num, sizeof(unsigned int));

    cout << "This System Has " << usr_num << " Registered Users Now:" << endl;
    for (unsigned int i = 0; i < usr_num; i++)
    {
        User read_usr_tmp;
        FileManager::Read_File(register_file_path, (char *)&read_usr_tmp, sizeof(User));

        cout << read_usr_tmp.username << endl;
    }

    FileManager::Close_File(register_file_path);
}

void Shell::Init_Command_Exec()
{
    this->command_exec[string("ls")] = &Shell::Func_Ls;
    this->command_exec[string("exit")] = &Shell::Func_Exit;
    this->command_exec[string("mkdir")] = &Shell::Func_Mkdir;
    this->command_exec[string("cd")] = &Shell::Func_Cd;
    this->command_exec[string("touch")] = &Shell::Func_Touch;
    this->command_exec[string("rmdir")] = &Shell::Func_Rmdir;
    this->command_exec[string("rm")] = &Shell::Func_Rm;
    this->command_exec[string("write")] = &Shell::Func_Write;
    this->command_exec[string("read")] = &Shell::Func_Read;
    this->command_exec[string("help")] = &Shell::Func_Help;
    this->command_exec[string("open")] = &Shell::Func_Open;
    this->command_exec[string("openlist")] = &Shell::Func_Openlist;
    this->command_exec[string("close")] = &Shell::Func_Close;
    this->command_exec[string("lseek")] = &Shell::Func_Lseek;
    this->command_exec[string("tree")] = &Shell::Func_Tree;
    this->command_exec[string("logout")] = &Shell::Func_Logout;
    this->command_exec[string("chmod")] = &Shell::Func_Chmod;
    this->command_exec[string("register")] = &Shell::Func_Register;
    this->command_exec[string("userlist")] = &Shell::Func_Userlist;
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
            cout << endl;
            cout << "Log In Successfully!" << endl;
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

    g_user.uid = 0;
    g_user.gid = 0;

    BufferManager::Init_Buffer_System();

    if (format == "y")
    {
        FileSystem::Format_Disk();
    }

    FileSystem::Boot();

    this->Init_Command_Exec();

    this->flag = true;

    g_user.uid = (unsigned short)(-1);
    g_user.gid = (unsigned short)(-1);
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
    if (args.size() == 1)
    {
        cout << "For More Information On A Specific Command, Type HELP Command-name." << endl
             << endl;
        cout << "cd             Changes The Current Directory." << endl;
        cout << "close          Close A File." << endl;
        cout << "chmod          Change Specific File's Access Permission." << endl;
        cout << "exit           Exit File System." << endl;
        cout << "help           Print Help Documentation." << endl;
        cout << "ls             List Files And Directories Included In Current Directory." << endl;
        cout << "logout         User Log Out." << endl;
        cout << "lseek          Move A File's R/W Pointer Or Print The Pointer's Offset." << endl;
        cout << "mkdir          Create A New Directory." << endl;
        cout << "open           Open A File." << endl;
        cout << "openlist       Print Files Which Have Been Already Opened." << endl;
        cout << "rmdir          Remove A Directory." << endl;
        cout << "rm             Remove A File." << endl;
        cout << "read           Read A File." << endl;
        cout << "register       Register A User." << endl;
        cout << "touch          Create A New File." << endl;
        cout << "tree           Print File Structure Tree Of Current Directory." << endl;
        cout << "userlist       Print User List The File System Now Registered." << endl;
        cout << "write          Write A File." << endl
             << endl;
        cout << "For More Information On A Specific Command, Type HELP Command-name." << endl;
    }
    else if (args.size() == 2)
    {
        if (args[1] == "cd")
        {
            cout << "Changes The Current Directory." << endl;
            cout << "Usage: cd <path>" << endl;
            cout << "This Command Supports Absolute/Relative Path." << endl;
            cout << "Eg: cd ../Users/dir0" << endl;
        }
        else if (args[1] == "close")
        {
            cout << "Close A File." << endl;
            cout << "Usage: close <file_path>" << endl;
            cout << "This Command Supports Absolute/Relative Path." << endl;
            cout << "Eg: close ../Users/file0" << endl;
        }
        else if (args[1] == "chmod")
        {
            cout << "Change Specific File's Access Permission." << endl;
            cout << "Usage: chmod <file_pat> <7~0>^3" << endl;
            cout << "This Command Supports Absolute/Relative Path." << endl;
            cout << "Eg: chmod ../Users/file0 644" << endl;
        }
        else if (args[1] == "exit")
        {
            cout << "Exit File System." << endl;
            cout << "Usage: exit" << endl;
        }
        else if (args[1] == "ls")
        {
            cout << "List Files And Directories Included In Current Directory." << endl;
            cout << "Usage: ls [-l]" << endl;
            cout << "-l: List Details Of This Directory." << endl;
        }
        else if (args[1] == "logout")
        {
            cout << "User Log Out." << endl;
            cout << "Usage: logout" << endl;
        }
        else if (args[1] == "lseek")
        {
            cout << "Move A File's R/W Pointer Or Print The Pointer's Offset." << endl;
            cout << "Usage: lseek <file_path> [offset_from_beg]" << endl;
            cout << "offset_from_beg: R/W Pointer's Offset From Beginning Of The File" << endl;
            cout << "This Command Supports Absolute/Relative Path." << endl;
            cout << "Eg: lseek ../Users/file0 10" << endl;
        }
        else if (args[1] == "mkdir")
        {
            cout << "Create A New Directory." << endl;
            cout << "Usage: mkdir <directory_path>" << endl;
            cout << "This Command Supports Absolute/Relative Path." << endl;
            cout << "Eg: mkdir ../Users/dir0" << endl;
        }
        else if (args[1] == "open")
        {
            cout << "Open A File." << endl;
            cout << "Usage: open <file_path>" << endl;
            cout << "This Command Supports Absolute/Relative Path." << endl;
            cout << "Eg: open ../Users/file0" << endl;
        }
        else if (args[1] == "openlist")
        {
            cout << "Print Files Which Have Been Already Opened." << endl;
            cout << "Usage: openlist" << endl;
        }
        else if (args[1] == "rmdir")
        {
            cout << "Remove A Directory." << endl;
            cout << "Usage: rmdir <directory_path>" << endl;
            cout << "This Command Supports Absolute/Relative Path." << endl;
            cout << "Eg: rmdir ../Users/dir0" << endl;
        }
        else if (args[1] == "rm")
        {
            cout << "Remove A File." << endl;
            cout << "Usage: rm <file_path>" << endl;
            cout << "This Command Supports Absolute/Relative Path." << endl;
            cout << "Eg: rm ../Users/file0" << endl;
        }
        else if (args[1] == "read")
        {
            cout << "Read A File." << endl;
            cout << "Usage: read <source_file_path> -s <size>" << endl;
            cout << "       read <source_file_path> -f <destination_file_path>" << endl;
            cout << "-s: Output To Screen" << endl;
            cout << "-f: Output To File" << endl;
            cout << "This Command Supports Absolute/Relative Path." << endl;
            cout << "Eg: read ../Users/file0 -s 10" << endl;
            cout << "    read ../Users/file0 -f destination" << endl;
        }
        else if (args[1] == "register")
        {
            cout << "Register A User." << endl;
            cout << "Usage: register <username> <password>" << endl;
            cout << "Eg: register Tom 123456" << endl;
        }
        else if (args[1] == "touch")
        {
            cout << "Create A File." << endl;
            cout << "Usage: touch <file_path>" << endl;
            cout << "This Command Supports Absolute/Relative Path." << endl;
            cout << "Eg: touch ../Users/file0" << endl;
        }
        else if (args[1] == "tree")
        {
            cout << "Print File Structure Tree Of Current Directory." << endl;
            cout << "Usage: tree" << endl;
        }
        else if (args[1] == "userlist")
        {
            cout << "Print User List The File System Now Registered." << endl;
            cout << "Usage: userlist" << endl;
        }
        else if (args[1] == "write")
        {
            cout << "Write A File." << endl;
            cout << "Usage: read <destination_file_path> -s <content>" << endl;
            cout << "       read <destination_file_path> -f <source_file_path>" << endl;
            cout << "-s: Screen Content Will Be Written Into Destination File" << endl;
            cout << "-f: Specific File's Content Will Be Written Into Destination File" << endl;
            cout << "This Command Supports Absolute/Relative Path." << endl;
            cout << "Eg: write ../Users/file0 -s HelloWorld!" << endl;
            cout << "    write ../Users/file0 -f source" << endl;
        }
        else
        {
            cout << "Wrong Instruction!" << endl;
        }
    }
    else
    {
        cout << "Wrong Instruction!" << endl;
    }
}