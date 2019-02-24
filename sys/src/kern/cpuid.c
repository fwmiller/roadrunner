/*
 *  Roadrunner/pk
 *    Copyright (C) 1989-2002  Cornfed Systems, Inc.
 *
 *  The Roadrunner/pk operating system is free software; you can
 *  redistribute and/or modify it under the terms of the GNU General
 *  Public License, version 2, as published by the Free Software
 *  Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA 02111-1307 USA
 *
 *  More information about the Roadrunner/pk operating system of
 *  which this file is a part is available on the World-Wide Web
 *  at: http://www.cornfed.com.
 *
 */

#include <stdio.h>
#include <string.h>

/* Must be at least one greater than 12 */
#define VENDOR_STRING_LENGTH	16

#define cpuid_asm(in,a,b,c,d) \
    asm("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in));

static char *intel_brand[] = {
    "reserved",
    "Celeron",
    "Pentium III",
    "Pentium III Xeon",
    "Pentium III",
    "reserved",
    "Mobile Pentium III",
    "Mobile Celeron",
    "Pentium 4",
    "Pentium 4",
    "Pentium 4",
    "Pentium 4 Xeon",
    "reserved",
    "reserved",
    "Pentium 4 Xeon",
    "reserved"
};

void
cpuid()
{
    u_long eax, ebx, ecx, edx;
    char vendorstring[VENDOR_STRING_LENGTH];
    int i, j;

    bzero(vendorstring, VENDOR_STRING_LENGTH);

    /* Basic CPUID information */
    cpuid_asm(0, eax, ebx, ecx, edx);

    j = 0;
    for (i = 0; i < 4; i++)
	vendorstring[j++] = ebx >> (8 * i);
    for (i = 0; i < 4; i++)
	vendorstring[j++] = edx >> (8 * i);
    for (i = 0; i < 4; i++)
	vendorstring[j++] = ecx >> (8 * i);

    if (strcmp(vendorstring, "GenuineIntel") == 0) {
	int brandid;

	kprintf("Intel");

	/* 32-bit Processor Signature */
	cpuid_asm(1, eax, ebx, ecx, edx);

	brandid = ebx & 0xff;
	if (brandid > 0 && strcmp(intel_brand[brandid], "reserved") != 0)
	    kprintf(" %s", intel_brand[brandid]);
	else {
	    int familyid = (eax >> 8) & 0x0f;
	    int modelno = (eax >> 4) & 0x0f;

	    if (familyid == 6) {
		if (modelno == 1)
		    kprintf(" Pentium Pro");
		else if (modelno == 3 || modelno == 5)
		    kprintf(" Pentium II");
		else if (modelno == 6)
		    kprintf(" Celeron");
		else if (modelno == 7 || modelno == 8 || modelno == 11)
		    kprintf(" Pentium III");
		else if (modelno == 10)
		    kprintf(" Pentium III Xeon");
	    } else if (familyid == 15)
		kprintf(" Pentium 4");
	}

    } else if (strcmp(vendorstring, "AuthenticAMD") == 0) {
	int familyid, modelno;

	kprintf("AMD");

	/* 32-bit Processor Signature */
	cpuid_asm(1, eax, ebx, ecx, edx);

	familyid = (eax >> 8) & 0x0f;
	modelno = (eax >> 4) & 0x0f;

	if (familyid == 5) {
	    if (modelno >= 0 && modelno < 4)
		kprintf(" K5");
	    else if (modelno >= 6 && modelno < 10)
		kprintf(" K6");
	} else if (familyid == 6) {
	    if (modelno == 1 || modelno == 2 || modelno == 4
		|| modelno == 6)
		kprintf(" Athlon");
	    else if (modelno == 3 || modelno == 7)
		kprintf(" Duron");
	}

    } else if (strcmp(vendorstring, "GenuineTMx86") == 0)
	kprintf("Transmeta");
    else if (strcmp(vendorstring, "CyrixInstead") == 0)
	kprintf("Cyrix");
    else
	kprintf("[%s]", vendorstring);

    kprintf(" processor\n");
}
