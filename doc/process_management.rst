==================
Process management
==================

Historic note: The process code comes from a time before posix_spawn. It also
relies on the POSIX functions ``fork``, ``execv``, ``execvp`` and ``pipe``, so
it may not be available everywhere. Where this is the case, the functions will
return ``-ENOSYS``.

Process metadata structure
==========================

.. code-block:: c

	#include <libHX/proc.h>

	struct HXproc {
		const struct HXproc_ops *p_ops;
		void *p_data;
		unsigned int p_flags;

		/* Following members should only be read */
		int p_stdin, p_stdout, p_stderr;
		int p_pid;
		char p_status;
		bool p_exited, p_terminated;
	};

When creating a new process with the intent of running it asynchronously (using
``HXproc_run_async``), the first three fields must be filled in by the user.

``p_ops``
	A table of callbacks, generally used for setting and/or
	restoring signals before/after execution. This member may be
	``NULL``.

``p_data``
	The user may provide a pointer of his choosing. It will be
	passed the callback functions when invoked.

``p_flags``
	Process creation flags, see below.

After the subprocess has been started, ``HXproc_run_async`` will have filled in
some fields:

``p_stdin``
	If ``HXPROC_STDIN`` was specified in ``p_flags``, ``p_stdin`` will be
	assigned the write side file descriptor of the subprocess's to-be
	stdin. The subprocess will get the read side file descriptor in this
	member. This is so that the correct fd is used in when
	``p_ops->p_postfork`` is called.

``p_stdout``
	If ``HXPROC_STDOUT`` is specified in ``p_flags``, ``p_stdout`` will be
	assigned the read side file descriptor of the subprocess's to-be
	stdout. The subprocess will get the write side file descriptor value
	from this member.

``p_stderr``
	If ``HXPROC_STDERR`` is specified in ``p_flags``, ``p_stderr`` will be
	assigned the read side file descriptor of the subprocess's to-be stderr, and
	the subprocess will get the write side fd.

``p_pid``
	The process ID of the spawned process.

Upon calling ``HXproc_wait``, further fields will have been filled when the
function returns:

``p_exited``
	Whether the process exited normally (cf. signalled/terminated).

``p_terminated``
	Whether the process was terminated (signalled).

``p_status``
	The exit status of the process or the termination signal.

Flags
-----

Possible values for the ``p_flags`` member are:

``HXPROC_STDIN``
	The subprocess's stdin file descriptor shall be connected to the master
	program, that is, not inherit the stdin of the master. Cannot be used
	with ``HXproc_run_sync`` (because there would be no one to provide data
	in a sync operation).

``HXPROC_STDOUT``
	Connect the stdout file descriptor of the subprocess with the master.
	Cannot be used with ``HXproc_run_sync``.

``HXPROC_STDERR``
	Connect the stderr file descriptor of the subprocess with the master.
	Cannot be used with ``HXproc_run_sync``.

``HXPROC_NULL_STDIN``
	The subprocess's stdin file descriptor shall be connected to
	``/dev/null``. ``HXPROC_STDIN`` and ``HXPROC_NULL_STDIN`` are mutually
	exclusive.

``HXPROC_NULL_STDOUT``
	Connect the stdout file descriptor of the subprocess to ``/dev/null``,
	thereby essentially discarding its output. ``HXPROC_STDOUT`` and
	``HXPROC_NULL_STDOUT`` are mutually exclusive.

``HXPROC_NULL_STDERR``
	Connect the stderr file descriptor of the subprocess to ``/dev/null``,
	thereby essentially discarding its output. ``HXPROC_STDERR`` and
	``HXPROC_NULL_STDERR`` are mutually exclusive.

``HXPROC_VERBOSE``
	Have the subprocess print an error message to stderr if exec'ing
	returned an error.

``HXPROC_A0``
	``argv[0]`` refers to program file, while ``argv[1]`` to the program
	invocation name, with ``argv[2]`` being the first argument, etc.
	Without this flag, ``argv[0]`` will be both the program file and
	program invocation name, and arguments begin at ``argv[1]``.

