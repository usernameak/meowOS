#!/bin/bash

make

rm iso/isolinux/meow.c32
cp meow.c32 iso/isolinux
mkisofs -o output.iso -b isolinux/isolinux.bin -c isolinux/boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table iso
qemu-system-x86_64 -cdrom output.iso $@