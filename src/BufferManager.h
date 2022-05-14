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

    static void Flush();
};

#endif

// #ifndef BUFFERMANAGER_H
// #define BUFFERMANAGER_H

// static const unsigned int BLOCK_SIZE = 512;

// static const unsigned int BUFFER_BLOCK_NUM = 1000;

// struct BufferBlock
// {
//     // The block number
//     unsigned int blkno;

//     // Record whether it's a dirty buffer block
//     bool dirty;

//     // Record whether it's a valid buffer block
//     bool valid;

//     // Pointers used in ring linked list
//     BufferBlock *forward;
//     BufferBlock *backward;

//     // The data region
//     unsigned char data[BLOCK_SIZE];
// };

// class BufferManager
// {

// private:

//     // Blocks' linked list
//     static BufferBlock *head;

//     // The static array of buffer blocks
//     static BufferBlock buffer_pool[BUFFER_BLOCK_NUM];

// private:
//     static BufferBlock *Fetch_Buffer(unsigned int);

//     static void Tail_Insert(BufferBlock*);

// public:
//     // Buffer R/W Interface for external use
//     static void Read(unsigned int, unsigned int, char *, unsigned int);
//     static void Write(unsigned int, unsigned int, char *, unsigned int);

//     // Init when filesystem boot
//     static void Init_Buffer_System();

//     // Write all buffers to disk before exit
//     static void Write_All_Buffers_To_Disk();

// public:
// };

// #endif