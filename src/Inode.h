#ifndef INODE_H
#define INODE_H

#include <ctime>

// The number of pointers in inode
static const unsigned int PTR_NUM = 10;
// The number of direct pointers in inode
static const unsigned int DIRECT_PTR_NUM = 6;
// The number of secondary pointers in inode
static const unsigned int SEC_PTR_NUM = 2;
// The number of tertiary pointers in inode
static const unsigned int TER_PTR_NUM = 2;

static const unsigned int PTR_IN_BLOCK_NUM = 128;

class Inode
{
public:
    enum Mode
    {
        IsDir = 0x1,
        IsFile = 0x0
    };
    enum Permission
    {
        Owner_R = 0x100,
        Owner_W = 0x80,
        Owner_E = 0x40,
        GROUP_R = 0x20,
        GROUP_W = 0x10,
        GROUP_E = 0x8,
        ELSE_R = 0x4,
        ELSE_W = 0x2,
        ELSE_E = 0x1
    };

    unsigned int i_addr[PTR_NUM];

    unsigned int i_size; // The number of bytes file contains

    unsigned short i_count; // Number of opened file handles

    unsigned short i_number; // The number of Inode

    unsigned short i_mode; // Directroy(1) or File(0)

    unsigned short i_permission;

    unsigned short i_uid; // The file's owner's id

    unsigned short i_gid; // The group which the owner belongs to

    time_t i_time; // last modified time

public:
    // file offset to disk index (byte as unit)
    unsigned int Offset_To_Index(unsigned int offset);

public:
    friend class FileSystem;
};

#endif