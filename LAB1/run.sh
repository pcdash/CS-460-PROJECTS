#!/bin/sh

as86 -o bs.o bs.s

bcc -c -ansi t.c

ld86 -d bs.o t.o /usr/lib/bcc/libc.a

ls -l a.out

dd if=a.out of=mtximage bs=1024 count=1 conv=notrunc

qemu-system-i386 -fda mtximage -no-fd-bootchk -serial mon:stdio

rm *.o
echo done

