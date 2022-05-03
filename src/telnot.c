/*
** telnot.c -- Not telnet, but can be used in place of telnet for
**             the guide demos.
**
** This doesn't implement the telnet protocol in the least.
**
** Usage: telnot hostname port
**
** Then type things and hit RETURN to send them. (It uses the current
** terminal line discipline, which is probably line-buffered so nothing
** will get sent until you hit RETURN.) It will print things to
** standard output as it receives them.
**
** Hit ^C to break out.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>

#include <arpa/inet.h>

void *get_in_addr(struct sockaddr *sa);

int socket_connect(const char *hostname, const char *port, char *addr_str,
		   size_t addr_str_size, FILE *errlog)
{
	assert(addr_str_size >= INET6_ADDRSTRLEN);

	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int sockfd = -1;
	int rv = 0;
	struct addrinfo *servinfo = NULL;
	if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		if (errlog) {
			fprintf(errlog, "getaddrinfo: %s\n", gai_strerror(rv));
		}
		sockfd = -1;
	}

	struct addrinfo *p = NULL;
	// loop through all the results and connect to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				     p->ai_protocol)) == -1) {
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			continue;
		}

		break;
	}
	if (p == NULL) {
		sockfd = -1;
	} else {
		int afamily = p->ai_family;
		const void *src = get_in_addr((struct sockaddr *)p->ai_addr);
		memset(addr_str, 0x00, addr_str_size);
		inet_ntop(afamily, src, addr_str, addr_str_size);
	}
	freeaddrinfo(servinfo);	// All done with this structure

	return sockfd;
}

// Poll fd0, fd1 for incoming data (ready-to-read)
int read_write_fds_loop(int fd0in, int fd0out, int fd1in, int fd1out)
{
	struct pollfd fds[2];

	fds[0].fd = fd0in;
	fds[0].events = POLLIN;

	fds[1].fd = fd1in;
	fds[1].events = POLLIN;

	// Not reached--use ^C to exit.

	for (;;) {
		if (poll(fds, 2, -1) == -1) {
			perror("poll");
			return EXIT_FAILURE;
		}

		for (int i = 0; i < 2; i++) {
			// Check for ready-to-read
			if (!(fds[i].revents & POLLIN)) {
				continue;
			}
			// Compute where to write data. If we're stdin (0),
			// we'll write to the sockfd. If we're the sockfd, we'll
			// write to stdout (1).
			int outfd = fds[i].fd == fd0in ? fd1out : fd0out;

			const size_t buf_size = 1000;
			char buf[1000];

			// We use read() and write() in here since those work on
			// all fds, not just sockets. send() and recv() would
			// fail on stdin and stdout since they're not sockets.
			int readbytes = 0;
			if ((readbytes = read(fds[i].fd, buf, buf_size)) == -1) {
				perror("read");
				return EXIT_FAILURE;
			}
			// Write all data out
			char *p = buf;
			int remainingbytes = readbytes;
			int writebytes = 0;
			while (remainingbytes > 0) {
				if ((writebytes =
				     write(outfd, p, remainingbytes)) == -1) {
					perror("write");
					return EXIT_FAILURE;
				}

				p += writebytes;
				remainingbytes -= writebytes;
			}
		}
	}

	// not reached;
	return -1;
}

/**
 * Main
 */
int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "usage: telnot hostname port\n");
		exit(1);
	}

	const char *hostname = argv[1];
	const char *port = argv[2];
	char addr_str[INET6_ADDRSTRLEN];
	int sockfd =
	    socket_connect(hostname, port, addr_str, INET6_ADDRSTRLEN, stderr);
	if (sockfd < 0) {
		fprintf(stderr, "client: failed to connect\n");
		return EXIT_FAILURE;
	}
	printf("Connected to %s port %s\n", addr_str, port);

	printf("Hit ^C to exit\n");

	int stdinfd = 0;
	int stdoutfd = 1;
	int err = read_write_fds_loop(stdinfd, stdoutfd, sockfd, sockfd);
	close(sockfd);

	return err;
}
