/*
 *	Process management
 *	Copyright Jan Engelhardt, 2008-2009
 *
 *	This file is part of libHX. libHX is free software; you can
 *	redistribute it and/or modify it under the terms of the GNU Lesser
 *	General Public License as published by the Free Software Foundation;
 *	either version 2.1 or (at your option) any later version.
 */
#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif
#include "internal.h"
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#ifdef HAVE_SYS_RESOURCE_H
#	include <sys/resource.h>
#endif
#include <libHX/proc.h>
#ifdef _WIN32
#	include <winsock2.h>
#endif

#if defined(HAVE_INITGROUPS) && defined(HAVE_SETGID)
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

static int HXproc_switch_gid(const struct passwd *pw, gid_t gr_gid)
{
	bool do_setgid = getgid() != gr_gid || getegid() != gr_gid;
	if (do_setgid && setgid(gr_gid) != 0) {
		if (errno == 0)
			errno = -EINVAL;
		return HXPROC_SETGID_FAILED;
	}
	if (pw == NULL)
		return do_setgid ? HXPROC_SU_SUCCESS : HXPROC_SU_NOOP;

	/* pw!=NULL: user switch requested, do initgroups now. */

	if (geteuid() == pw->pw_uid) {
		/*
		 * Target identity (usually unprivileged) already reached.
		 * initgroups is unlikely to succeed.
		 */
		if (initgroups(pw->pw_name, gr_gid) < 0)
			/* ignore */;
		return do_setgid ? HXPROC_SU_SUCCESS : HXPROC_SU_NOOP;
	}
	if (initgroups(pw->pw_name, gr_gid) != 0)
		return HXPROC_INITGROUPS_FAILED;
	return do_setgid ? HXPROC_SU_SUCCESS : HXPROC_SU_NOOP;
}

static int HXproc_switch_group(const struct passwd *pw, const char *group)
{
	char *end;
	unsigned long gid = strtoul(group, &end, 10);
	const struct group *gr = *end == '\0' ? getgrgid(gid) : getgrnam(group);
	if (gr == NULL) {
		if (errno == 0)
			errno = ENOENT;
		return HXPROC_GROUP_NOT_FOUND;
	}
	return HXproc_switch_gid(pw, gr->gr_gid);
}

EXPORT_SYMBOL enum HXproc_su_status HXproc_switch_user(const char *user, const char *group)
{
	const struct passwd *pw = NULL;
	if (user != NULL && *user != '\0') {
		char *end;
		unsigned long uid = strtoul(user, &end, 10);
		pw = *end == '\0' ? getpwuid(uid) : getpwnam(user);
		if (pw == NULL) {
			if (errno == 0)
				errno = ENOENT;
			return HXPROC_USER_NOT_FOUND;
		}
	}
	int ret = HXPROC_SU_NOOP;
	if (group != NULL && *group != '\0') {
		ret = HXproc_switch_group(pw, group);
		if (ret < 0)
			return ret;
	} else if (group == NULL && pw != NULL) {
		ret = HXproc_switch_gid(pw, pw->pw_gid);
		if (ret < 0)
			return ret;
	}
	bool do_setuid = pw != NULL && (getuid() != pw->pw_uid || geteuid() != pw->pw_uid);
	if (do_setuid && setuid(pw->pw_uid) != 0) {
		if (errno == 0)
			errno = -EINVAL;
		return HXPROC_SETUID_FAILED;
	}
	return do_setuid ? HXPROC_SU_SUCCESS : ret;
}

#else

EXPORT_SYMBOL enum HXproc_su_status HXproc_switch_user(const char *user, const char *group)
{
	return -ENOSYS;
}

#endif /* HAVE_lots */

#if defined(HAVE_FORK) && defined(HAVE_PIPE) && defined(HAVE_EXECV) && \
    defined(HAVE_EXECVP)
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libHX/defs.h>
#include "internal.h"

#ifdef _WIN32
#	define NULL_DEVICE "nul"
#else
#	define NULL_DEVICE "/dev/null"
#endif

/**
 * HXproc_build_pipes -
 * @fd_request:	user array to tell us which pipe sets to create
 * @p:		result array
 *
 * Create some pipes.
 * Explicitly initialize the @p array with -1 so that we can call close()
 * on all of them without any side effects.
 */
