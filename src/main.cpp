#include "FileSystem.h"
#include "User.h"
#include "Shell.h"
FileSystem g_filesystem;
int main()
{
    Shell shell;
    shell.Run();
    return 0;
}