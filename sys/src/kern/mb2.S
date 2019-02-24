#define ASM_FILE
#include "sys/multiboot2.h"

	// Based on a 64-bit version by Wink Saville

	.set MAGIC, MULTIBOOT2_HEADER_MAGIC
	.set FLAGS, 0		// i386 protected mode
	.set CHECKSUM, -(MAGIC + FLAGS + (mbh_end - mbh_start))

	.section .text
	.align 4
mbh_start:
	// Multiboot2 header
	.long		MAGIC
	.long		FLAGS
	.long		mbh_end - mbh_start
	.long		CHECKSUM

	// End tag
	.word		0		// Type
	.word		0		// Flags
	.long		8		// Size
mbh_end:
