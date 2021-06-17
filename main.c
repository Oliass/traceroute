#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>

#include <arpa/inet.h>

#include "send.h"
#include "receive.h"


#define MAX_TTL 30

void print(struct in_addr addr[PACKET_COUNT], float time, int packets_received)
{
    if (packets_received == 0)
        printf("*\n");

    else
    {
        char a1[20], a2[20], a3[20];

        inet_ntop(AF_INET, &addr[0], a1, sizeof(a1));
        inet_ntop(AF_INET, &addr[1], a2, sizeof(a2));
        inet_ntop(AF_INET, &addr[2], a3, sizeof(a3));

        printf("%s", a1);

        if ((packets_received > 1) && strcmp(a1, a2))
            printf(" %s", a2);
        if ((packets_received > 2) && strcmp(a1, a3) && strcmp(a2, a3))
            printf(" %s", a3);

        if (packets_received == 3)
            printf("  %dms\n", (int)round(time / (double)PACKET_COUNT));
        else
            printf("  ???\n");
    }
}

int main(int argc, char *argv[])
{

    if (argc != 2) {
        fprintf(stderr, "Usage: ./traceroute [address]\n");
        return EXIT_FAILURE;
    }

    pid_t pid = getpid();
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;

    if (inet_pton(AF_INET, argv[1], &(dest.sin_addr)) == 0)
    {
        fprintf(stderr, "Error parsing IP address: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if (sockfd < 0)
    {
        fprintf(stderr, "Socket error: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    for (int current_ttl = 1; current_ttl <= MAX_TTL; current_ttl++)
    {
        if (send_packets(sockfd, dest, current_ttl, pid, PACKET_COUNT) < 0)
        {
            fprintf(stderr, "Error while sending packets: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
        struct in_addr addr[PACKET_COUNT];
        int packets_received = 0;
        float time = 0.0;
        bool should_end = receive_all(sockfd, current_ttl, pid, addr, &time, &packets_received);

        print(addr, time, packets_received);

        if (should_end)
            break;
    }

    return EXIT_SUCCESS;
}
