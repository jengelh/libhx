static void *p_slurp(const char *file, size_t *outsize)
{
	struct stat sb;
	int ret = 0, fd = open(file, O_RDONLY | O_BINARY);
	void *buf = NULL;
	ssize_t rdret;

	if (fd < 0) {
		fprintf(stderr, "ERROR: Slurping %s failed: %s\n",
		        file, strerror(errno));
		return NULL;
	}
	if (fstat(fd, &buf) < 0) {
		ret = errno;
		perror("fstat");
		goto out;
	}
	*outsize = sb.st_size; /* truncate if need be */
	buf = malloc(*outsize);
	if (buf == NULL) {
		ret = errno;
		perror("malloc");
		goto out;
	}
	rdret = read(fd, buf, *outsize);
	if (rdret < 0) {
		ret = errno;
		perror("read");
		free(buf);
	} else {
		*outsize = rdret;
	}
 out:
	close(fd);
	errno = ret;
	return buf;
}

