#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

#define MULTIBOOT_MAGIC 0x36d76289
#define MULTIBOOT_TAG_TYPE_MMAP 6
#define MULTIBOOT_MEMORY_AVAILABLE 1

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    /* entries follow: (size - 16) / entry_size of them */
};

struct multiboot_mmap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
} __attribute__((packed));

#endif // MULTIBOOT_H
