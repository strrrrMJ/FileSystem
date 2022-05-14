#include <cstring>
#include "DiskDriver.h"
#include "BufferManager.h"
#include <iostream>
using namespace std;
Buf BufferManager::freelist;
Buf BufferManager::uselist;
Buf BufferManager::buf_pool[BUFFER_NUM];

void BufferManager::Init_Buffer_System()
{
    // cout << "Init Buffer System..." << endl;
    uselist.forw = uselist.back = &uselist;
    uselist.b_blkno = -1;
    // cout << "Init Freelist..." << endl;
    Buf *tmp1 = &freelist;
    for (int i = 0; i < BUFFER_NUM; i++)
    {
        tmp1->forw = &buf_pool[i];
        tmp1 = tmp1->forw;
    }
    tmp1->forw = &freelist;
    // cout << "Init Freelist Successfully!" << endl;
    Buf *tmp2 = &freelist;
    // cout << "Init Uselist..." << endl;
    for (int i = BUFFER_NUM - 1; i >= 0; i--)
    {
        tmp2->back = &buf_pool[i];
        tmp2 = tmp2->back;
    }
    tmp2->back = &freelist;
    freelist.b_blkno = -1;
    // cout << "Init Uselist Successfully!" << endl;
    // cout << "Init Buffer System Successfully!" << endl;
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

    // add it into freelist
    buf->forw = &freelist;
    buf->back = freelist.back;
    freelist.back->forw = buf;
    freelist.back = buf;

    buf->dirty = false;
}

Buf *BufferManager::Allocate_Buffer()
{
    // if there'a any free buffer remained
    // if (freelist.forw != &freelist)
    // {
    //     Buf *allocate_buf = freelist.forw;
    //     freelist.forw->forw->back = &freelist;
    //     freelist.forw = freelist.forw->forw;

    //     allocate_buf->forw = &uselist;
    //     allocate_buf->back = uselist.back;
    //     uselist.back->forw = allocate_buf;
    //     uselist.back = allocate_buf;

    //     allocate_buf->dirty = false;
    //     return allocate_buf;
    // }
    // else
    // {
    //     if (uselist.forw == &uselist)
    //     {
    //         throw "Out Of Buffer!";
    //     }
    //     else
    //     {
    //         Buf *free_buf = uselist.forw;
    //         // Free_Buffer(uselist.forw);

    //         return free_buf;
    //     }
    // }

    if (freelist.forw == &freelist)
    {
        if (uselist.forw == &uselist)
        {
            throw "Out Of Buffer!";
        }
        else
        {
            Buf *free_buf = uselist.forw;
            Free_Buffer(uselist.forw);
        }
    }
    Buf *allocate_buf = freelist.forw;

    allocate_buf->forw->back = allocate_buf->back;
    allocate_buf->back->forw = allocate_buf->forw;

    allocate_buf->forw = &uselist;
    allocate_buf->back = uselist.back;
    uselist.back->forw = allocate_buf;
    uselist.back = allocate_buf;

    allocate_buf->dirty = false;
    return allocate_buf;
}

