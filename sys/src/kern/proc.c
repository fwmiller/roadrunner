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

#include <errno.h>
#include <fcntl.h>
#include <fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys.h>
#include <sys/config.h>
#include <sys/intr.h>
#include <sys/mem.h>
#include <sys/proc.h>
#include <sys/queue.h>
#include <unistd.h>

static char *cwdpaths = NULL;
struct proc proctab[PROCS];
proc_t current = NULL;
struct queue ready;

void proc_clear(proc_t proc)
{
	int i;

	if (proc->context.ptrec != NULL) {
		pt_push(proc->context.ptrec);
		proc->context.ptrec = NULL;
	}
	proc->context.kstk = NULL;
	proc->context.stk = NULL;
	proc->context.start = NULL;
	proc->context.argc = 0;
	proc->context.argv = NULL;
	proc->next = NULL;
	proc->q = NULL;
	proc->state = PS_NULL;
	proc->wait = NULL;
	bzero(proc->cwd, PATH_LENGTH);
	strcpy(proc->cwd, "/");
	for (i = 0; i < PFDS; i++)
		proc->fd[i] = (-1);
}

void proc_sysinit()
{
	int i;

	cwdpaths = (char *)kmalloc(PROCS * PATH_LENGTH);
	bzero(cwdpaths, PROCS * PATH_LENGTH);

	for (i = 0; i < PROCS; i++) {
		proctab[i].context.tss = &(tsstab[i]);
		proctab[i].context.tssdesc = &(tssdesctab[i]);
		proctab[i].slot = i;
		proctab[i].cwd = cwdpaths + (i * PATH_LENGTH);
		proc_clear(&(proctab[i]));
	}
}

void proc_transfer()
{
	proc_t proc = current;
	struct {
		u_long eip;
		u_short cs;
	} __attribute__ ((packed)) tssdesc;

	/* Assume interrupts are disabled */

	if ((current = remfirstq(&ready)) == NULL) {
		kprintf("proc_transfer: no ready processes\n");
		halt();
	}
	current->state = PS_RUN;
	if (current == proc) {
		enable;
		return;
	}
	tssdesc.eip = 0;
	tssdesc.cs =
	    (u_short) ((u_long) (current->context.tssdesc) - (u_long) gdt);
	jmptss(&tssdesc);	/* enables interrupts */
}

void proc_start()
{
	(current->context.start) (current->context.argc, current->context.argv);
	proc_exit(0);
	/* Not reached */
}

int proc_init(proc_t * proc, void *start, int argc, char **argv)
{
	void *kstk, *stk;
	int i;

	/* Find process table slot */
	for (i = 0; i < PROCS; i++)
		if (proctab[i].state == PS_NULL)
			break;
	if (i == PROCS) {
		*proc = NULL;
		return EAGAIN;
	}

	/* Allocate kernel stack */
	kstk = mem_alloc(STACK_SIZE);
	if (kstk == NULL) {
		*proc = NULL;
		return ENOMEM;
	}

	/* Allocate user stack */
	stk = mem_alloc(STACK_SIZE);
	if (stk == NULL) {
		mem_free(kstk);
		*proc = NULL;
		return ENOMEM;
	}
	*proc = &(proctab[i]);
	mem_transfer(kstk, *proc);
	mem_transfer(stk, *proc);
	strcpy((*proc)->cwd, current->cwd);

	/* Duplicate access to file descriptors */
#if 0
	{
		char fullpath[PATH_LENGTH];
		file_t file;

		bzero(fullpath, PATH_LENGTH);
		mkpath(filetab[current->fd[PFD_STDERR]].fs->path,
		       filetab[current->fd[PFD_STDERR]].path, fullpath);
		file_open(fullpath, O_RDONLY, &file);
		(*proc)->fd[PFD_STDERR] = file->slot;

		bzero(fullpath, PATH_LENGTH);
		mkpath(filetab[current->fd[PFD_STDIN]].fs->path,
		       filetab[current->fd[PFD_STDIN]].path, fullpath);
		file_open(fullpath, O_RDONLY, &file);
		(*proc)->fd[PFD_STDIN] = file->slot;

		bzero(fullpath, PATH_LENGTH);
		mkpath(filetab[current->fd[PFD_STDOUT]].fs->path,
		       filetab[current->fd[PFD_STDOUT]].path, fullpath);
		file_open(fullpath, O_RDONLY, &file);
		(*proc)->fd[PFD_STDOUT] = file->slot;
	}
#endif
	filetab[current->fd[PFD_STDIN]].refcnt++;
	filetab[current->fd[PFD_STDOUT]].refcnt++;
	filetab[current->fd[PFD_STDERR]].refcnt++;
	(*proc)->fd[PFD_STDIN] = current->fd[PFD_STDIN];
	(*proc)->fd[PFD_STDOUT] = current->fd[PFD_STDOUT];
	(*proc)->fd[PFD_STDERR] = current->fd[PFD_STDERR];

	/* Initialize execution context */
	(*proc)->context.kstk = kstk;
	(*proc)->context.stk = stk;
	(*proc)->context.start = start;
	tssinit((*proc)->context.tss, (u_long) kstk, STACK_SIZE,
		(u_long) stk, STACK_SIZE);

	/* Initialize memory protection */

	/* XXX Need to do error handling on pt_pop() */
	(*proc)->context.ptrec = pt_pop();

	(*proc)->context.tss->cr3 = (u_long) (*proc)->context.ptrec->pd;
	vm_map_init((pt_t) (*proc)->context.tss->cr3);
	vm_kmap((pt_t) (*proc)->context.tss->cr3);

	vm_map_range((pt_t) (*proc)->context.tss->cr3,
		     kstk, STACK_SIZE, PTE_WRITE | PTE_PRESENT);
	vm_map_range((pt_t) (*proc)->context.tss->cr3,
		     stk, STACK_SIZE, PTE_WRITE | PTE_USER | PTE_PRESENT);

	/* Handle argv array */
	if (argv != NULL) {
		region_t r;

		r = valid_region(argv);
		if (r == NULL) {
			(*proc)->context.argc = 0;
			(*proc)->context.argv = NULL;

		} else {
			(*proc)->context.argc = argc;
			(*proc)->context.argv = argv;

			/* Transfer ownership of argv array memory to new process */
			mem_transfer(argv, *proc);
			vm_map_range((pt_t) (*proc)->context.tss->cr3, argv,
				     r->len,
				     PTE_WRITE | PTE_USER | PTE_PRESENT);
			vm_unmap_range((pt_t) current->context.tss->cr3, argv,
				       r->len);
		}
	}
	return 0;
}

