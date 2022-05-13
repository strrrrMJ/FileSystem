#include "Buffer.h"
#include "DiskDriver.h"

void Buffer::Read(int offset, char *mem_addr, int size)
{
    DiskDriver::Read(offset, mem_addr, size);
}

void Buffer::Write(int offset, char *mem_addr, int size)
{
    DiskDriver::Write(offset, mem_addr, size);
}
