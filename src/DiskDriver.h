#ifndef DISKDRIVER_H
#define DISKDRIVER_H

#include <fstream>

static const std::string DISK_NAME = "MyDisk.img";

class DiskDriver
{
private:
    static std::fstream fp;

public:
    static void Read(int offset, char *mem_addr, int size);

    static void Write(int offset, char *mem_addr, int size);

    // If the disk already exists, it will create and overwrite
    static void Create_Disk();

    static void Destroy_Disk();

    static bool Exists();
};

#endif