pid_t proc_exec(const char *path, int argc, char **argv)
{
	proc_t proc;
	char *prog, *start;
	u_long size;
	int result;

	result = load(path, &prog, &size, &start);
	if (result < 0) {
#if _DEBUG
		kprintf("proc_exec: could not load %s (%s)\n",
			path, strerror(result));
#endif
		return result;
	}
	disable;

	result = proc_init(&proc, start, argc, argv);
	if (result < 0) {
#if _DEBUG
		kprintf("proc_exec: init failed (%s)\n", strerror(result));
#endif
		enable;
		return result;
	}
	/* Transfer ownership of program memory to new process */
	mem_transfer(prog, proc);
	vm_map_range((pt_t) proc->context.tss->cr3, prog, size,
		     PTE_WRITE | PTE_USER | PTE_PRESENT);
	vm_unmap_range((pt_t) current->context.tss->cr3, prog, size);

	/* Make new process ready to execute */
	insq(proc, &ready);
	current->state = PS_READY;
	insq(current, &ready);
	proc_transfer();	/* enables interrupts */
	return proc->slot;
}

char *proc_getcwd(char *buf, size_t size)
{
	if (buf == NULL)
		return NULL;

	disable;
	strncpy(buf, current->cwd, size);
	enable;
	return buf;
}

int proc_chdir(const char *path)
{
	char *fullpath;
	file_t file;
	int result;

	if (path == NULL || strlen(path) > (PATH_LENGTH - 1))
		return EINVAL;

	/* Make sure we have an absolute path */
	if (path[0] == '/')
		fullpath = path;
	else {
		fullpath = (char *)malloc(PATH_LENGTH);
		if (fullpath == NULL)
			return ENOMEM;
		mkpath(current->cwd, path, fullpath);
	}
	/* Check whether directory exists */
	result = file_open(fullpath, O_RDONLY, &file);
	if (result < 0) {
		if (fullpath != path)
			free(fullpath);
		return result;
	}
	/* Make sure the file is really a directory */
	if (!(file->flags & F_DIR)) {
		file_close(file);
		if (fullpath != path)
			free(fullpath);
		return ENOTDIR;
	}
	file_close(file);

	disable;

	/* Set new working directory for current process */
	bzero(current->cwd, PATH_LENGTH);
	strcpy(current->cwd, fullpath);

	enable;

	if (fullpath != path)
		free(fullpath);
	return 0;
}

int proc_wait(pid_t pid)
{
	proc_t proc;

	if (pid < 0 || pid >= PROCS)
		return EINVAL;
	proc = &(proctab[pid]);

	disable;

	if (proc == current) {
		enable;
		return EINVAL;
	}
	if (proc->state == PS_NULL) {
		enable;
		return ESRCH;
	}
	current->state = PS_WAIT;
	current->next = proc->wait;
	proc->wait = current;
	proc_transfer();	/* enables interrupts */
	return 0;
}

void proc_exit(int status)
{
	proc_t proc;

	/* Close stdpath file descriptors */
	file_close(&(filetab[current->fd[PFD_STDIN]]));
	file_close(&(filetab[current->fd[PFD_STDOUT]]));
	file_close(&(filetab[current->fd[PFD_STDERR]]));

	disable;

	/* Schedule any waiting processes */
	for (;;) {
		proc = current->wait;
		if (proc == NULL)
			break;
		current->wait = proc->next;
		proc->next = NULL;
		proc->state = PS_READY;
		insq(proc, &ready);
	}
	/* Free all memory allocated to the process */
	mem_reclaim(current);

	/* Reclaim process table entry */
	proc_clear(current);

	/* Transfer control to the next ready process */
	proc_transfer();	/* enables interrupts */

	kprintf("proc_exit: attempt to restart terminated process\n");
	halt();
	/* Not reached */
}

pid_t proc_getpid()
{
	pid_t pid;

	disable;
	pid = current->slot;
	enable;
	return pid;
}

int proc_getstdpath(pid_t pid, int stdpath)
{
	int fd;

	if (pid < 0 || pid >= PROCS || proctab[pid].state == PS_NULL)
		return EINVAL;
	disable;
	fd = proctab[pid].fd[stdpath];
	enable;
	return fd;
}

void proc_setstdpath(pid_t pid, int stdpath, int fd)
{
	if (pid < 0 || pid >= PROCS || proctab[pid].state == PS_NULL)
		return;
	disable;
	proctab[pid].fd[stdpath] = fd;
	enable;
}
