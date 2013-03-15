#
# Makefile for cs452 micokernel
#
XCC     = gcc
AS		= as
LD      = ld

USER:=$(shell whoami)
SRC_DIRS:=$(shell find src/ -mindepth 1 -type d)
.SECONDARY:

CFLAGS  = -c -fPIC -Wall -Iinclude -mcpu=arm920t -msoft-float -fno-builtin
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs: always generate a complete stack frame

LDFLAGS = -init main -Map build/kernel.map -N -T orex.ld -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2

SRC = $(wildcard src/**/*.c) $(wildcard src/*.c)
ASM = $(wildcard src/**/*.S) $(wildcard src/*.S)
OBJS = $(SRC:src/%.c=build/%.o) $(ASM:src/%.S=build/%.o)

all: build/kernel.elf $(SRC)

clean:
	@rm -rf build/*

install:
	@cp build/kernel.elf /u/cs452/tftp/ARM/$(USER)/kernel.elf
	@chmod a+r /u/cs452/tftp/ARM/$(USER)/kernel.elf
## Compile

build/%.s: src/%.c
	@mkdir -p $(SRC_DIRS:src/%=build/%)
	@$(XCC) -S $(CFLAGS) -o $@ $<

build/%.s: src/%.S
	@mkdir -p $(SRC_DIRS:src/%=build/%)
	@cp $< $@

## Assemble

build/%.o: build/%.s
	@$(AS) $(ASFLAGS) -o $@ $<

## Link

build/kernel.elf: $(OBJS) 
	@$(LD) $(LDFLAGS) -o $@ $(OBJS) -lgcc
