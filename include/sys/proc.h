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

#ifndef __PROCESS_H
#define __PROCESS_H

#include <sys/segment.h>
#include <sys/time.h>
#include <sys/vm.h>

/* Stdpaths */
#define PFDS		3
#define PFD_STDIN	0
#define PFD_STDOUT	1
#define PFD_STDERR	2

#if _KERNEL

/* Execution states */
#define PS_NULL		0
#define PS_READY	1
#define PS_RUN		2
#define PS_WAIT		3
#define PS_MUTEX	4
#define PS_EVENT	5
#define PS_SOCKET	6

typedef int (*proc_func_t) (int, char **);
typedef int proc_state_t;

struct context
{
  tss_t tss;			/* x86 Task State Segment */
  tssdesc_t tssdesc;		/* x86 Task State Segment desc */
  pt_rec_t ptrec;		/* Page tables record */
  void *kstk;			/* Kernel stack */
  void *stk;			/* User stack */
  proc_func_t start;		/* Process function */
  int argc;			/* Argv array count */
  char **argv;			/* Argv array */
};

struct proc
{
  struct context context;	/* Machine dependent context */
  int slot;
  struct proc *next;
  void *q;
  proc_state_t state;
  struct proc *wait;
  char *cwd;
  int fd[PFDS];
};

struct queue
{
  struct proc *h, *t;
};

typedef struct proc *proc_t;
typedef struct queue *queue_t;

extern struct proc proctab[];
extern proc_t current;
extern struct queue ready;

void proc_clear (proc_t proc);
void proc_sysinit ();
void proc_transfer ();
void proc_start ();
int proc_init (proc_t * proc, void *start, int argc, char **argv);
pid_t proc_exec (const char *path, int argc, char **argv);
char *proc_getcwd (char *buf, size_t size);
int proc_chdir (const char *path);
int proc_wait (pid_t pid);
void proc_exit (int status);
pid_t proc_getpid ();
int proc_getstdpath (pid_t pid, int stdpath);
void proc_setstdpath (pid_t pid, int stdpath, int fd);

#else

int getstdpath (pid_t pid, int stdpath);
void setstdpath (pid_t pid, int stdpath, int fd);

#endif /* _KERNEL */

#endif
