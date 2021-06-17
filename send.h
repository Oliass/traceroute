#ifndef send_h
#define send_h


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>


int send_packets (int socket, struct sockaddr_in dest, int ttl, pid_t pid, int number_of_packets);
uint16_t compute_icmp_checksum (const void* buff, int length);


#endif
