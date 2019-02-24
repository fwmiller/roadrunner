# The Roadrunner Operating System

This repository contains the source code for the Roadrunner Operating System.

I wrote this OS as part of my Ph.D. dissertation which I completed at the
University of Maryland in 1998.  The development started while I was at
NASA/JSC in about 1989 and proceeded for about 14 years before I stopped
working on it.  That last release when I stopped was 0.8.16 and this code
can be found in the repo https://github.com/fwmiller/roadrunner-0.8.16

The OS was originally written for the Intel i386 processor.  It uses a single
32-bit flat address space with paged memory protection but but does not use
virtual addresses, i.e. all addresses map to identical physical addresses.
I wrote a paper on the memory protection that can still be found at:
https://www.usenix.org/conference/2002-usenix-annual-technical-conference/simple-memory-protection-embedded-operating

I have recently (2018) begun work on the OS again.  I am targeting qemu
as the execution environment and the system now boots from an xorriso
generated ISO image.

The paged memory protection maps well to the recent ARM Cortex processor
that include the Memory Protection Unit (MPU).  The work will likely move
towards porting to that processor.
