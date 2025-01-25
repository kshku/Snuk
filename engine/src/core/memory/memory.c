#include "memory.h"

#include "../assertions.h"
#include "../logger.h"
#include "platform/memory.h"

// /**
//  * @brief Get the number of bytes for writing the size.
//  *
//  * @param size Size to write
//  */
// #define BYTES_FOR_WRITING_SIZE(size) ((size / 0x7f) + (size % 0x7f ? 1 : 0))

/**
 * @brief Offset in block
 */
#define OFFSET_IN_FREE_BLOCK(block) (*(u64 *)block)

// Create one main allocator who makes call to the platformAllocator and gets
// the memory Every other allocators will get their memory from the main
// allocator.

// TODO : Refactor the code and reuse functions and ....
// TODO: Error handling

// I had few options in mind and finally chose this one.
// When we allocate we also allocate 8 bytes more for header to store the size
// of the allocation. After allocation is freed, in the 8 bytes we will have the
// offset form that address to next free block (if offset is zero, no block is
// free next). A lot of calculations is required, but total metadata size is
// just 8 bytes. Minimum size of allocation is 1 byte + header size =
// 9 bytes. After freeing the size of block is saved the next bytes after the
// header. Size is accessed using the writeBlockSize function and readBlockSize
// functions.
// Previously was thinking about using the Freeblcok linked list to store the
// pointer to the free block and store the size in the free block itself using
// the writeBlockSize functions and when allocating, using writeBlockSize
// function to minimize the size of header. This method would require ((2 *
// sizeof(void *)) + header size to store the allocation size by writeBlockSize
// function (which can easily go beyond 8 bytes)). But for the method we are
// following we just need 8 bytes extra.
// NOTE: Offset is always with respect to where the offset is stored

typedef struct BlockHeader {
        void *previous_block;
        u64 block_size;
        // Will always store the offset to the first free block in the current
        // block (if not there zero)
        u64 first_free_block_offset;
} BlockHeader;

typedef struct MemState {
        b8 initialized;
        BlockHeader *header;
        u64 page_size;
} MemState;

static MemState mem_state;

/**
 * @brief Write the size from given address.
 *
 * @param block Pointer to the block
 * @param size Size to write into block
 *
 * @note This function assumes required number of bytes are there in block. Use
 * BYTES_FOR_WRITING_SIZE macro to get the number of bytes requried.
 */
void writeBlockSize(void *block, u64 size) {
    // We will have size = 1 => 1 byte atleast
    sassert_msg(size > 0, "Size zero?");

    u8 *b = (u8 *)block;

    // If size > 127, we have to write remaining size in next byte (which is
    // guaranteed to be free since size > 1).
    // To say we have written in next bytes we will set 8th bit to 1
    // while (size > 0x7f) {
    //     size -= 0x7f;
    //     *b = 0xff;
    //     ++b;
    // }
    // for (; size > 0x7f; size -= 0x7f, ++b) *b = 0xff;

    // u64 full_bytes = size / 0x7f;
    // sMemSet(block, full_bytes, 0xff);
    // *((u8 *)block + full_bytes) = size % 0x7f;

    // Write the remaining size
    // *b = size;

    // If storing the multiplier of 128 then the 8th should be 1 which also
    // indicates we have to read next byte too. If 8th bit is 0 then this is the
    // last bit and the multiplier is 1.

    // All values in blocks with 8th bit 1 should be added and multiplied by 128
    // and the result should be added with the value in the last byte (byte
    // whose 8th bit is 0)

    // This to speed up the readBlockSize a little bit. It have to read all the
    // bytes to calculate the size. If we store the multipler it would have to
    // read less number of bytes

    // Multiplier of 128 because in the last byte we can store from 0-127. For
    // example to represent 129 we will have first byte 1000 0001 and
    // second(last) byte 0000 0001
    u64 multiplier = size / 128;

    // Number of bytes will have 127
    u64 full_bytes = multiplier / 0x7f;

    // Write value to those full_bytes
    sMemSet(b, full_bytes, 0xff);

    // Move right next to the byte having 0xff
    b += full_bytes;

    // Write the remaining multipliers here and since we have to read the next
    // byte set 8th bit to 1
    *b = 0x80 | (multiplier % 0x7f);

    // Write to last byte
    b++;
    *b = size % 128;
}

/**
 * @brief Get the size written in the given block.
 *
 * @param block The block to read
 *
 * @return The size written in the block.
 */
