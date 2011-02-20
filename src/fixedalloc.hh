#ifndef INCLUDED_Fixed_alloc_HH
#define INCLUDED_Fixed_alloc_HH

#include "assert.hh"

class FixedAlloc
{
    const int m_itemsPerBlock ;
    const int m_maxBlocks ;
    const int m_itemSize;
    int m_numBlocks ;
    char* m_blockHead;
    char* m_nextFree;
    
public:
    explicit FixedAlloc( int itemsPerBlock, int itemSize, int maxBlocks = -1) 
        : m_itemsPerBlock(itemsPerBlock)
        , m_maxBlocks(maxBlocks)
        , m_itemSize(itemSize)
        , m_numBlocks(0)
        , m_blockHead(0)
        , m_nextFree(0)
    {
        ASSERT( itemSize >= (int)sizeof(void*) ); // we're stuffing ptrs over top of unused Ts

    }

    ~FixedAlloc()
    {
        char* curBlock = m_blockHead;

        while(curBlock)
        {
            char* nextBlock = *reinterpret_cast<char**>(curBlock);
            delete[] curBlock;
            curBlock = nextBlock;
        }
    }

    void* Allocate()
    {
        if(m_nextFree == 0)
        {
            if(m_maxBlocks > 0 && m_numBlocks == m_maxBlocks) {
                return 0;
            }

            const int itemSize = m_itemSize;
            const int itemsPerBlock = m_itemsPerBlock;

            // header and free chunks
            char* blockHead = new char[ (1 + itemsPerBlock) * itemSize ];
            
            // update linked list of blocks
            char** pBlock = reinterpret_cast<char**>(blockHead);
            *pBlock = m_blockHead;
            m_blockHead = blockHead;

            // waste first one. This is in case T has some alignment requirements
            char* pT = blockHead + itemSize; 
            for(int i = 0; i < itemsPerBlock - 1 ; ++i)
            {
                char** ppCur = reinterpret_cast<char**>(pT + i * itemSize);
                *ppCur = reinterpret_cast<char*>(ppCur) + itemSize;
            }

            // last one is the end of the block
            *reinterpret_cast<char**>(pT + (itemsPerBlock - 1) * itemSize) = 0; 

            m_nextFree = pT;
            ++m_numBlocks;
        }

        void* result = m_nextFree;
        m_nextFree = *reinterpret_cast<char**>(m_nextFree);
        return result;
    }

    void Free(void* p)
    {
        *reinterpret_cast<char**>(p) = m_nextFree;
        m_nextFree = reinterpret_cast<char*>(p);
    }
};

template<class T>
class FixedAllocTyped
{
    FixedAlloc m_fixedAlloc;
public:
    FixedAllocTyped( int itemsPerBlock, int maxBlocks = -1) :
        m_fixedAlloc(itemsPerBlock, sizeof(T), maxBlocks)
        {
        }
    ~FixedAllocTyped() {}

    T* Allocate()
    {
        void* result = reinterpret_cast<T*>(m_fixedAlloc.Allocate());
        return new (result) T;
    }

    void Free(T* p)
    {
        p->~T();
        m_fixedAlloc.Free(p);
    }

};

#endif

