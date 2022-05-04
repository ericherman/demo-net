/*
** showip.c -- show IP addresses for a host given on the command line
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "ipaddr.h"

int main(int argc, char **argv)
{
	const char *host = argc > 1 ? argv[1] : "localhost";

	int err = 0;
	struct addrinfo *addrinfos = get_addrinfo_list(host, &err);
	if (!addrinfos) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
		return 1;
	}

	printf("IP addresses for %s:\n\n", host);

	for (struct addrinfo *ai = addrinfos; ai != NULL; ai = ai->ai_next) {
		int is_ipv6 = 0;
		void *addr = addrinfo_to_inaddr(ai, &is_ipv6);
		const char *ipver = is_ipv6 ? "IPv6" : "IPv4";
		// convert the IP to a string and print it:
		char ipstr[INET6_ADDRSTRLEN];
		inet_ntop(ai->ai_family, addr, ipstr, INET6_ADDRSTRLEN);
		printf("  %s: %s\n", ipver, ipstr);
	}

	freeaddrinfo(addrinfos);	// free the linked list

	return 0;
}
