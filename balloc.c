/*
 * atheme-services: A collection of minimalist IRC services   
 * balloc.c: Block allocation of memory segments
 *
 * Copyright (c) 2005-2007 Atheme Project (http://www.atheme.org)           
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#define _BSD_SOURCE
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include "balloc.h"
#include "conf.h"
#include "dlink.h"
#include "evt.h"
#include "logger.h"
#include "memory.h"
#include "timestr.h"
#ifndef MAP_ANONYMOUS
#ifdef MAP_ANON
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

static int  blockheap_block_new(BlockHeap * bh);
static int  blockheap_garbagecollect(BlockHeap *);
static dlink_list heap_lists;

#define blockheap_fail(x) _blockheap_fail(x, __FILE__, __LINE__)

static void _blockheap_fail(const char *reason, const char *file, int line) {
   Log(LOG_INFO, "Blockheap failure: %s (%s:%d)", reason, file, line);
   conf.dying = 1;
}

/*
 * static void blockheap_block_free(void *ptr, size_t size)
 *
 * Inputs: The block and its size
 * Output: None
 * Side Effects: Returns memory for the block back to the OS
 */
static void blockheap_block_free(void *ptr, size_t size) {
   munmap(ptr, size);
}

/*
 * void blockheap_init(void)
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: Initializes the block heap
 */

static void blockheap_gc(int fd, short event, void *arg) {
   dlink_node *ptr, *tptr;

   DLINK_FOREACH_SAFE(ptr, tptr, heap_lists.head) {
      blockheap_garbagecollect(ptr->data);
   }
}

void blockheap_init(void) {
   evt_timer_add_periodic(blockheap_gc, "gc.blockheap",
                 nn2_timestr_to_time(dconf_get_str("tuning.timer.blockheap_gc", NULL), 60));
}

/*
 * static void *blockheap_block_get(size_t size)
 * 
 * Input: Size of block to allocate
 * Output: Pointer to new block
 * Side Effects: None
 */
static void *blockheap_block_get(size_t size) {
   void       *ptr;
   ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

   if (ptr == MAP_FAILED)
      ptr = NULL;

   return (ptr);
}

/* ************************************************************************ */
/* FUNCTION DOCUMENTATION:                                                  */
/*    blockheap_block_new                                                              */
/* Description:                                                             */
/*    Allocates a new block for addition to a blockheap                     */
/* Parameters:                                                              */
/*    bh (IN): Pointer to parent blockheap.                                 */
/* Returns:                                                                 */
/*    0 if successful, 1 if not                                             */
/* ************************************************************************ */

static int blockheap_block_new(BlockHeap * bh) {
   MemBlock   *newblk;
   Block      *b;
   unsigned long i;
   void       *offset;

   /*
    * Setup the initial data structure. 
    */
   b = (Block *) mem_calloc(1, sizeof(Block));

   if (b == NULL)
      return (1);

   b->free_list.head = b->free_list.tail = NULL;
   b->used_list.head = b->used_list.tail = NULL;
   b->next = bh->base;

   b->alloc_size = (bh->elemsPerBlock + 1) * (bh->elemSize + sizeof(MemBlock));

   b->elems = blockheap_block_get(b->alloc_size);

   if (b->elems == NULL)
      return (1);

   offset = b->elems;

   /*
    * Setup our blocks now 
    */
   for (i = 0; i < bh->elemsPerBlock; i++) {
      void       *data;
      newblk = (void *)offset;
      newblk->block = b;
#ifdef DEBUG_BALLOC
      newblk->magic = BALLOC_MAGIC;
#endif
      data = (void *)((size_t) offset + sizeof(MemBlock));
      newblk->block = b;
      dlink_add(data, &newblk->self, &b->free_list);
      offset = (unsigned char *)((unsigned char *)offset + bh->elemSize + sizeof(MemBlock));
   }

   ++bh->blocksAllocated;
   bh->freeElems += bh->elemsPerBlock;
   bh->base = b;

   return (0);
}

