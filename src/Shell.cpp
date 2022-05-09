#include "Shell.h"

#include <string>

void Shell::prompt(){
    std::string user_name = "vw";
    std::string work_directory = "/usr/local/bin";
    printf("# %s in %s\n$ ",user_name.c_str(),work_directory.c_str());
}