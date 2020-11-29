#ifndef __ORANGE_ALLOC_H__
#define __ORANGE_ALLOC_H__

#include <new>
#include <cstddef>
#include <cstdio>

namespace orange_stl
{
    //联合体：FreeList
    //采用链表的方式管理内存，分配与回收小内存(<=4k)的内存块
    union FreeList
    {
        union FreeList *next;   //指向下一个内存块
        char data[1];          //存储本快内存的首地址
    };
    /* 从第一个字段来看，FreeList可以被看作是一个指针，指向相同形式的另一个FreeList
       从第二个字段来看，FreeList可以被看作是一个指针，指向实际的内存区域，避免维护链表指针造成内存的浪费
     */

    //不同范围的上调大小
    enum
    {
        EAlign128=8,
        EAlign256=16,
        EAlign512=32,
        EAlign1024=64,
        EAlign2048=128,
        EAlign4096=256
    };

    //小对象的内存大小
    enum { ESmallObjectBytes = 4096 };

    //FreeList的个数
    /*  区间        间隔    个数 
        0-128       8       16  
        128-256     16      8
        265-512     32      8
        512-1024    64      8
        1024-2048   128     8
        2048-4096   256     8

        加起来总共有56个链表     
     */
    enum { EFreeListsNumber = 56 };

    //空间配置类alloc
    //当内存较大的时候(>4096)，直接调用std::malloc与std::free
    //当内存较小的时候，以内存池管理，每次配置一块大的内存，并维护对应的自由链表
    class alloc
    {
    private:
        static char* start_free;    //内存池起始位置
        static char* end_free;      //内存池结束位置
        static size_t heap_size;     //申请heap空间附加值的大小

        static FreeList* free_list[EFreeListsNumber];    //自由链表

    private:
        static size_t O_align(size_t bytes);
        static size_t O_round_up(size_t bytes);
        static size_t O_freelist_index(size_t bytes);
        static void* O_refill(size_t n);
        static char* O_chunk_alloc(size_t size, size_t &nblock);
    public:
        static void* allocate(size_t n);
        static void  deallocate(void *p, size_t n);
        static void* reallocate(void *p, size_t old_size, size_t new_size);
    };

    //静态成员变量的初始化
    char*  alloc::start_free = nullptr;
    char*  alloc::end_free = nullptr;
    size_t alloc::heap_size = 0;

    FreeList* alloc::free_list[EFreeListsNumber] = {
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr
    };

    //分配大小为n的空间，n>0
    inline void* alloc::allocate(size_t n)
    {
        FreeList* my_free_list;
        FreeList* result;

        if(n>static_cast<int>(ESmallObjectBytes))
            return std::malloc(n);
        
        my_free_list = free_list[O_freelist_index(n)];
        result = my_free_list;
        if(!result) //自由链表的该内存块位置为空
        {
            void* res=O_refill(O_round_up(n));
            return res;
        }
        my_free_list=result->next;
        return result;
    }

