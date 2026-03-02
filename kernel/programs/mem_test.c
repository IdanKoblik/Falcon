#include <stdint.h>
#include "../drivers/vga.h"
#include "../mmu/heap.h"
#include "../mmu/pmm.h"
#include "../colors.h"

static void pass(const char *msg) {
    vga_color_print("[PASS] ", GREEN);
    vga_print(msg);
    vga_print("\n");
}

static void fail(const char* msg) {
    vga_color_print("[FAIL] ", RED);
    vga_print(msg);
    vga_print("\n");
}

void run_memory_tests(void) {
    vga_color_print("=== Memory Tests ===\n\n", CYAN);

    // pmm
    uint32_t f1 = pmm_alloc_frame();
    uint32_t f2 = pmm_alloc_frame();

    if (f1 && f2 && (f1 != f2))
        pass("PMM allocates unique frames");
    else
        fail("PMM frame allocation failed");

    pmm_free_frame(f1);
    pmm_free_frame(f2);

    uint32_t f3 = pmm_alloc_frame();
    if (f3 == f1)
        pass("PMM free + reuse works");
    else
        fail("PMM free did not recycle frame");

    pmm_free_frame(f3);

    // kmalloc
    void *a = kmalloc(32);
    if (a)
        pass("kmalloc basic allocation");
    else
        fail("kmalloc returned NULL");

    uint32_t* p = (uint32_t*)a;
    p[0] = 0xDEADBEEF;

    if (p[0] == 0xDEADBEEF)
        pass("kmalloc memory writable");
    else
        fail("kmalloc memory corruption");

    // split
    kfree(a);
    void* c = kmalloc(16);

    if (c == a)
        pass("Freed block reused");
    else
        fail("Freed block not reused");

    // free
    void* d = kmalloc(128);
    void* e = kmalloc(128);

    kfree(d);
    kfree(e);

    void* big = kmalloc(256);
    if (big == d)
        pass("Block coalescing works");
    else
        fail("Block coalescing failed");

    vga_color_print("\n=== Tests Complete ===\n", CYAN);
}
