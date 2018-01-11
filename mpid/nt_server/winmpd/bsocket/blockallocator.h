#ifndef BLOCKALLOCATOR_H
#define BLOCKALLOCATOR_H

typedef struct BlockAllocator_struct * BlockAllocator;

BlockAllocator BlockAllocInit(unsigned int blocksize, int count, int incrementsize, void *(* alloc_fn)(unsigned int size), void (* free_fn)(void *p));
int BlockAllocFinalize(BlockAllocator *p);
void * BlockAlloc(BlockAllocator p);
int BlockFree(BlockAllocator p, void *pBlock);

#endif