u64 readBlockSize(const void *block) {
    u64 size = 0;
    u8 *b = (u8 *)block;

    // If 8th bit is set then we have to add the size written in next block too.
    // while (*b & 0x80) {  // Can also write as (*b & BITFLAG(7))
    //     ++b;
    //     size += 0x7f;
    // }

    // Read the multiplier
    for (; *b & 0x80; ++b) size += ((*b) & 0x7f);

    // Calculate the size from the multipler
    size *= 128;

    // Read the size in this byte.
    size += *b;

    return size;
}

/**
 * @brief Adds the free block.
 *
 * The block will have size atleast 9 bytes.
 *
 * @param pb Previous block
 * @param b The free block
 * @param size The size of the free blcok
 * @param is_header_field Is the pb header field?
 */
void addFreeBlock(void *pb, void *b, u64 size, b8 is_header_field) {
    u8 *block = (u8 *)b;

    u8 *previous_block = (u8 *)pb;
    u64 previous_block_offset = OFFSET_IN_FREE_BLOCK(previous_block);

    u8 *next_block =
        previous_block_offset ? previous_block + previous_block_offset : NULL;
    u64 next_block_offset = next_block ? OFFSET_IN_FREE_BLOCK(next_block) : 0;

    // Try to merge with previous block first
    // NOTE: We should not merge with the free block in the header field
    if (!is_header_field) {
        // Check whether we can merge this block to previous free block
        u64 previous_block_size = readBlockSize(previous_block + 8);
        if (previous_block + previous_block_size == block) {
            // We can merge
            // NOTE: Don't have to change the offset while merging
            // Add this to previous block's size
            previous_block_size += size;

            // Can we also merge with next block?
            if (previous_block + previous_block_size == next_block) {
                // Yes we can, merge it now!
                // We have to change the offset of previous block
                if (next_block_offset)
                    OFFSET_IN_FREE_BLOCK(previous_block) =
                        previous_block_offset
                        + OFFSET_IN_FREE_BLOCK(next_block);
                else OFFSET_IN_FREE_BLOCK(previous_block) = 0;

                // Add next block's size to previous block too
                previous_block_size += readBlockSize(next_block + 8);
            }

            writeBlockSize((previous_block + 8), previous_block_size);

            // Thats it we added free block
            return;
        }
    }

    // Couldn't merge. Update the previous free block's offset to here
    OFFSET_IN_FREE_BLOCK(previous_block) = block - previous_block;

    if (!next_block) {
        // previous block was last block, now this is last block
        OFFSET_IN_FREE_BLOCK(block) = 0;
        writeBlockSize(block + 8, size);
        return;
    }

    // There is a free block next to this

    // Check can we merge with next free block
    if (block + size == next_block) {
        // We can merge with next free block and the next free block's next
        // offset should be this block's offset
        if (next_block_offset)
            OFFSET_IN_FREE_BLOCK(block) =
                next_block - block + OFFSET_IN_FREE_BLOCK(next_block);
        else OFFSET_IN_FREE_BLOCK(block) = 0;

        // Update this block's size by adding next block's size
        size += readBlockSize(next_block + 8);

        writeBlockSize(block + 8, size);

        // Done with the task
        return;
    }

    // We couldn't merge. This block is not connected to any other free block
    // this block's offset should point to next block
    OFFSET_IN_FREE_BLOCK(block) = next_block - block;
    writeBlockSize(block + 8, size);
}

/**
 * @brief Update the free block.
 *
 * If the remaining size is zero then removes the entry, else just moves the
 * free block.
 *
 * @param pb Previous block
 * @param b The target block
 * @param size The previous size of block
 * @param new_size The new size of block
 */
void updateFreeBlock(void *pb, void *b, u64 size, u64 new_size) {
    sassert(size > new_size);

    u8 *block = (u8 *)b;
    u8 *previous_block = (u8 *)pb;
    u64 block_offset = OFFSET_IN_FREE_BLOCK(block);
    u64 diff = size - new_size;

    if (diff) {
        OFFSET_IN_FREE_BLOCK(previous_block) += diff;
        block += diff;

        if (block_offset) OFFSET_IN_FREE_BLOCK(block) = block_offset - diff;
        else OFFSET_IN_FREE_BLOCK(block) = 0;

        writeBlockSize(block + 8, new_size);
    } else {
        if (block_offset)
            OFFSET_IN_FREE_BLOCK(previous_block) += OFFSET_IN_FREE_BLOCK(block);
        else OFFSET_IN_FREE_BLOCK(previous_block) = 0;
    }
}