static __inline__ int
HXproc_build_pipes(const struct HXproc *proc, int (*p)[2])
{
	unsigned int x, y;

	for (x = 0; x < 3; ++x)
		for (y = 0; y < 2; ++y)
			p[x][y] = -1;

	if ((proc->p_flags & HXPROC_STDIN) && pipe(p[0]) < 0)
		return -errno;
	if ((proc->p_flags & HXPROC_STDOUT) && pipe(p[1]) < 0)
		return -errno;
	if ((proc->p_flags & HXPROC_STDERR) && pipe(p[2]) < 0)
		return -errno;

	return 1;
}

/**
 * HXproc_close_pipes -
 * @p:	pipe fds to close
 *
 * In @p, there might be some fds that are -1 (due to the p[x][y] = -1 in
 * HXproc_build_pipes()). That is ok, as closing -1 does not do anything.
 */
static void HXproc_close_pipes(int (*p)[2])
{
#define xc(fd) do { close(fd); (fd) = -1; } while (false)
	if (p[0][0] >= 0)
		xc(p[0][0]);
	if (p[0][1] >= 0)
		xc(p[0][1]);
	if (p[1][0] >= 0)
		xc(p[1][0]);
	if (p[1][1] >= 0)
		xc(p[1][1]);
	if (p[2][0] >= 0)
		xc(p[2][0]);
	if (p[2][1] >= 0)
		xc(p[2][1]);
#undef xc
}

/**
 * HXproc_run_async -
 * @argv:	program and arguments
 * @proc:	control block with flags, also used to return info like fds
 *
 * Sets up pipes and runs the specified program.
 */
EXPORT_SYMBOL int
HXproc_run_async(const char *const *argv, struct HXproc *proc)
{
	int pipes[3][2], nullfd = -1, ret, saved_errno;

	if (argv == NULL || *argv == NULL)
		return -EFAULT;
	if ((proc->p_flags & (HXPROC_STDIN | HXPROC_NULL_STDIN)) ==
	    (HXPROC_STDIN | HXPROC_NULL_STDIN))
		return -EINVAL;
	if ((proc->p_flags & (HXPROC_STDOUT | HXPROC_NULL_STDOUT)) ==
	    (HXPROC_STDOUT | HXPROC_NULL_STDOUT))
		return -EINVAL;
	if ((proc->p_flags & (HXPROC_STDERR | HXPROC_NULL_STDERR)) ==
	    (HXPROC_STDERR | HXPROC_NULL_STDERR))
		return -EINVAL;
	if (proc->p_flags & (HXPROC_NULL_STDIN | HXPROC_NULL_STDOUT |
	    HXPROC_NULL_STDERR)) {
		if ((nullfd = open(NULL_DEVICE, O_RDWR)) < 0)
			return -errno;
	}
	proc->p_stdin = proc->p_stdout = proc->p_stderr = -1;
	if ((ret = HXproc_build_pipes(proc, pipes)) <= 0) {
		saved_errno = errno;
		HXproc_close_pipes(pipes);
		if (nullfd >= 0)
			close(nullfd);
		errno = saved_errno;
		return ret;
	}

	if (proc->p_ops != NULL && proc->p_ops->p_prefork != NULL)
		proc->p_ops->p_prefork(proc->p_data);
	if ((proc->p_pid = fork()) < 0) {
		saved_errno = errno;
		if (proc->p_ops != NULL && proc->p_ops->p_complete != NULL)
			proc->p_ops->p_complete(proc->p_data);
		HXproc_close_pipes(pipes);
		if (nullfd >= 0)
			close(nullfd);
		return -(errno = saved_errno);
	} else if (proc->p_pid == 0) {
		const char *prog = *argv;

		/*
		 * Put file descriptors in place... and do so before postfork,
		 * as someone could have used proc.p_data = &proc; already.
		 *
		 * Take a dup of the pipe ends, so that close_pipes does not
		 * accidentally close them.
		 */
		if (proc->p_flags & HXPROC_STDIN)
			proc->p_stdin = dup(pipes[0][0]);
		else if (proc->p_flags & HXPROC_NULL_STDIN)
			proc->p_stdin = dup(nullfd);
		if (proc->p_flags & HXPROC_STDOUT)
			proc->p_stdout = dup(pipes[1][1]);
		else if (proc->p_flags & HXPROC_NULL_STDOUT)
			proc->p_stdout = dup(nullfd);
		if (proc->p_flags & HXPROC_STDERR)
			proc->p_stderr = dup(pipes[2][1]);
		else if (proc->p_flags & HXPROC_NULL_STDERR)
			proc->p_stderr = dup(nullfd);
		if (proc->p_ops != NULL && proc->p_ops->p_postfork != NULL)
			proc->p_ops->p_postfork(proc->p_data);

		/*
		 * The rest of housekeeping. Now move the pipe ends onto
		 * their final fds.
		 */
		HXproc_close_pipes(pipes);
		if ((proc->p_flags & (HXPROC_STDIN | HXPROC_NULL_STDIN)) &&
		    proc->p_stdin >= 0 && proc->p_stdin != STDIN_FILENO) {
			dup2(proc->p_stdin, STDIN_FILENO);
			close(proc->p_stdin);
		}
		if ((proc->p_flags & (HXPROC_STDOUT | HXPROC_NULL_STDOUT)) &&
		    proc->p_stdout >= 0 && proc->p_stdout != STDOUT_FILENO) {
			dup2(proc->p_stdout, STDOUT_FILENO);
			close(proc->p_stdout);
		}
		if ((proc->p_flags & (HXPROC_STDERR | HXPROC_NULL_STDERR)) &&
		    proc->p_stderr >= 0 && proc->p_stderr != STDERR_FILENO) {
			dup2(proc->p_stderr, STDERR_FILENO);
			close(proc->p_stderr);
		}
		if (nullfd >= 0)
			close(nullfd);
		if (proc->p_flags & HXPROC_A0)
			++argv;
		if (proc->p_flags & HXPROC_EXECV)
			execv(prog, const_cast2(char * const *, argv));
		else
			execvp(prog, const_cast2(char * const *, argv));
		if (proc->p_flags & HXPROC_VERBOSE)
			fprintf(stderr, "%s: %s: %s\n", __func__,
			        prog, strerror(errno));
		_exit(-1);
	}

	if (proc->p_flags & HXPROC_STDIN) {
		close(pipes[0][0]);
		proc->p_stdin = pipes[0][1];
	}
	if (proc->p_flags & HXPROC_STDOUT) {
		close(pipes[1][1]);
		proc->p_stdout = pipes[1][0];
	}
	if (proc->p_flags & HXPROC_STDERR) {
		close(pipes[2][1]);
		proc->p_stderr = pipes[2][0];
	}
	if (nullfd >= 0)
		close(nullfd);
	return 1;
}