``HXPROC_EXECV``
	Normally, ``execvp`` will be used which scans ``$PATH for the program.
	Use this flag to use ``execv`` instead, which will not do such thing.

Callbacks
=========

.. code-block:: c

	#include <libHX/proc.h>

	struct HXproc_ops {
		void (*p_prefork)(void *);
		void (*p_postfork)(void *);
		void (*p_complete)(void *);
	};

``struct HXproc_ops`` provides a way to run user-specified functions just
before the fork, after, and when the process has been waited for. They can be
used to set and/or restore signals as needed, for example. The function
pointers can be ``NULL``. The ``p_data`` member is passed as an argument.

``p_prefork``
	Run immediately before calling ``fork``. This is useful for taking any
	action regarding signals, like setting ``SIGCHLD`` to ``SIG_DFL``, or
	``SIGPIPE`` to ``SIG_IGN``, for example.

``p_postfork``
	Run in the subprocess (and only there) after forking. Useful
	to do a ``setuid`` or other change in privilege level.

``p_complete``
	Run in ``HXproc_wait`` when the process has been waited for.
	Useful to restore the signal handler(s).

Process control
===============

.. code-block:: c

	#include <libHX/proc.h>

	int HXproc_run_async(const char *const *argv, struct HXproc *proc);
	int HXproc_run_sync(const char *const *argv, unsigned int flags);
	int HXproc_wait(struct HXproc *proc);

``HXproc_run_async``
	Start a subprocess according to the parameters in proc. Returns a
	negative errno code if something went wrong, or positive non-zero on
	success.

``HXproc_run_sync``
	Start a subprocess synchronously, similar to calling ``system``, but
	with the luxury of being able to specify arguments as separate strings
	(via argv) rather than one big command line that is run through the
	shell. ``flags`` is a value composed of the HXproc flags mentioned
	above. ``HXPROC_STDIN``, ``HXPROC_STDOUT`` and ``HXPROC_STDERR`` are ignored
	because there would be no one in a synchronous execution that
	could supply data to these file descriptors or read from them.[#f1]_

.. [#f1] Even for threads, please just use the async model.

``HXproc_wait``
	Wait for a subprocess to terminate, if it has not already. It will also
	retrieve the exit status of the process and store it in the ``struct
	HXproc``.

The return value will be positive non-zero on success, or negative on
error. Underlying system function's errors are returned, plus:

``EINVAL``
	Flags were not accepted.


User identity control
=====================

.. code-block: c

	#include <libHX/proc.h>

	enum HXproc_su_status {
		HXPROC_INITGROUPS_FAILED = -5,
		HXPROC_SETGID_FAILED = -4,
		HXPROC_SETUID_FAILED = -3,
		HXPROC_GROUP_NOT_FOUND = -2,
		HXPROC_USER_NOT_FOUND = -1,
		HXPROC_SU_NOOP = 0,
		HXPROC_SU_SUCCESS = 1,
	};

	enum HXproc_su_status HXproc_switch_user(const char *user, const char *group);

``HXproc_switch_user`` is a wrapper for changing process identity to an
unprivileged user. This utilizes ``setuid``, and possibly ``setgid`` plus
``initgroups``.

``user`` can either be a username or a numeric UID in string form, the latter
of which will be parsed with strtoul in automatic base. If ``user`` is ``NULL``
or the empty string, no change of process user identity occurs.

``group`` can likewise be a name or GID. When ``group`` is ``NULL``, the
process group(s) will change to the the user's group(s) — both primary and
secondary — provided a user was specified (see above). When ``gruop`` is the
empty string, no change of process group identity occurs.

The return value is an enum indicating failure with values <0, and success with
>=0.


Process information
===================

.. code-block:: c

	#include <libHX/proc.h>

	int HXproc_top_fd(void);

``HXproc_top_fd``
	This function gives a heuristic for the highest fd in the process.
	The returned number may be higher than the highest live fd actually.
	On error, negative errno is returned.
