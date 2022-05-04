#include "ipaddr.h"
#include <assert.h>
#include <string.h>

void *sockaddr_to_inaddr(struct sockaddr *sa, int *is_ipv6)
{
	_Static_assert(sizeof(struct sockaddr_in) == sizeof(struct sockaddr),
		       "size of sockaddr_in should equal size of sockaddr");
	assert(sa);
	assert(is_ipv6);
	assert(sa->sa_family == AF_INET6 || sa->sa_family == AF_INET);

	*is_ipv6 = sa->sa_family == AF_INET6 ? 1 : 0;

	if (*is_ipv6) {
		struct sockaddr_in6 *sai6 = (struct sockaddr_in6 *)sa;
		return &(sai6->sin6_addr);
	}
	// IPv4
	struct sockaddr_in *sai4 = (struct sockaddr_in *)sa;
	return &(sai4->sin_addr);
}

void *addrinfo_to_inaddr(struct addrinfo *ai, int *is_ipv6)
{
	assert(sa);
	assert(is_ipv6);
	assert(sa->sa_family == AF_INET6 || sa->sa_family == AF_INET);

	*is_ipv6 = ai->ai_family == AF_INET6 ? 1 : 0;

	// get the pointer to the address itself,
	// different fields in IPv4 and IPv6:
	if (*is_ipv6) {
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ai->ai_addr;
		return &(ipv6->sin6_addr);
	}
	// IPv4
	struct sockaddr_in *ipv4 = (struct sockaddr_in *)ai->ai_addr;
	return &(ipv4->sin_addr);
}

struct addrinfo *get_addrinfo_list(const char *host, int *err)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;	// AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;

	int status = 0;
	struct addrinfo *addrinfos = NULL;
	if ((status = getaddrinfo(host, NULL, &hints, &addrinfos)) != 0) {
		if (err) {
			*err = status;
		}
		return NULL;
	}
	return addrinfos;
}
