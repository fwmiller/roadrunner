/* 
 *  Roadrunner/pk
 *    Copyright (C) 1989-2001  Cornfed Systems, Inc.
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

#ifndef __SYSCALL_H
#define __SYSCALL_H

/* System call interface */
#define SYSCALL_NULL			0
#define SYSCALL_ACCEPT			1
#define SYSCALL_ATTR			2
#define SYSCALL_BIND			3
#define SYSCALL_CHDIR			4
#define SYSCALL_CLOSE			5
#define SYSCALL_CONNECT			6
#define SYSCALL_EXEC			7
#define SYSCALL_EXIT			8
#define SYSCALL_FREE			9
#define SYSCALL_GETCWD			10
#define SYSCALL_GETFSTAB		11
#define SYSCALL_GETPID			12
#define SYSCALL_GETSTDPATH		13
#define SYSCALL_GETTIMEOFDAY		14
#define SYSCALL_HALT			15
#define SYSCALL_IOCTL			16
#define SYSCALL_LISTEN			17
#define SYSCALL_LOAD			18
#define SYSCALL_MALLOC			19
#define SYSCALL_MKDIR			20
#define SYSCALL_MOUNT			21
#define SYSCALL_OPEN			22
#define SYSCALL_READ			23
#define SYSCALL_READDIR			24
#define SYSCALL_REBOOT			25
#define SYSCALL_RECV			26
#define SYSCALL_SEND			27
#define SYSCALL_SETSTDPATH		28
#define SYSCALL_SHUTDOWN		29
#define SYSCALL_SOCKET			30
#define SYSCALL_UNAME			31
#define SYSCALL_UNLINK			32
#define SYSCALL_UNMOUNT			33
#define SYSCALL_WAIT			34
#define SYSCALL_WRITE			35

int syscall (int syscallno, void *params);

#endif
