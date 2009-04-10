#include "config.h"
#include "internal.h"

#if !defined(HAVE_FORK) || !defined(HAVE_PIPE) || !defined(HAVE_EXECV) || \
    !defined(HAVE_EXECVP)
#include <errno.h>
#include <libHX/proc.h>

struct HXproc;

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

#else /* HAVE_FORK, HAVE_PIPE, HAVE_EXECVE */

#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libHX/defs.h>
#include <libHX/proc.h>
#include "internal.h"

/**
 * HXproc_build_pipes -
 * @fd_request:	user array to tell us which pipe sets to create
 * @p:		result array
 *
 * Create some pipes.
 * Explicitly initialize the @p array with -1 so that we can call close()
 * on all of them without any side effects.
 */
static inline int HXproc_build_pipes(const struct HXproc *proc, int (*p)[2])
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
	if (p[0][0] >= 0)
		close(p[0][0]);
	if (p[0][1] >= 0)
		close(p[0][1]);
	if (p[1][0] >= 0)
		close(p[1][0]);
	if (p[1][1] >= 0)
		close(p[1][1]);
	if (p[2][0] >= 0)
		close(p[2][0]);
	if (p[2][1] >= 0)
		close(p[2][1]);
}

/**
 * HXproc_run_async -
 * @argv:	program and arguments
 * @proc:	control block with flags, also used to return info like fds
 *
 * Sets up pipes and runs the specified program.
 */
EXPORT_SYMBOL int HXproc_run_async(const char *const *argv, struct HXproc *proc)
{
	int pipes[3][2], ret, saved_errno;

	proc->p_stdin = proc->p_stdout = proc->p_stderr = -1;
	if ((ret = HXproc_build_pipes(proc, pipes)) <= 0)
		return ret;

	if (proc->p_ops != NULL && proc->p_ops->p_prefork != NULL)
		proc->p_ops->p_prefork(proc->p_data);
	if ((proc->p_pid = fork()) < 0) {
		saved_errno = errno;
		if (proc->p_ops != NULL && proc->p_ops->p_complete != NULL)
			proc->p_ops->p_complete(proc->p_data);
		HXproc_close_pipes(pipes);
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
		if (proc->p_flags & HXPROC_STDOUT)
			proc->p_stdout = dup(pipes[1][1]);
		if (proc->p_flags & HXPROC_STDERR)
			proc->p_stderr = dup(pipes[2][1]);
		if (proc->p_ops != NULL && proc->p_ops->p_postfork != NULL)
			proc->p_ops->p_postfork(proc->p_data);

		/*
		 * The rest of housekeeping. Now move the pipe ends onto
		 * their final fds.
		 */
		HXproc_close_pipes(pipes);
		if ((proc->p_flags & HXPROC_STDIN) &&
		    proc->p_stdin != STDIN_FILENO) {
			dup2(proc->p_stdin, STDIN_FILENO);
			close(proc->p_stdin);
		}
		if ((proc->p_flags & HXPROC_STDOUT) &&
		    proc->p_stdout != STDOUT_FILENO) {
			dup2(proc->p_stdout, STDOUT_FILENO);
			close(proc->p_stdout);
		}
		if ((proc->p_flags & HXPROC_STDERR) &&
		    proc->p_stderr != STDERR_FILENO) {
			dup2(proc->p_stderr, STDERR_FILENO);
			close(proc->p_stderr);
		}
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

	return 1;
}

EXPORT_SYMBOL int HXproc_run_sync(const char *const *argv, unsigned int flags)
{
	struct HXproc proc;
	int ret;

	memset(&proc, 0, sizeof(proc));
	/*
	 * Assigning file descriptos makes no sense because they would not
	 * be read from.
	 */
	proc.p_flags = flags & ~(HXPROC_STDIN | HXPROC_STDOUT | HXPROC_STDERR);
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

#endif /* HAVE_lots */
