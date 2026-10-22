/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ocean",
    /* First member's full name */
    "sanhemu",
    /* First member's email address */
    "sanhemu@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 1)

#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define PREV_FREE(bp) (*(char **)(bp))
#define NEXT_FREE(bp) (*(char **)((char *)(bp) + WSIZE))

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void place(void *bp, size_t asize);
static void *first_fit(size_t size);
static void insert_block(void *bp);
static void delete_block(void *bp);
static char *heap_listp;
static char *free_listp;
/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
    {
        return -1;
    }
    PUT(heap_listp, 0);
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));

    heap_listp += (2 * WSIZE);
    free_listp = NULL;

    if ((heap_listp = extend_heap(CHUNKSIZE / WSIZE)) == NULL)
    {
        return -1;
    }
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{

    char *bp;
    size_t extend_size;
    size_t newsize = ALIGN(size + SIZE_T_SIZE);
    if (size == 0)
    {
        newsize = 16;
    }
    if ((bp = first_fit(newsize)) != NULL)
    {
        place(bp, newsize);
        return bp;
    }

    extend_size = MAX(newsize, CHUNKSIZE);
    if ((bp = extend_heap(extend_size / WSIZE)) == NULL)
    {
        return NULL;
    }

    place(bp, newsize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));

    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *newptr;
    size_t old_size;
    size_t copy_size;

    if (ptr == NULL)
        return mm_malloc(size);

    if (size == 0)
    {
        mm_free(ptr);
        return NULL;
    }

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;

    old_size = GET_SIZE(HDRP(ptr));
    copy_size = old_size - DSIZE;

    if (size < copy_size)
        copy_size = size;

    memcpy(newptr, ptr, copy_size);

    mm_free(ptr);
    return newptr;
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
    {
        return NULL;
    }

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}

static void *first_fit(size_t size)
{
    void *p = free_listp;
    if (!p)
    {
        return NULL;
    }
    while (GET_SIZE(HDRP(p)) < size)
    {
        if (!NEXT_FREE(p))
            return NULL;
        p = NEXT_FREE(p);
    }
    return p;
}

static void place(void *bp, size_t asize)
{
    size_t old_size = GET_SIZE(HDRP(bp));
    if (old_size - asize < 2 * DSIZE)
    {
        delete_block(bp);
        PUT(FTRP(bp), PACK(old_size, 1));
        PUT(HDRP(bp), PACK(old_size, 1));
    }
    else
    {
        delete_block(bp);
        PUT(bp + asize - DSIZE, PACK(asize, 1));
        PUT(bp + asize - WSIZE, PACK(old_size - asize, 0));
        PUT(FTRP(bp), PACK(old_size - asize, 0));
        PUT(HDRP(bp), PACK(asize, 1));
        insert_block(NEXT_BLKP(bp));
    }
}

static void *coalesce(void *bp)
{
    void *prev = PREV_BLKP(bp);
    void *next = NEXT_BLKP(bp);

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)
    {
        insert_block(bp);
        return bp;
    }

    else if (prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));

        delete_block(next);

        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        insert_block(bp);
    }

    else if (!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));

        delete_block(prev);

        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

        insert_block(bp);
    }

    else
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));

        delete_block(prev);
        delete_block(next);

        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);

        insert_block(bp);
    }
    return bp;
}

static void insert_block(void *bp)
{
    PREV_FREE(bp) = NULL;
    NEXT_FREE(bp) = free_listp;
    if (free_listp != NULL)
    {
        PREV_FREE(free_listp) = bp;
    }
    free_listp = bp;
    return;
}

static void delete_block(void *bp)
{
    if (free_listp == NULL)
        return;
    else if (free_listp == bp)
    {
        free_listp = NEXT_FREE(bp);

        if (free_listp != NULL)
            PREV_FREE(free_listp) = NULL;
    }
    else if (NEXT_FREE(bp) == NULL)
    {
        NEXT_FREE(PREV_FREE(bp)) = NULL;
        PREV_FREE(bp) = NULL;
    }
    else
    {
        NEXT_FREE(PREV_FREE(bp)) = NEXT_FREE(bp);
        PREV_FREE(NEXT_FREE(bp)) = PREV_FREE(bp);
    }
}