/**
 * @brief Get the free block previous to the given pointer.
 *
 * Also fills whether the block is header block or not in is_header_field if it
 * is not NULL. parameter.
 *
 * @param ptr Pointer
 * @param is_header_filed Whether free block is header field or not
 *
 * @return Returns the pointer to the free block if found else returns NULL.
 */
void *getPreviousFreeBlock(const void *ptr, b8 *is_header_field) {
    BlockHeader *cur_header = mem_state.header;

    while (cur_header) {
        if ((void *)cur_header < ptr
            && ptr < (void *)((u8 *)cur_header + cur_header->block_size)) {
            u8 *previous_block = (u8 *)&cur_header->first_free_block_offset;
            while ((void *)previous_block < ptr
                   && (void *)(previous_block
                               + OFFSET_IN_FREE_BLOCK(previous_block))
                          < ptr)
                previous_block += OFFSET_IN_FREE_BLOCK(previous_block);

            if (is_header_field)
                *is_header_field = previous_block
                                == (u8 *)&cur_header->first_free_block_offset;

            return previous_block;
        }

        cur_header = cur_header->previous_block;
    }

    return NULL;
}

/**
 * @brief Initialize the memory subsystem.
 *
 * @return true memory subsystem was initialized successfully.
 */
b8 initializeMemory(void) {
    if (mem_state.initialized) {
        sError("Memory system is already initialized, but initializeMemory was "
               "called again");
        return false;
    }

    sMemZeroOut(&mem_state, sizeof(MemState));

    i64 page_size = platformGetPageSize();
    if (page_size < 1) {
        sFatal("Page size is less than 1");
        return false;
    }
    mem_state.page_size = page_size;

    mem_state.initialized = true;
    return true;
}

/**
 * @brief Shutdown the memory subsystem.
 */
void shutdownMemory(void) {
    if (!mem_state.initialized) {
        sError("shutdownMemory called without initializing Memory");
        return;
    }

    BlockHeader *cur = mem_state.header;
    BlockHeader *prev;
    while (cur) {
        // Get the header which is pointer to another base (page)
        prev = cur->previous_block;
        platformDeallocateMemory(cur, cur->block_size);
        cur = prev;
    }

    mem_state.initialized = false;
}

/**
 * @brief Get the page size.
 *
 * Queries the platform for the size of page. If fails returns the default page
 * size (4 * KiB which is common).
 *
 * @return Page size.
 */
u64 sMemGetPageSize(void) {
    return mem_state.page_size;
}

/**
 * @brief Get n continuous pages of memory.
 *
 * If n is zero NULL will be returned.
 *
 * @param n Number of pages
 *
 * @return Returns the address of the page or NULL if failed.
 *
 * @note Get page size from sMemGetPageSize function.
 */
void *sMemAllocatePages(u64 n) {
    // TODO: Keep track of this too and provide function for deallocating
    sassert_msg(mem_state.initialized,
                "Haven't initialized the memory subsystem?");

    return n ? platformAllocateMemory(n * mem_state.page_size) : NULL;
}

/**
 * @brief Get more memory [INTERNAL FUNCTION].
 *
 * @param size Minimum size(bytes) the block will have
 *
 * @return Returns true if got more page else false.
 */
b8 getMemoryWithAtleast(u64 size) {
    size += sizeof(BlockHeader);
    u64 n_pages =
        (size / mem_state.page_size) + (size % mem_state.page_size ? 1 : 0);
    BlockHeader *ptr = (BlockHeader *)sMemAllocatePages(n_pages);
    if (!ptr) {
        sError("Failed to allocate more pages");
        return false;
    }

    // Write the previous base address to the header
    ptr->previous_block = mem_state.header;
    ptr->block_size = mem_state.page_size * n_pages;
    ptr->first_free_block_offset = 0;

    // addFreeBlock(ptr, ptr + 1, ptr->block_size);
    addFreeBlock(&ptr->first_free_block_offset, ptr + 1, ptr->block_size, true);

    mem_state.header = ptr;

    return true;
}

/**
 * @brief Get free block having atleast given size.
 *
 * @param block_header The block header in which to search
 * @param size The size to search for
 *
 * @return Pointer to the block having atleast given size. If not found returns
 * NULL.
 */
void *getFreeBlock(const BlockHeader *block_header, u64 size) {
    if (!block_header->first_free_block_offset) return NULL;

    u8 *prev = (u8 *)&block_header->first_free_block_offset;
    u8 *free_block = prev + OFFSET_IN_FREE_BLOCK(prev);

    u64 free_block_size = readBlockSize(free_block + 8);
    while (free_block_size < size) {
        if (!OFFSET_IN_FREE_BLOCK(free_block)) return NULL;
        prev = free_block;
        free_block += OFFSET_IN_FREE_BLOCK(free_block);
        free_block_size = readBlockSize(free_block + 8);
    }

    updateFreeBlock(prev, free_block, free_block_size, free_block_size - size);

    return free_block;
}

