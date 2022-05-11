#include "FileSystem.h"
#include "User.h"
#include "Shell.h"
<<<<<<< HEAD

#include <iostream>

using namespace std;

=======
#include <iostream>
>>>>>>> 3061ef49dbf050a66a328c6536ad788f0ac930ee
FileSystem g_filesystem;

int main()
{
    try
    {
        Shell shell;
        shell.Run();
    }
<<<<<<< HEAD
    catch (const char *msg)
    {
        cout << msg << endl;
=======
    catch (const char *e)
    {
        std::cout << e << std::endl;
>>>>>>> 3061ef49dbf050a66a328c6536ad788f0ac930ee
    }
    return 0;
}