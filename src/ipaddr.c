#include <arpa/inet.h>

void *get_in_addr(struct sockaddr *sa)
{
	_Static_assert(sizeof(struct sockaddr_in) == sizeof(struct sockaddr),
		       "size of sockaddr_in should equal size of sockaddr");

	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
