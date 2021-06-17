#include "receive.h"

bool receive_all(int socket, int ttl, pid_t pid, struct in_addr addr[PACKET_COUNT], float *time, int *packets_received)
{
    bool last_packet_received = false;
    struct timeval time1, time2;
    gettimeofday(&time1, NULL);

    int received_status;
    float response_time = 0.0;

    while ((*packets_received < PACKET_COUNT) && (response_time < TIMEOUT_MS))
    {
        received_status = receive(socket, ttl, pid, TIMEOUT_MS - response_time, &addr[*packets_received]);
        gettimeofday(&time2, NULL);
        response_time = (time2.tv_sec - time1.tv_sec) * 1000.0 + (time2.tv_usec - time1.tv_usec) / 1000.0;

        if (received_status != DISCARD)
        {
            (*packets_received)++;
            *time += response_time;

            if (received_status == LAST)
                last_packet_received = true;
        }
    }

    if (last_packet_received)
        return true;
    else
        return false;
}

int receive(int socket, int ttl, pid_t pid, float wait, struct in_addr *addr)
{
    fd_set descriptors;
    FD_ZERO(&descriptors);
    FD_SET(socket, &descriptors);

    struct timeval time;
    time.tv_sec = 0;
    time.tv_usec = wait;

    int ready = select(socket + 1, &descriptors, NULL, NULL, &time);

    if (ready < 0)
    {
        fprintf(stderr, "Select error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else if (ready == 0)
        return DISCARD;

    struct sockaddr_in sender;
    socklen_t sender_size = sizeof(sender);
    uint8_t buffer[IP_MAXPACKET];
    if (recvfrom(socket, buffer, IP_MAXPACKET, MSG_DONTWAIT,
							 (struct sockaddr *)&sender, &sender_size) < 0)
    {
        fprintf(stderr, "Error while receiving a packet: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct ip *ip_header = (struct ip *)buffer;
    struct icmp *icmp_packet = (struct icmp *)((uint8_t *)ip_header + (*ip_header).ip_hl * 4);

    if (icmp_packet->icmp_type == ICMP_TIME_EXCEEDED)
    {
        struct ip *temp_ip = (struct ip *)((uint8_t *)icmp_packet + 8);
        struct icmp *temp_icmp = (struct icmp *)((uint8_t *)temp_ip + (*temp_ip).ip_hl * 4);

        if ((temp_icmp->icmp_id == pid) && (temp_icmp->icmp_seq == ttl))
        {
            *addr = sender.sin_addr;
            return NODE;
        }
    }

    else if ((icmp_packet->icmp_id == pid) && (icmp_packet->icmp_seq == ttl) && (icmp_packet->icmp_type == ICMP_ECHOREPLY))
    {
        *addr = sender.sin_addr;
        return LAST;
    }

    return DISCARD;
}
