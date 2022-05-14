#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

static const unsigned int BLOCK_SIZE = 512;

static const unsigned int BUFFER_NUM = 10000;
// 0x423670
struct Buf
{
    unsigned int b_blkno;
    bool dirty;
    Buf *back;
    Buf *forw;

    char b_data[BLOCK_SIZE];
};

class BufferManager
{
private:
    static Buf buf_pool[BUFFER_NUM];
    static Buf freelist;
    static Buf uselist;

private:
    static void Free_Buffer(Buf *);

    static Buf *Allocate_Buffer();

    static Buf *Fetch_Buffer(unsigned int);

    static void LRU(Buf *);

public:
    // static void Read(int offset, char *mem_addr, int size);

    // static void Write(int offset, char *mem_addr, int size);

    static void Read(unsigned int, unsigned int, char *, unsigned int);

    static void Write(unsigned int, unsigned int, char *, unsigned int);

    static void Init_Buffer_System();
};

#endif