/**
 * @brief Get the free block which can hold given size of data.
 *
 * @param size Size required
 *
 * @return Pointer to the block which can hold given size of data.
 */
void *getBlockForSize(u64 size) {
    BlockHeader *cur_header = mem_state.header;
    void *block = NULL;

    while (!block && cur_header) {
        block = getFreeBlock(cur_header, size);
        cur_header = cur_header->previous_block;
    }

    // Suitable block was not found, allocate more memory
    if (!block) {
        if (!getMemoryWithAtleast(size)) return NULL;
        cur_header = mem_state.header;
        block = getFreeBlock(cur_header, size);
    }

    return block;
}

/**
 * @brief Similar to malloc.
 *
 * @param size Size of bytes to allocate
 *
 * @return Pointer to allocated memory on success else NULL.
 */
void *sMalloc(u64 size) {
    sassert_msg(mem_state.initialized, "Memory subsystem is not initialized!");

    if (size == 0) return NULL;

    size += 8;

    u64 *ptr = (u64 *)getBlockForSize(size);

    if (!ptr) return NULL;

    *ptr = size;

    return (void *)(ptr + 1);
}

/**
 * @brief Similar to calloc.
 *
 * @param nmemb Number of blocks
 * @param size Size of each block
 *
 * @return Pointer to allocated memory on success else NULL.
 */
void *sCalloc(u64 nmemb, u64 size) {
    sassert_msg(mem_state.initialized, "Memory subsystem is not initialized!");

    u64 total_size = size * nmemb;

    void *ptr = sMalloc(total_size);
    if (ptr) sMemZeroOut(ptr, total_size);

    return ptr;
}

/**
 * @brief Similar to realloc.
 *
 * If size is zero calls sFree(returns null) and if ptr is NULL calls
 * sMalloc. If ptr is NULL as well as size is zero throws error If ptr is
 * NULL as well as size is zero shows warning and returns NULL.
 *
 * @param ptr Pointer to the Allocated Memory
 * @param size New size
 *
 * @return NULL if failed else pointer to the Memory allocated with new
 * size.
 */
void *sRealloc(void *ptr, u64 size) {
    sassert_msg(mem_state.initialized, "Memory subsystem is not initialized!");

    if (!ptr && !size) {
        sError("sRealloc called with NULL pointer and 0 size");
        return ptr;
    }

    if (!size) {
        sFree(ptr);
        return NULL;
    }

    if (!ptr) return sMalloc(size);

    u64 *p = (u64 *)ptr - 1;

    // NOTE: both previous_size and size(parameter) doesn't have the header size
    // included
    u64 previous_size = *p - 8;

    // new size is same as old size nothing to do
    if (previous_size == size) return ptr;

    b8 is_header_field;
    u8 *previous_block = getPreviousFreeBlock(ptr, &is_header_field);

    // If we are decresing the size then add extra size as free block
    if (previous_size > size) {
        addFreeBlock(previous_block, (u8 *)ptr + size, previous_size - size,
                     is_header_field);

        // Upadate the size in header
        *p = size + 8;

        // No need to change the pointer. It has required size of memory
        return ptr;
    }

    // We are going to increase the size
    // Check whether we have free block right next to this block and
    // whether it's size is enough for to extend this block
    u8 *next_block = previous_block + OFFSET_IN_FREE_BLOCK(previous_block);
    if ((u8 *)ptr + previous_size == next_block) {
        // Yes next block is free
        u64 next_block_size = readBlockSize(next_block + 8);
        u64 extra_size_to_allocate = size - previous_size;
        if (next_block_size > extra_size_to_allocate) {
            // We can just extend this blocks size
            // Update the free block
            updateFreeBlock(previous_block, next_block, next_block_size,
                            next_block_size - extra_size_to_allocate);

            // Upadate the size in header
            *p = size + 8;

            // Just return the pointer which is not changed
            return ptr;
        }
    }

    // We need to allocate memory somewhere else, couldn't extend this block
    // Header is managed by the malloc we can just deal with the rest of memory
    void *new_block = sMalloc(size);
    if (!new_block) return NULL;

    sMemCopy(new_block, ptr, previous_size);

    // NOTE: The size in the header should not be manipulated
    sassert(*p == previous_size + 8);
    sFree(ptr);

    return new_block;
}

