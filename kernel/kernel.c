#include <stdint.h>
#include "multiboot.h"
#include "drivers/vga.h"
#include "type_helper.h"
#include "programs/mem_test.h"
#include "mmu/heap.h"
#include "mmu/pmm.h"

static void print_info(uint32_t magic, uint32_t addr) {
    vga_print("Multiboot magic number: ");
    vga_print(uint32_to_str(magic, (char[11]){}, 16));
    vga_print("\nMultiboot info address: ");
    vga_print(uint32_to_str(addr, (char[11]){}, 16));
    vga_print("\n\n");
}

void kmain(uint32_t magic, uint32_t addr) {
    vga_init();

    if (magic != MULTIBOOT_MAGIC) {
        vga_color_print("Invalid multiboot magic number\n", 0x0C);
        while (1);
    }

    vga_print("Falcon (x86_32)\n===============\n\n");
    print_info(magic, addr);

    pmm_init(addr);
    heap_init();

    run_memory_tests();
    while (1);
}
