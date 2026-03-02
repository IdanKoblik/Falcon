#include "pmm.h"
#include "../multiboot.h"

#define FRAME_SIZE 4096
#define BITMAP_BYTES (128 * 1024) // 128 KiB
#define TOTAL_FRAMES (BITMAP_BYTES * 8) // 1M frames = 4 GiB

extern char _kernel_end;

static uint8_t bitmap[BITMAP_BYTES];
static uint32_t total_frames;

static inline void bitmap_set(uint32_t f)   { bitmap[f / 8] |=  (1u << (f % 8)); }
static inline void bitmap_clear(uint32_t f) { bitmap[f / 8] &= ~(1u << (f % 8)); }
static inline int  bitmap_test(uint32_t f)  { return bitmap[f / 8] &  (1u << (f % 8)); }

// Mark a physical range [base, base+len) as FREE
static void mark_free(uint32_t base, uint32_t len) {
    uint32_t first = (base + FRAME_SIZE - 1) / FRAME_SIZE; // ceil
    uint32_t last = (base + len) / FRAME_SIZE; // floor

    for (uint32_t f = first; f < last && f < TOTAL_FRAMES; f++) {
        bitmap_clear(f);
        if (f + 1 > total_frames)
            total_frames = f + 1;
    }
}

// Mark a physical range [base, base+len) as USED
static void mark_used(uint32_t base, uint32_t len) {
    uint32_t first = base / FRAME_SIZE;
    uint32_t last = (base + len + FRAME_SIZE - 1) / FRAME_SIZE;
    for (uint32_t f = first; f < last && f < TOTAL_FRAMES; f++)
        bitmap_set(f);
}

void pmm_init(uint32_t multiboot_addr) {
    for (uint32_t i = 0; i < BITMAP_BYTES; i++)
        bitmap[i] = 0xFF;

    total_frames = 0;

    // Free AVAILABLE mmap regions
    struct multiboot_tag *tag;
    for (tag = (struct multiboot_tag *)(multiboot_addr + 8); tag->type != 0; tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7u))) {
        if (tag->type != MULTIBOOT_TAG_TYPE_MMAP)
            continue;

        struct multiboot_tag_mmap *m = (struct multiboot_tag_mmap *)tag;
        uint8_t *p = (uint8_t *)m + sizeof(*m);
        uint8_t *end = (uint8_t *)m + m->size;

        while (p < end) {
            struct multiboot_mmap_entry *e = (struct multiboot_mmap_entry *)p;
            if (e->type == MULTIBOOT_MEMORY_AVAILABLE && e->addr < 0x100000000ULL) {
                uint32_t base = (uint32_t)e->addr;
                uint64_t top = e->addr + e->len;
                uint32_t len = (top > 0x100000000ULL) ? (uint32_t)(0x100000000ULL - e->addr) : (uint32_t)e->len;

                mark_free(base, len);
            }

            p += m->entry_size;
        }
    }

    // re-protect [0, _kernel_end] - covers IVT, BIOS, kernel, this bitmap
    mark_used(0, (uint32_t)&_kernel_end);

    // re-protect the multiboot info block
    uint32_t multiboot_size = *(uint32_t *)multiboot_addr;
    mark_used(multiboot_addr, multiboot_size);
}

uint32_t pmm_alloc_frame(void) {
    for (uint32_t f = 0; f < total_frames; f++) {
        if (!bitmap_test(f)) {
            bitmap_set(f);
            return f * FRAME_SIZE;
        }
    }

    return 0; // OOM - frame 0 is always protected so 0 is safe
}

void pmm_free_frame(uint32_t phys) {
    uint32_t f = phys / FRAME_SIZE;
    if (f < TOTAL_FRAMES)
        bitmap_clear(f);
}

uint32_t pmm_get_total_frames(void) {
    return total_frames;
}
