# Tiny-Ray-Tracing

Cal Poly CPE 233 Final Project, by Daniel Brathwaite and James Gruber

This is a demonstration of ray tracing on an RV32I ISA RISCV MCU.

Makefile, link.ld, init.s by Dr. Paul Hummel Dr. Joseph Callenes, main.c by Daniel Brathwaite and James Gruber.
Some small changes have been made to the orignally provided makefile and linker script to get proper compilation for the OTTER MCU.

Run `make` to build the software. This produces a build directory, containing:

- object files (`.o`) produced from your source code
- a  `rt.mem` file which is the memory image to be flashed into the memory
  module of the otter during synthesis
- a `program.dump` file which is the full disassembled program output.
- and some intermediate `.bin` and `.elf` files which you can ignore.

You can also run:

- `make clean` to remove the build directory
