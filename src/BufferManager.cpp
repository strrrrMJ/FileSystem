#include "BufferManager.h"
#include "DiskDriver.h"

void BufferManager::Read(int offset, char *mem_addr, int size)
{
    DiskDriver::Read(offset, mem_addr, size);
}

void BufferManager::Write(int offset, char *mem_addr, int size)
{
    DiskDriver::Write(offset, mem_addr, size);
}
