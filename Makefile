ROOT = $(CURDIR)

CC = gcc
LD = ld

CFLAGS = -Os \
	-fomit-frame-pointer \
	-falign-functions=0 \
	-falign-jumps=0 \
	-falign-loops=0 \
	-mregparm=3 \
	-DREGPARM=3 \
	-W -Wall -Wstrict-prototypes \
	-Wno-clobbered \
	-DDYNAMIC_DEBUG \
	-W \
	-Wall \
	-fomit-frame-pointer \
	-D__COM32__ \
	-D__FIRMWARE___ \
	-DDYNAMIC_MODULE \
	-nostdinc \
	-iwithprefix include \
	-I$(ROOT)/src \
	-I$(ROOT)/libutil/include \
	-I$(ROOT)/include \
	-I$(ROOT)/include/sys \
	-I$(ROOT)/core/include \
	-I/home/usernameak/syslinux_mod -DLDLINUX="" -m32 \
	-fno-stack-protector

LDFLAGS = -m elf_i386 \
	-shared \
	--hash-style=gnu \
	-T $(ROOT)/lib/elf.ld \
	--as-needed \
	-S
	

OBJECTS = src/main.o src/util.o src/vesa.o src/cli.o src/system.o
C_LIBS = $(ROOT)/lib/libutil.c32 $(ROOT)/lib/libcom32.c32

ifndef VERBOSE
.SILENT:
endif

.PHONY: clean all

all: meow.c32

meow.c32: $(OBJECTS) $(C_LIBS)
	$(LD) $(LDFLAGS) -o $@ $^

iso: all
	 mkisofs -o output.iso    -b isolinux/isolinux.bin -c isolinux/boot.cat    -no-emul-boot -boot-load-size 4 iso

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f src/*.o *.c32
