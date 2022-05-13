#include <cstring>
#include "DiskDriver.h"
#include "BufferManager.h"
#include <iostream>
using namespace std;
Buf BufferManager::buf_pool[BUFFER_NUM];
Buf BufferManager::freelist;
Buf BufferManager::uselist;

void BufferManager::Init_Buffer_System()
{
    cout << "Init Buffer System..." << endl;
    uselist.forw = uselist.back = &uselist;
    cout << "Init Freelist..." << endl;
    Buf *tmp1 = &freelist;
    for (int i = 0; i < BUFFER_NUM; i++)
    {
        tmp1->forw = &buf_pool[i];
        tmp1 = tmp1->forw;
    }
    tmp1->forw = &freelist;
    cout << "Init Freelist Successfully!" << endl;
    Buf *tmp2 = &freelist;
    cout << "Init Uselist..." << endl;
    for (int i = BUFFER_NUM - 1; i >= 0; i--)
    {
        tmp2->back = &buf_pool[i];
        tmp2 = tmp2->back;
        cout << i << endl;
    }
    tmp2->back = &freelist;
    cout << "Init Uselist Successfully!" << endl;
    cout << "Init Buffer System Successfully!" << endl;
}

void BufferManager::Free_Buffer(Buf *buf)
{
    // write back to disk
    if (buf->dirty)
    {
        DiskDriver::Write(buf->b_blkno * BLOCK_SIZE, (char *)buf->b_data, BLOCK_SIZE);
    }

    // delete it from uselist
    buf->forw->back = buf->back;
    buf->back->forw = buf->forw;

    buf->back = buf->forw = NULL;

    // add it into freelist
    buf->forw = &freelist;
    buf->back = freelist.back;
    freelist.back->forw = buf;

    buf->dirty = false;
    freelist.back = buf;
}

Buf *BufferManager::Allocate_Buffer()
{
    // if there'a any free buffer remained
    if (freelist.forw != &freelist)
    {
        Buf *allocate_buf = freelist.forw;
        freelist.forw->forw->back = &freelist;
        freelist.forw = freelist.forw->forw;

        allocate_buf->forw = &uselist;
        allocate_buf->back = uselist.back;
        uselist.back->forw = allocate_buf;
        uselist.back = allocate_buf;

        allocate_buf->dirty = false;
        return allocate_buf;
    }
    else
    {
        if (uselist.forw == &uselist)
        {
            throw "Out Of Buffer!";
        }
        else
        {
            Buf *free_buf = uselist.forw;
            Free_Buffer(uselist.forw);
            return free_buf;
        }
    }
}

Buf *BufferManager::Fetch_Buffer(unsigned int blk_no)
{
    Buf *p = &uselist;
    p = p->back;

    Buf *res = NULL;
    while (p != &uselist)
    {
        if (p->b_blkno == blk_no)
        {
            res = p;
            break;
        }
        p = p->back;
    }
    return res;
}

// void BufferManager::Read(unsigned int blk_no, unsigned int offset, char *mem_addr, unsigned int size)
// {
//     DiskDriver::Read(blk_no * BLOCK_SIZE + offset, mem_addr, size);
// }

// void BufferManager::Write(unsigned int blk_no, unsigned int offset, char *mem_addr, unsigned int size)
// {
//     DiskDriver::Write(blk_no * BLOCK_SIZE + offset, mem_addr, size);
// }

void BufferManager::Read(unsigned int blk_no, unsigned int offset, char *mem_addr, unsigned int size)
{
    Buf *buf = Fetch_Buffer(blk_no);
    if (!buf)
    {
        buf = Allocate_Buffer();
        buf->b_blkno = blk_no;

        DiskDriver::Read(buf->b_blkno, (char *)buf->b_data, sizeof(BLOCK_SIZE));
    }

    memcpy(mem_addr, buf->b_data + offset, size);
}

void BufferManager::Write(unsigned int blk_no, unsigned int offset, char *mem_addr, unsigned int size)
{

    Buf *buf = Fetch_Buffer(blk_no);
    if (!buf)
    {
        buf = Allocate_Buffer();
        buf->b_blkno = blk_no;

        DiskDriver::Read(buf->b_blkno, (char *)buf->b_data, sizeof(BLOCK_SIZE));
    }

    memcpy(buf->b_data + offset, mem_addr, size);
}
