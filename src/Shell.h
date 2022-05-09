#ifndef SHELL_H
#define SHELL_H

#include "Directory.h"

class Shell
{
private:
    Directory current_dirrectory;

public:
    void prompt();
};

#endif