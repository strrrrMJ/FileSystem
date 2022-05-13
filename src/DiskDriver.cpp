#include "DiskDriver.h"

std::fstream DiskDriver::fp;

void DiskDriver::Read(int offset, char *mem_addr, int size)
{
    fp.open(DISK_NAME, std::ios::in | std::ios::out | std::ios::binary);
    if (!fp.is_open())
    {
        throw "THE DISK IS NOT EXIST!";
    }
    fp.seekg(offset, std::ios::beg);
    fp.read(mem_addr, size);
    fp.close();
}

void DiskDriver::Write(int offset, char *mem_addr, int size)
{
    fp.open(DISK_NAME, std::ios::in | std::ios::out | std::ios::binary);
    if (!fp.is_open())
    {
        throw "THE DISK IS NOT EXIST!";
    }
    fp.seekg(offset, std::ios::beg);
    fp.write(mem_addr, size);
    fp.close();
}

void DiskDriver::Create_Disk()
{
    fp.open(DISK_NAME, std::ios::out | std::ios::binary);
    if (!fp)
    {
        throw "ERROR WHEN CREATE DISK IMAGE!";
    }
    fp.close();
}

void DiskDriver::Destroy_Disk()
{
    if (remove(DISK_NAME.c_str()) != 0)
    {
        throw "ERROR DESTROYING THE DISK!";
    }
}

bool DiskDriver::Exists()
{
    fp.open(DISK_NAME, std::ios::in | std::ios::binary);
    bool ans;
    if (fp)
    {
        ans = true;
    }
    else
    {
        ans = false;
    }
    fp.close();
    return ans;
}