/* ************************************************************************ */
/* FUNCTION DOCUMENTATION:                                                  */
/*    BlockHeapCreate                                                       */
/* Description:                                                             */
/*   Creates a new blockheap from which smaller blocks can be allocated.    */
/*   Intended to be used instead of multiple calls to malloc() when         */
/*   performance is an issue.                                               */
/* Parameters:                                                              */
/*   elemsize (IN):  Size of the basic element to be stored                 */
/*   elemsperblock (IN):  Number of elements to be stored in a single block */
/*         of memory.  When the blockheap runs out of free memory, it will  */
/*         allocate elemsize * elemsperblock more.                          */
/*   name (IN):	Name (<= 63 bytes) of this heap                             */
/* Returns:                                                                 */
/*   Pointer to new BlockHeap, or NULL if unsuccessful                      */
/* ************************************************************************ */
BlockHeap  *blockheap_create(size_t elemsize, int elemsperblock, const char *name) {
   BlockHeap  *bh;

   /*
    * Catch idiotic requests up front 
    */
   if ((elemsize <= 0) || (elemsperblock <= 0)) {
      blockheap_fail("Attempting to BlockHeapCreate idiotic sizes");
   }

   /*
    * Allocate our new BlockHeap 
    */
   bh = (BlockHeap *) mem_calloc(1, sizeof(BlockHeap));

   if (bh == NULL) {
      Log(LOG_INFO, "Attempt to calloc() failed: (%s:%d)", __FILE__, __LINE__);
      conf.dying = 1;
   }

   if ((elemsize % sizeof(void *)) != 0) {
      /*
       * Pad to even pointer boundary 
       */
      elemsize += sizeof(void *);
      elemsize &= ~(sizeof(void *) - 1);
   }

   bh->elemSize = elemsize;
   bh->elemsPerBlock = elemsperblock;
   bh->blocksAllocated = 0;
   bh->freeElems = 0;
   bh->base = NULL;

   /*
    * Be sure our malloc was successful 
    */
   if (blockheap_block_new(bh)) {
      if (bh != NULL)
         mem_free(bh);
      Log(LOG_INFO, "blockheap_block_new() failed");
      conf.dying = 1;
   }

   if (bh == NULL) {
      blockheap_fail("bh == NULL when it shouldn't be");
   }

   if (name != NULL)
      memcpy(bh->name, name, sizeof(bh->name) - 1);
   dlink_add(bh, &bh->hlist, &heap_lists);
   return (bh);
}

/* ************************************************************************ */
/* FUNCTION DOCUMENTATION:                                                  */
/*    BlockHeapAlloc                                                        */
/* Description:                                                             */
/*    Returns a pointer to a struct within our BlockHeap that's free for    */
/*    the taking.                                                           */
/* Parameters:                                                              */
/*    bh (IN):  Pointer to the Blockheap.                                   */
/* Returns:                                                                 */
/*    Pointer to a structure (void *), or NULL if unsuccessful.             */
/* ************************************************************************ */

void       *blockheap_alloc(BlockHeap * bh) {
   Block      *walker;
   dlink_node *new_node;

   if (bh == NULL) {
      blockheap_fail("Cannot allocate if bh == NULL");
   }

   if (bh->freeElems == 0) {
      /*
       * Allocate new block and assign 
       */
      /*
       * blockheap_block_new returns 1 if unsuccessful, 0 if not 
       */

      if (blockheap_block_new(bh)) {
         /*
          * That didn't work..try to garbage collect 
          */
         blockheap_garbagecollect(bh);
         if (bh->freeElems == 0) {
            Log(LOG_FATAL, "blockheap_block_new() failed and garbage collection didn't help");
            conf.dying = 1;
         }
      }
   }

   for (walker = bh->base; walker != NULL; walker = walker->next) {
      if (DLINK_LENGTH(&walker->free_list) > 0) {
         bh->freeElems--;
         new_node = walker->free_list.head;
         dlink_move(new_node, &walker->free_list, &walker->used_list);
         if (new_node->data == NULL)
            blockheap_fail("new_node->data is NULL and that shouldn't happen!!!");
         memset(new_node->data, 0, bh->elemSize);
         return (new_node->data);
      }
   }
   blockheap_fail("BlockHeapAlloc failed, giving up");
   return NULL;
}