    //释放p指向的大小为n的空间，p不能为0
    inline void  alloc::deallocate(void *p, size_t n)
    {
        if(n>static_cast<size_t>(ESmallObjectBytes))
        {
            std::free(p);
            return;
        }
        FreeList *q=reinterpret_cast<FreeList*>(p);
        FreeList *my_free_list;
        my_free_list=free_list[O_freelist_index(n)];
        q->next=my_free_list;
        my_free_list=q;
    }
    //重新分配空间，接受三个参数，参数1位指向新空间的指针，参数2为原来空间的大小，参数3为申请空间的大小
    inline void* alloc::reallocate(void *p, size_t old_size, size_t new_size)
    {
        deallocate(p, old_size);
        p=allocate(new_size);
        return p;
    }
    //上调bytes的大小
    size_t alloc::O_align(size_t bytes)
    {
        if (bytes <= 512)
        {
            return bytes <= 256
            ? bytes <= 128 ? EAlign128 : EAlign256
            : EAlign512;
        }
        return bytes <= 2048
            ? bytes <= 1024 ? EAlign1024 : EAlign2048
            : EAlign4096;
    }
    //将byte上调至对应区间的大小
    size_t alloc::O_round_up(size_t bytes)
    {
        return ((bytes+O_align(bytes)-1)&~(O_align(bytes)-1));
    }
    //根据区块的大小，选择第n个free lists
    size_t alloc::O_freelist_index(size_t bytes)
    {
        if (bytes <= 512)
        {
            return bytes <= 256
            ? bytes <= 128 
                ? ((bytes + EAlign128 - 1) / EAlign128 - 1) 
                : (15 + (bytes + EAlign256 - 129) / EAlign256)
            : (23 + (bytes + EAlign512 - 257) / EAlign512);
        }
        return bytes <= 2048
            ? bytes <= 1024 
            ? (31 + (bytes + EAlign1024 - 513) / EAlign1024)
            : (39 + (bytes + EAlign2048 - 1025) / EAlign2048)
            : (47 + (bytes + EAlign4096 - 2049) / EAlign4096);
    }
    //重新填充freelist,单个对象的大小为n
    void* alloc::O_refill(size_t n)
    {
        //缺省取得10个新节点（新区块），若内存池空间不足，可能取得数量少于10
        size_t nblock=10;
        char *c=O_chunk_alloc(n, nblock);

        FreeList* my_free_list;
        FreeList* result, *cur, *next;
        //如果只有一个内存块，就把这个内存块返回给调用者，freelist没有增加新的节点
        if(nblock==1)
            return c;
        //否则把一个区块给调用者，剩下的纳入free list作为新的节点
        my_free_list=free_list[O_freelist_index(n)];
        result=(FreeList*)c;
        //使freelist指向新配置的空间
        my_free_list=next=(FreeList*)(c+n);

        //以下将freelist的各个节点串接起来，从1开始，第0个返回给客户端
        for(size_t i=1; ;++i)
        {
            cur=next;
            next=(FreeList*)((char*)next+n);
            if(i==nblock-1)
            {
                cur->next=nullptr;
                break;
            }
            else
            {
                cur->next=next;
            }
        }
        return result;
    }
    //从内存池中取空间给free list，条件不允许时，会调整nblock
    char* alloc::O_chunk_alloc(size_t size, size_t &nblock)
    {
        char *result;
        size_t need_bytes = size*nblock;
        size_t pool_bytes = end_free - start_free;

        //如果内存池大小完全满足需求量，返回该值
        if(pool_bytes >= need_bytes)
        {
            result=start_free;
            start_free += need_bytes;
            return result;
        }
        // 如果内存池剩余大小不能完全满足需求量，但是可以分配至少一个或者一个以上的内存块，就返回它
        else if(pool_bytes > size)
        {
            nblock = pool_bytes / size;
            need_bytes = size * nblock;
            result=start_free;
            start_free += need_bytes;
            return result;
        }
        //如果内存池的剩余大小连一个区块都无法满足
        else
        {
            if(pool_bytes > 0)
            {
                //若内存池还有剩余，就先把剩余的空间加入到free list中
                FreeList *my_free_list = free_list[O_freelist_index(pool_bytes)];
                ((FreeList*)start_free)->next=my_free_list;
                my_free_list=(FreeList*)start_free;
            }
            //申请堆栈空间
            size_t bytes_to_get=(need_bytes<<1)+O_round_up(heap_size>>4);
            start_free = (char*)std::malloc(bytes_to_get);
            if(!start_free)
            {
                //堆空间不够用
                FreeList *my_free_list, *p;
                //试着查找有无未用的区块，且足够大的free list   
                for(size_t i=size; i<ESmallObjectBytes; i+=O_align(i))
                {
                    my_free_list=free_list[O_freelist_index(i)];
                    p=my_free_list;
                    if(p)
                    {
                        my_free_list=p->next;
                        start_free=(char*)p;
                        end_free=start_free+i;
                        return O_chunk_alloc(size, nblock);
                    }
                }
                std::printf("out of memory!!!");
                end_free=nullptr;
                throw std::bad_alloc();
            }
            end_free=start_free+bytes_to_get;
            heap_size += bytes_to_get;
            return O_chunk_alloc(size, nblock);
        }
    }
}
#endif // !__ORANGE_ALLOC_H__