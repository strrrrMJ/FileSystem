#ifndef FILE_H
#define FILE_H

class File
{
public:
    unsigned int f_inode_id;

    unsigned int f_offset; // The r/w pointer's position

    unsigned int f_uid; // The user who open the file
};

#endif