/* ************************************************************************ */
/* FUNCTION DOCUMENTATION:                                                  */
/*    BlockHeapFree                                                         */
/* Description:                                                             */
/*    Returns an element to the free pool, does not free()                  */
/* Parameters:                                                              */
/*    bh (IN): Pointer to BlockHeap containing element                      */
/*    ptr (in):  Pointer to element to be "freed"                           */
/* Returns:                                                                 */
/*    0 if successful, 1 if element not contained within BlockHeap.         */
/* ************************************************************************ */
int blockheap_free(BlockHeap * bh, void *ptr) {
   Block      *block;
   struct MemBlock *memblock;

   if (bh == NULL) {
      Log(LOG_DEBUG, "balloc.c:BlockHeapFree() bh == NULL");
      return (1);
   }

   if (ptr == NULL) {
      Log(LOG_DEBUG, "balloc.c:BlockHeapFree() ptr == NULL");
      return (1);
   }

   memblock = (void *)((size_t) ptr - sizeof(MemBlock));
#ifdef DEBUG_BALLOC
   if (memblock->magic != BALLOC_MAGIC) {
      blockheap_fail("memblock->magic != BALLOC_MAGIC");
      conf.dying = 1;
   }
#endif
   if (memblock->block == NULL) {
      blockheap_fail("memblock->block == NULL, not a valid block?");
      conf.dying = 1;
   }

   /* Just in case... */
   memset(ptr, 0, bh->elemSize);

   block = memblock->block;
   bh->freeElems++;
   dlink_move(&memblock->self, &block->used_list, &block->free_list);

   return (0);
}

/* ************************************************************************ */
/* FUNCTION DOCUMENTATION:                                                  */
/*    BlockHeapGarbageCollect                                               */
/* Description:                                                             */
/*    Performs garbage collection on the block heap.  Any blocks that are   */
/*    completely unallocated are removed from the heap.  Garbage collection */
/*    will never remove the root node of the heap.                          */
/* Parameters:                                                              */
/*    bh (IN):  Pointer to the BlockHeap to be cleaned up                   */
/* Returns:                                                                 */
/*   0 if successful, 1 if bh == NULL                                       */
/* ************************************************************************ */
static int blockheap_garbagecollect(BlockHeap * bh) {
   Block      *walker, *last;
   if (bh == NULL) {
      return (1);
   }

   if (bh->freeElems < bh->elemsPerBlock || bh->blocksAllocated == 1) {
      /*
       * There couldn't possibly be an entire free block.  Return. 
       */
      return (0);
   }

   last = NULL;
   walker = bh->base;

   while (walker != NULL) {
      if ((DLINK_LENGTH(&walker->free_list) == bh->elemsPerBlock) != 0) {
         blockheap_block_free(walker->elems, walker->alloc_size);
         if (last != NULL) {
            last->next = walker->next;
            if (walker != NULL)
               mem_free(walker);
            walker = last->next;
         } else {
            bh->base = walker->next;
            if (walker != NULL)
               mem_free(walker);
            walker = bh->base;
         }
         bh->blocksAllocated--;
         bh->freeElems -= bh->elemsPerBlock;
      } else {
         last = walker;
         walker = walker->next;
      }
   }
   return (0);
}

/* ************************************************************************ */
/* FUNCTION DOCUMENTATION:                                                  */
/*    BlockHeapDestroy                                                      */
/* Description:                                                             */
/*    Completely free()s a BlockHeap.  Use for cleanup.                     */
/* Parameters:                                                              */
/*    bh (IN):  Pointer to the BlockHeap to be destroyed.                   */
/* Returns:                                                                 */
/*   0 if successful, 1 if bh == NULL                                       */
/* ************************************************************************ */
int blockheap_destroy(BlockHeap * bh) {
   Block      *walker, *next;

   if (bh == NULL) {
      return (1);
   }

   for (walker = bh->base; walker != NULL; walker = next) {
      next = walker->next;
      blockheap_block_free(walker->elems, walker->alloc_size);
      if (walker != NULL)
         mem_free(walker);
   }
   dlink_delete(&bh->hlist, &heap_lists);
   mem_free(bh);
   return (0);
}

void blockheap_usage(BlockHeap * bh, size_t * bused, size_t * bfree, size_t * bmemusage) {
   size_t      used;
   size_t      freem;
   size_t      memusage;
   if (bh == NULL) {
      return;
   }

   freem = bh->freeElems;
   used = (bh->blocksAllocated * bh->elemsPerBlock) - bh->freeElems;
   memusage = used * (bh->elemSize + sizeof(MemBlock));

   if (bused != NULL)
      *bused = used;
   if (bfree != NULL)
      *bfree = freem;
   if (bmemusage != NULL)
      *bmemusage = memusage;
   Log(LOG_INFO, "Block Heap Allocator statistics: heap=%s used=%lu free=%lu memusage=%lu", bh->name,
     used, freem, memusage);
}

/* vim:cinoptions=>s,e0,n0,f0,{0,}0,^0,=s,ps,t0,c3,+s,(2s,us,)20,*30,gs,hs
 * vim:ts=8
 * vim:sw=8
 * vim:noexpandtab
 */
