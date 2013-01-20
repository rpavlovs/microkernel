#
# Makefile for cs452 micokernel
#
XCC     = gcc
AS			= as
LD      = ld

CFLAGS  = -c -fPIC -Wall -Iinclude -mcpu=arm920t -msoft-float
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs: always generate a complete stack frame

LDFLAGS = -init main -Map build/kernel.map -N  -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -Llib

SRC = $(wildcard kernel/*.c kernel/*.C)
ASM = $(wildcard kernel/*.s kernel/*.S)
OBJS = $(SRC:kernel/%.c=build/%.o) $(ASM:kernel/%.s=build/%.o)

all: build/kernel.elf

staff:

clean:
  rm -f build/*


## Compile

build/%.s: kernel/%.c
	$(XCC) -S $(CFLAGS) -o $@ $<

build/%.s: kernel/%.S
	cp $< $@


## Assemble

build/%.o: build/%.s
	$(AS) $(ASFLAGS) -o $@ $<


## Link

build/kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) -lbwio -lgcc
