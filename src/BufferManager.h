#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

class BufferManager
{
private:

public:
    static void Read(int offset, char *mem_addr, int size);

    static void Write(int offset, char *mem_addr, int size);
};

#endif