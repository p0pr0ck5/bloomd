#ifndef CBLOOM_BITMAP_H
#define CBLOOM_BITMAP_H
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>

typedef enum {
    SHARED,         // MAP_SHARED mmap used. File backed.
    ANONYMOUS       // MAP_ANONYMOUS mmap used. No file backing.
} cbloom_flags;

typedef struct {
    uint64_t size;  // Size of bitmap in bytes
    cbloom_flags flags;    // Bitmap flags.
    int fileno;   // Underlying fileno for the mmap
    unsigned char* mmap;   // Starting address of the mem-map region
} cbloom_bitmap;

/**
 * Returns a cbloom_bitmap pointer from a file handle
 * that is already opened with read/write privileges.
 * @arg fileno The fileno
 * @arg len The length of the bitmap in bytes.
 * @arg map The output map. Will be initialized.
 * @return 0 on success. Negative on error.
 */
int bitmap_from_file(int fileno, uint64_t len, cbloom_bitmap *map);

/**
 * Returns a cbloom_bitmap pointer from a filename.
 * Opens the file with read/write privileges. If create
 * is true, then a file will be created if it does not exist.
 * If the file cannot be opened, NULL will be returned.
 * @arg fileno The fileno
 * @arg len The length of the bitmap in bytes.
 * @arg create If 1, then the file will be created if it does not exist.
 * @arg resize If 1, then the file will be expanded to len
 * @arg map The output map. Will be initialized.
 * @return 0 on success. Negative on error.
 */
int bitmap_from_filename(char* filename, uint64_t len, int create, int resize, cbloom_bitmap *map);

/**
 * Flushes the bitmap back to disk. This is
 * a syncronous operation. It is a no-op for
 * ANONYMOUS bitmaps.
 * @arg map The bitmap
 * @returns 0 on success, negative failure.
 */
int bitmap_flush(cbloom_bitmap *map);

/**
 * * Closes and flushes the bitmap. This is
 * a syncronous operation. It is a no-op for
 * ANONYMOUS bitmaps. The caller should free()
 * the structure after.
 * @arg map The bitmap
 * @returns 0 on success, negative on failure.
 */
int bitmap_close(cbloom_bitmap *map);

/**
 * Returns the value of the bit at index idx for the
 * cbloom_bitmap map
 */
#define BITMAP_GETBIT(map, idx) ((map->mmap[idx >> 3] >> (7 - (idx % 8))) & 0x1)

/**
 * Sets the value of the bit at index idx for the
 * cbloom_bitmap map
 */
#define BITMAP_SETBIT(map, idx, val) {                   \
            unsigned char byte = map->mmap[idx >> 3];    \
            unsigned char byte_off = 7 - idx % 8;        \
            if (val) {                                   \
                byte |= 1 << byte_off;                   \
            } else {                                     \
                byte &= ~(1 << byte_off);                \
            }                                            \
            map->mmap[idx >> 3] = byte;                  \
        }                                                \

#endif
