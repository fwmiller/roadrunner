#!/usr/bin/python

import os
from os.path import isfile, join

binpath = '../bin'
ramfs = 'ramfs'

# Get list of binary files to include in ramfs
files = [f for f in os.listdir(binpath) if isfile(join(binpath, f))]

# Count the list of binary files to include in ramfs
count = 0
for f in files:
	if f[0] != '.':
		count += 1

# Create the ramfs file
with open(join(binpath, ramfs), 'wb') as ramfsfd:
	# Write the count
	ramfsfd.write('%u\n' % count)

	# Write the filename and size for each binary file
	for f in files:
		if f[0] != '.':
			size = os.path.getsize(join(binpath, f))
			ramfsfd.write('%s,%u\n' % (f, size));

	# Concatenate each binary file to the ramfs file
	for f in files:
		if f[0] != '.':
			path = join(binpath,f)
			fd = open(path, 'rb')
			ramfsfd.write(fd.read())
