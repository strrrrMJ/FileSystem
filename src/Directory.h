#ifndef DIRECTORY_H
#define DIRECTORY_H

static const int MAX_SUB_DIR_NUM = 12;
static const int MAX_FILE_NAME = 32;

class Directory
{
public:
    unsigned int d_inode_num[MAX_SUB_DIR_NUM];
    char d_filename[MAX_SUB_DIR_NUM][MAX_FILE_NAME];
};

#endif