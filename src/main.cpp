#include "FileSystem.h"
#include "User.h"
#include "Shell.h"
#include <iostream>
FileSystem g_filesystem;
int main()
{
    try
    {
        Shell shell;
        shell.Run();
    }
    catch (const char *e)
    {
        std::cout << e << std::endl;
    }
    return 0;
}