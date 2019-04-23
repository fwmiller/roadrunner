#!/usr/bin/python

import os
from os.path import isfile, join

binpath = '../bin'
ramdisk = 'ramdisk'

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
	# Write the length of the root directory string
	ramdiskfd.write('%u\n' % len(rootdir))

	# Write the root directory string
	ramdiskfd.write(rootdir)

	# Concatenate each binary file to the ramdisk file
	for f in files:
		if f[0] != '.':
			path = join(binpath,f)
			fd = open(path, 'rb')
			ramdiskfd.write(fd.read())
