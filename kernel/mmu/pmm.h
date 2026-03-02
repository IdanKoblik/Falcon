#ifndef PMM_H
#define PMM_H

#include <stdint.h>

void pmm_init(uint32_t multiboot_addr);
uint32_t pmm_alloc_frame(void);
void pmm_free_frame(uint32_t phys);
uint32_t pmm_get_total_frames(void);

#endif // PMM_H
