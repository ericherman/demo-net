/* adapted from Beej's Guide to Network Programming */
/* https://beej.us/guide/bgnet/html/ */

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void *get_in_addr(struct sockaddr *sa);

void sigchld_handler(int s)
{
	(void)s;

	while (1) {
		pid_t pid = -1;
		int *wstatus = NULL;
		int options = WNOHANG;

		int saved_errno = errno;
		pid_t result = waitpid(pid, wstatus, options);
		errno = saved_errno;

		if (result <= 0) {
			return;
		}
	}
}

int child_accept(int new_fd)
{
	if (send(new_fd, "hello, world", 13, 0) == -1) {
		perror("send");
		exit(EXIT_FAILURE);
	}
	close(new_fd);
	exit(EXIT_SUCCESS);
}

int main_accept_loop(int sockfd)
{
	while (1) {
		struct sockaddr_storage their_addr;
		struct sockaddr *their_sa = (struct sockaddr *)&their_addr;
		socklen_t sin_size = sizeof(struct sockaddr_storage);
		int new_fd = accept(sockfd, their_sa, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		char addr_str[INET6_ADDRSTRLEN];
		size_t addr_str_size = INET6_ADDRSTRLEN;
		inet_ntop(their_addr.ss_family, get_in_addr(their_sa), addr_str,
			  addr_str_size);
		printf("server: got connection from %s\n", addr_str);

		pid_t child_pid = fork();
		if (child_pid) {
			close(new_fd);	// parent doesn't need this
		} else {
			close(sockfd);	// child doesn't need the listener
			child_accept(new_fd);
		}
	}
}

int main(int argc, char **argv)
{
	const char *port = NULL;

	if (argc > 1) {
		port = argv[1];
	}

	struct addrinfo hints;
	memset(&hints, 0x00, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *servinfo = NULL;
	int rv;
	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and bind to the first we can
	int sockfd;
	struct addrinfo *p = NULL;
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				     p->ai_protocol)) == -1) {
			perror("socket");
			continue;
		}

		int yes = 1;
		size_t yes_size = sizeof(int);
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, yes_size)
		    == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo);	// all done with this structure

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	const int backlog = 8;
	if (listen(sockfd, backlog) == -1) {
		perror("listen");
		exit(1);
	}
	// reap all dead processes
	struct sigaction sa;
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");
	main_accept_loop(sockfd);

	return 0;
}
