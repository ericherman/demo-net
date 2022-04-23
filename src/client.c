/* adapted from Beej's Guide to Network Programming */
/* https://beej.us/guide/bgnet/html/ */

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa);

int main(int argc, char **argv)
{
	if (argc != 3) {
		fprintf(stderr, "usage: %s hostname port\n", argv[0]);
		exit(1);
	}
	const char *host = argv[1];
	const char *port = argv[2];

	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int rv;
	struct addrinfo *servinfo = NULL;
	if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and connect to the first we can
	int sockfd;
	struct addrinfo *p = NULL;
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				     p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	char addr_str[INET6_ADDRSTRLEN];
	memset(addr_str, 0x00, INET6_ADDRSTRLEN);
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
		  addr_str, INET6_ADDRSTRLEN);
	printf("client: connecting to %s\n", addr_str);

	freeaddrinfo(servinfo);
	servinfo = NULL;

	const size_t buf_size = 100;
	char buf[100];
	memset(buf, 0x00, buf_size);
	const size_t buf_max = buf_size - 1;
	ssize_t numbytes;
	if ((numbytes = recv(sockfd, buf, buf_max, 0)) == -1) {
		perror("recv");
		exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received '%s'\n", buf);

	close(sockfd);

	return 0;
}
