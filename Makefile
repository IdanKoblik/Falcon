CC = gcc
AS = nasm
LD = ld

# -m32: Compile for 32-bit
# -ffreestanding: No standard library
# -fno-stack-protector: Required for kernels without a libs
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra \
	-fno-stack-protector \
	-MMD -MP
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -n -T linker.ld

KERNEL = falcon-x86_32.bin
ISO = falcon.iso

C_SRCS := $(shell find kernel -name '*.c')
C_OBJS := $(C_SRCS:.c=.o)

BOOT_OBJ := kernel/boot.o

OTHER_OBJS := $(filter-out $(BOOT_OBJ), $(C_OBJS))
FINAL_OBJS := $(BOOT_OBJ) $(OTHER_OBJS)

DEPS := $(FINAL_OBJS:.o=.d)

.PHONY: all clean run

all: $(ISO)

$(ISO): $(KERNEL)
	@mkdir -p isodir/boot/grub
	cp $(KERNEL) isodir/boot/
	cp grub.cfg isodir/boot/grub/
	grub2-mkrescue -o $(ISO) isodir

$(KERNEL): $(FINAL_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(FINAL_OBJS)

kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/%.o: kernel/%.s
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf $(KERNEL) $(ISO) isodir
	find kernel -name '*.o' -delete
	find kernel -name '*.d' -delete

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

-include $(DEPS)