/**
 * @brief Similar to free.
 *
 * Similar to free, sFree function frees the memory space pointed by the ptr,
 * which must be returned by previous call to malloc, calloc, or realloc
 * functions. Otherwise, or if ptr has already been freed, undefined behaviour
 * occurs.
 *
 * @param ptr Pointer to the memory to be deallocated
 */
void sFree(void *ptr) {
    sassert_msg(mem_state.initialized, "Memory subsystem is not initialized!");

    // If ptr is null then do nothing
    if (!ptr) return;

    u64 *p = (u64 *)ptr - 1;

    b8 is_header_field;
    void *previous_block = getPreviousFreeBlock(ptr, &is_header_field);

    if (!previous_block) sError("sFree called on the unallocated pointer");

    addFreeBlock(previous_block, p, *p, is_header_field);
}

/**
 * @brief Log how much memory is allocated.
 */
void sMemLogState(void) {
    sassert_msg(mem_state.initialized, "Memory subsystem is not initialized!");

    // f32 allocation = 0;
    // char ext[4] = "XiB";

    // if (mem_state.total_allocated >= GiB) {
    //     ext[0] = 'G';
    //     allocation = (f32)mem_state.total_allocated / GiB;
    // } else if (mem_state.total_allocated >= MiB) {
    //     ext[0] = 'M';
    //     allocation = (f32)mem_state.total_allocated / MiB;
    // } else if (mem_state.total_allocated >= KiB) {
    //     ext[0] = 'K';
    //     allocation = (f32)mem_state.total_allocated / KiB;
    // } else {
    //     ext[0] = 'B';
    //     ext[1] = 0;
    //     allocation = (f32)mem_state.total_allocated;
    // }

    // sInfo("Total of %f %s memory is allocated", allocation, ext);
}

/**
 * @brief Similar to memset sets each byte in memory to given value.
 *
 * @param ptr Pointer to the memory
 * @param size Size of memory
 * @param value The value to which each byte to be set
 *
 * @return Returns the given pointer.
 */
void *sMemSet(void *ptr, u64 size, u8 value) {
    // TODO: Probably should let the compiler to optimize by converting to SIMD
    // which might be faster than this manual large block copying.
    // I think it is better to set up SIMD things and all then come to optimize.

    // u64 v64 = (value << 56) | (value << 48) | (value << 40) | (value << 32)
    //         | (value << 24) | (value << 16) | (value << 8) | (value);

    // // First 64 bits (8 bytes)
    // u64 i = 0;
    // u64 *p64 = (u64 *)ptr;
    // for (; i + 8 < size; i += 8, ++p64) *p64 = v64;

    // // Next is 32 bits (4 bytes)
    // u32 *p32 = (u32 *)p64;
    // u32 v32 = (u32)v64;
    // for (; i + 4 < size; i += 4, ++p32) *p32 = v32;

    // // Next is 16 bits (2 bytes)
    // u16 *p16 = (u16 *)p32;
    // u16 v16 = (u16)v64;
    // for (; i + 2 < size; i += 2, ++p16) *p16 = v16;

    // // Next is 8 bits (1 bytes)
    // u8 *p = (u8 *)p16;
    // for (; i < size; ++i, ++p) *p = value;

    // And done
    // return ptr;

    u8 *p = (u8 *)ptr;
    for (u64 i = 0; i < size; ++i, ++p) *p = value;
    return ptr;
}

/**
 * @brief Copy memory from source to destination.
 *
 * @param dest Pointer to destination
 * @param src Pointer to source
 * @param size Number of bytes to copy
 *
 * @return Returns the destination pointer.
 *
 * @note Source and destination should not overlap.
 */
void *sMemCopy(void *dest, const void *src, u64 size) {
    if (dest == src) return dest;

    u8 *d = (u8 *)dest;
    u8 *s = (u8 *)src;
    for (u64 i = 0; i < size; ++i, ++d, ++s) *d = *s;

    return dest;
}

/**
 * @brief Copy memory from source to destination.
 *
 * @param dest Destination pointer
 * @param src Source pointer
 * @param size Number of bytes to copy
 *
 * @return Returns the destination pointer.
 *
 * @note Source and destination may overlap.
 */
void *sMemMove(void *dest, void *src, u64 size) {
    if (dest == src) return dest;

    u8 *d = (u8 *)dest;
    u8 *s = (u8 *)src;
    if (dest > src) {
        d += size;
        s += size;
        for (u64 i = size; i > 0; --i, --d, --s) *d = *s;
    } else {
        for (u64 i = 0; i < size; ++i, ++d, ++s) *d = *s;
    }

    return dest;
}