Buf *BufferManager::Fetch_Buffer(unsigned int blk_no)
{
    // cout << "blk_no" << blk_no << endl;
    // cout << "Fetch Buffer..." << endl;

    Buf *p = &uselist;
    p = p->back;

    Buf *res = NULL;
    while (p != &uselist)
    {
        // cout << "!!!!!!!!!!!" << endl;
        if (p->b_blkno == blk_no)
        {
            res = p;
            break;
        }
        p = p->back;
    }
    // int c;
    // cin >> c;
    // cout << "Fetch Return" << ((res == NULL) ? 876766 : res->b_blkno) << endl
    //      << endl;
    // int b;
    // cin >> b;
    // int d;
    // cin >> d;
    // int e;
    // cin >> e;
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

void BufferManager::LRU(Buf *buf)
{
    buf->forw->back = buf->back;
    buf->back->forw = buf->forw;

    buf->forw = &uselist;
    buf->back = uselist.back;
    uselist.back->forw = buf;
    uselist.back = buf;
}

void BufferManager::Read(unsigned int blk_no, unsigned int offset, char *mem_addr, unsigned int size)
{
    Buf *buf = Fetch_Buffer(blk_no);
    if (!buf)
    {
        // cout << "Allocate Buffer" << endl;
        buf = Allocate_Buffer();
        buf->b_blkno = blk_no;

        DiskDriver::Read(buf->b_blkno * BLOCK_SIZE, (char *)buf->b_data, BLOCK_SIZE);
    }

    memcpy(mem_addr, buf->b_data + offset, size);
    LRU(buf);
}

void BufferManager::Write(unsigned int blk_no, unsigned int offset, char *mem_addr, unsigned int size)
{

    Buf *buf = Fetch_Buffer(blk_no);
    if (!buf)
    {
        buf = Allocate_Buffer();
        buf->b_blkno = blk_no;

        DiskDriver::Read(buf->b_blkno * BLOCK_SIZE, (char *)buf->b_data, BLOCK_SIZE);
    }

    memcpy(buf->b_data + offset, mem_addr, size);
    if (size != 0)
    {
        buf->dirty = true;
    }
    LRU(buf);
}

void BufferManager::Flush()
{
    Buf *p_buf = uselist.forw;
    while (p_buf != &uselist)
    {
        if (p_buf->dirty)
        {
            DiskDriver::Write(p_buf->b_blkno * BLOCK_SIZE, (char *)p_buf->b_data, BLOCK_SIZE);
        }
        p_buf = p_buf->forw;
    }
}

// #include <cstring>
// #include "DiskDriver.h"
// #include "BufferManager.h"
// #include <iostream>

// using namespace std;

// BufferBlock BufferManager::buffer_pool[BUFFER_BLOCK_NUM];

// BufferBlock *BufferManager::head;

// void BufferManager::Init_Buffer_System()
// {
//     head = &buffer_pool[0];

//     for (int i = 0; i < BUFFER_BLOCK_NUM; i++)
//     {
//         buffer_pool[i].valid = false;
//     }

//     for (int i = 0; i <= BUFFER_BLOCK_NUM - 2; i++)
//     {
//         buffer_pool[i].forward = &buffer_pool[i + 1];
//     }
//     buffer_pool[BUFFER_BLOCK_NUM - 1].forward = &buffer_pool[0];

//     for (int i = 1; i <= BUFFER_BLOCK_NUM - 1; i++)
//     {
//         buffer_pool[i].backward = &buffer_pool[i - 1];
//     }
//     buffer_pool[0].backward = &buffer_pool[BUFFER_BLOCK_NUM - 1];
// }

// void BufferManager::Write_All_Buffers_To_Disk()
// {
//     BufferBlock *ptr = head;
//     while (ptr->forward != head)
//     {
//         if (ptr->valid && ptr->dirty)
//         {
//             DiskDriver::Write(ptr->blkno * BLOCK_SIZE, (char *)head->data, BLOCK_SIZE);
//         }
//         ptr->valid = false;
//     }
//     if (ptr->valid && ptr->dirty)
//     {
//         DiskDriver::Write(ptr->blkno * BLOCK_SIZE, (char *)head->data, BLOCK_SIZE);
//     }
//     ptr->valid = false;
// }

// void BufferManager::Read(unsigned int blkno, unsigned int offset, char *mem_addr, unsigned int size)
// {
//     BufferBlock *buf = Fetch_Buffer(blkno);
//     memcpy(mem_addr, buf->data + offset, size);
//     buf->blkno = blkno;
//     buf->valid = true;
//     buf->dirty = false;
//     Tail_Insert(buf);
// }

// void BufferManager::Write(unsigned int blkno, unsigned int offset, char *mem_addr, unsigned int size)
// {

//     BufferBlock *buf = Fetch_Buffer(blkno);
//     memcpy(buf->data + offset, mem_addr, size);
//     buf->blkno = blkno;
//     buf->valid = true;
//     buf->dirty = true;
//     Tail_Insert(buf);
// }

// void BufferManager::Tail_Insert(BufferBlock *buf)
// {
//     buf->forward = head;
//     buf->backward = head->backward;
//     head->backward->forward = buf;
//     head->backward = buf;
// }

// BufferBlock *BufferManager::Fetch_Buffer(unsigned int blkno)
// {
//     BufferBlock *ptr = head;
//     while (ptr->backward != head && (ptr->valid == false || ptr->blkno != blkno))
//     {
//         ptr = ptr->backward;
//     }
//     if (ptr->backward != head)
//     {
//         ptr->backward->forward = ptr->forward;
//         ptr->forward->backward = ptr->backward;
//         return ptr;
//     }
//     else
//     {
//         if (head->valid == true && head->dirty == true)
//         {
//             DiskDriver::Write(head->blkno * BLOCK_SIZE, (char *)head->data, BLOCK_SIZE);
//         }
//         DiskDriver::Read(blkno * BLOCK_SIZE, (char *)head->data, BLOCK_SIZE);

//         head->backward->forward = head->forward;
//         head->forward->backward = head->backward;

//         head = head->forward;

//         return head;
//     }
// }