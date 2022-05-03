#include <assert.h>
#include <arpa/inet.h>
#include <string.h>

int roundtrip_v4(const char *ip4_str)
{
	struct sockaddr_in sa_0;
	memset(&sa_0, 0x00, sizeof(struct sockaddr_in));
	int rv = inet_pton(AF_INET, ip4_str, &(sa_0.sin_addr));
	if (rv <= 0) {
		return 1;
	}

	struct sockaddr_in sa_1;
	memset(&sa_1, 0x00, sizeof(struct sockaddr_in));
	char ip4[INET_ADDRSTRLEN];
	memset(ip4, 0x00, INET_ADDRSTRLEN);
	const char *v =
	    inet_ntop(AF_INET, &(sa_1.sin_addr), ip4, INET_ADDRSTRLEN);
	return strcmp(v, ip4_str) == 0 ? 0 : 1;
}

int roundtrip_v6(const char *ip6_str)
{
	struct sockaddr_in6 sa_0;
	memset(&sa_0, 0x00, sizeof(struct sockaddr_in));
	int rv = inet_pton(AF_INET6, ip6_str, &(sa_0.sin6_addr));
	if (rv <= 0) {
		return 1;
	}

	struct sockaddr_in6 sa_1;
	memset(&sa_1, 0x00, sizeof(struct sockaddr_in));
	char ip6[INET6_ADDRSTRLEN];
	memset(ip6, 0x00, INET6_ADDRSTRLEN);
	const char *v =
	    inet_ntop(AF_INET6, &(sa_1.sin6_addr), ip6, INET6_ADDRSTRLEN);
	return strcmp(v, ip6_str) == 0 ? 0 : 1;
}

int main(void)
{
	roundtrip_v4("172.30.0.195");

	roundtrip_v6("2001:db8:63b3:1::3490");

	return 0;
}
