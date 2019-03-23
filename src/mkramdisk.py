#!/usr/bin/python

import os
from os.path import isfile, join

binpath = '../bin'
ramdisk = 'ramdisk'

# Get list of binary files to include in ramdisk
files = [f for f in os.listdir(binpath) if isfile(join(binpath, f))]

# Count the list of binary files to include in ramdisk
count = 0
for f in files:
	if f[0] != '.':
		count += 1

# Create the ramdisk file
with open(join(binpath, ramdisk), 'wb') as ramdiskfd:
	# Write the count
	ramdiskfd.write('%u\n' % count)

	# Write the filename and size for each binary file
	for f in files:
		if f[0] != '.':
			size = os.path.getsize(join(binpath, f))
			ramdiskfd.write('%s,%u\n' % (f, size));

	# Concatenate each binary file to the ramdisk file
	for f in files:
		if f[0] != '.':
			path = join(binpath,f)
			fd = open(path, 'rb')
			ramdiskfd.write(fd.read())
