#include <stdint.h>
#include "multiboot.h"

void kmain(uint32_t magic, uint32_t addr) {
    if (magic != MULTIBOOT_MAGIC) {
        while (1);
    }

    while (1);
}