EXPORT_SYMBOL int HXproc_run_sync(const char *const *argv, unsigned int flags)
{
	struct HXproc proc;
	int ret;

	memset(&proc, 0, sizeof(proc));
	/*
	 * Assigning file descriptors makes no sense because they would not
	 * be read from. %HXPROC_NULL_* is ok of course.
	 */
	if (flags & (HXPROC_STDIN | HXPROC_STDOUT | HXPROC_STDERR))
		return -EINVAL;
	proc.p_flags = flags;
	if ((ret = HXproc_run_async(argv, &proc)) <= 0)
		return ret;
	return HXproc_wait(&proc);
}

EXPORT_SYMBOL int HXproc_wait(struct HXproc *proc)
{
	int status;

	/* User has to close the pipes. Do not do it here. */

	if (waitpid(proc->p_pid, &status, 0) < 0)
		return -errno;
	if (proc->p_ops != NULL && proc->p_ops->p_complete != NULL)
		proc->p_ops->p_complete(proc->p_data);

	if ((proc->p_exited = WIFEXITED(status)))
		proc->p_status = WEXITSTATUS(status);
	if ((proc->p_terminated = WIFSIGNALED(status)))
		proc->p_status = WTERMSIG(status);
	if (proc->p_terminated)
		return static_cast(unsigned int,
		       static_cast(unsigned char, proc->p_status)) << 16;
	return static_cast(unsigned char, proc->p_status);
}

#else

EXPORT_SYMBOL int HXproc_run_async(const char *const *argv, struct HXproc *p)
{
	return -ENOSYS;
}

EXPORT_SYMBOL int HXproc_run_sync(const char *const *argv, unsigned int flags)
{
	/* Might use system() here... */
	return -ENOSYS;
}

EXPORT_SYMBOL int HXproc_wait(struct HXproc *p)
{
	return -ENOSYS;
}

#endif /* HAVE_lots */

EXPORT_SYMBOL int HXproc_top_fd(void)
{
#ifndef _WIN32
	struct rlimit r;
	if (getrlimit(RLIMIT_NOFILE, &r) == 0) {
		if (r.rlim_max > INT_MAX)
			r.rlim_max = INT_MAX;
		return r.rlim_max;
	}
	long v = sysconf(_SC_OPEN_MAX);
	if (v >= 0) {
		if (v > INT_MAX)
			v = INT_MAX;
		return v;
	}
#endif
	return FD_SETSIZE;
}
