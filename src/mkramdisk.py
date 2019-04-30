#!/usr/bin/python

import os
from os.path import isfile, join

blksize = 512
binpath = '../bin'
ramdisk = 'ramdisk'

def align(val, size):
	a = val + blksize - 1
	a = a / blksize
	a = a * blksize
	return a

# Get list of binary files to include in ramdisk
files = [f for f in os.listdir(binpath) if isfile(join(binpath, f))]

# Count the list of binary files to include in ramdisk
rootdir = ""
for f in files:
	if f != ramdisk and f[0] != '.':
		rootdir += f
		rootdir += ','
		rootdir += str(os.path.getsize(join(binpath, f)))
		rootdir += '\n'

# Create the ramdisk file
with open(join(binpath, ramdisk), 'wb') as ramdiskfd:
	# Write bootblk
	bootblk = "%u" % len(rootdir)
	bootblklen = len(bootblk)
	i = 0
	while i < blksize - bootblklen:
		bootblk += '\0'
		i += 1

	ramdiskfd.write(bootblk)

	# Write the root directory string
	rootdirlen = len(rootdir)
	i = 0
	while i < blksize - (rootdirlen % blksize):
		rootdir += '\0'
		i += 1

	ramdiskfd.write(rootdir)

	# Concatenate each binary file to the ramdisk file
	for f in files:
		if f[0] != '.':
			path = join(binpath,f)
			fd = open(path, 'rb')
			binary = fd.read()
			ramdiskfd.write(binary)
			binarylen = len(binary)
			alignedbinarylen = align(binarylen, blksize)
			pad = alignedbinarylen - binarylen
			i = 0
			while i < pad:
				ramdiskfd.write('\0')
				i += 1
