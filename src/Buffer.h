#ifndef BUFFER_H
#define BUFFER_H

class Buffer
{
public:

    static void Read(int offset, char *mem_addr, int size);

    static void Write(int offset, char *mem_addr, int size);

};

#endif