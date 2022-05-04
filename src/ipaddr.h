#include <arpa/inet.h>
#include <netdb.h>

// *is_ipv6 = sa->sa_family == AF_INET6 ? 1 : 0
// return &(((struct sockaddr_in*)sa)->sin_addr);
// return &(((struct sockaddr_in6*)sa)->sin6_addr);
void *sockaddr_to_inaddr(struct sockaddr *sa, int *is_ipv6);

// *is_ipv6 = ai->ai_family == AF_INET6 ? 1 : 0;
// return &(((struct sockaddr_in *)ai->ai_addr)->sin_addr));
// return &(((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr));
void *addrinfo_to_inaddr(struct addrinfo *ai, int *is_ipv6);

// caller required to free struct via "freeaddrinfo(list)"
// if *err is not-null, will be populated with status for gai_strerror
// if getaddrinfo fails, otherwise not not assigned
struct addrinfo *get_addrinfo_list(const char *host, int *err);
