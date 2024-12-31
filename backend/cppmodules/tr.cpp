#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <chrono>
#include <iostream>

struct icmphdr
{
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    union
    {
        struct
        {
            uint16_t id;
            uint16_t sequence;
        } echo;
        uint32_t gateway;
        struct
        {
            uint16_t __glibc_reserved;
            uint16_t mtu;
        } frag;
    } un;
};

#define ICMP_ECHOREPLY 0
#define ICMP_DEST_UNREACH 3
#define ICMP_SOURCE_QUENCH 4
#define ICMP_REDIRECT 5
#define ICMP_ECHO 8
#define ICMP_TIME_EXCEEDED 11
#define ICMP_PARAMETERPROB 12
#define ICMP_TIMESTAMP 13
#define ICMP_TIMESTAMPREPLY 14
#define ICMP_INFO_REQUEST 15
#define ICMP_INFO_REPLY 16
#define ICMP_ADDRESS 17
#define ICMP_ADDRESSREPLY 18
#define NR_ICMP_TYPES 18

// 计算校验和
static unsigned short checksum(void *b, int len)
{
    unsigned short *buf = (unsigned short *)b;
    unsigned sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
    {
        sum += *buf++;
    }
    if (len == 1)
    {
        sum += *(unsigned char *)buf;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

static unsigned check_arg(const char *str)
{
    char *ptr;
    unsigned val = strtoul(str, &ptr, 10);
    if (ptr == str || *ptr != '\0')
    {
        fprintf(stderr, "%s is a bad arg", str);
        exit(1);
    }
    return val;
}

static unsigned first_ttl = 1;
static unsigned max_ttl = 30;
static unsigned wait_time = 3;
static unsigned packet_num = 3;
static unsigned packet_size = 60;

static const char *parse_args(int argc, char *argv[])
{
    const struct option table[] = {
        {"first", required_argument, NULL, 'f'},
        {"max-hop", required_argument, NULL, 'm'},
        {"wait", required_argument, NULL, 'w'},
        {"queries", required_argument, NULL, 'q'},
        {0, 0, NULL, 0}};
    int opt;
    while ((opt = getopt_long(argc, argv, "f:m:w:q:", table, NULL)) != -1)
    {
        switch (opt)
        {
        case 'f':
            first_ttl = check_arg(optarg);
            break;
        case 'm':
            max_ttl = check_arg(optarg);
            break;
        case 'w':
            wait_time = check_arg(optarg);
            break;
        case 'q':
            packet_num = check_arg(optarg);
            break;
        default:
            exit(1);
        }
    }
    if (optind == argc || optind < argc - 2)
    {
        fprintf(stderr, "Usage:\n"
                        "traceroute [ -f first_ttl ] [ -m max_ttl ] [ -w MAX,HERE,NEAR ] [ -q nqueries ] host [ packetlen ]\n");
        exit(1);
    }
    if (optind == argc - 2)
    {
        packet_size = check_arg(argv[optind + 1]);
    }
    return argv[optind];
}

int main(int argc, char *argv[])
{
    const char *target_ip = parse_args(argc, argv);

    // 创建套接字
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    inet_pton(AF_INET, target_ip, &dest.sin_addr);

    char *packet = (char *)malloc(packet_size);
    memset(packet, 0, packet_size);

    struct icmphdr *icmp = (struct icmphdr *)packet;
    icmp->type = ICMP_ECHO; // 报文类型为回显请求
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->un.echo.id = getpid(); // 进程ID作为标识符
    icmp->un.echo.sequence = 1;

    struct timeval timeout;
    timeout.tv_sec = wait_time; // 设置最大等待时间
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    bool reach_dest = false;
    for (int ttl = first_ttl; ttl <= max_ttl && !reach_dest; ttl++)
    {
        std::cout << ttl;
        // 将TTL字段值设置为ttl
        setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        bool received_any = false;
        for (int i = 0; i < packet_num; i++)
        {
            // 记录开始时间
            auto start = std::chrono::high_resolution_clock::now();


            // modified
            // icmp->un.echo.sequence = ttl * packet_num + i;
            icmp->checksum = checksum(packet, sizeof(struct icmphdr));

            // 向目标ip发送数据包
            sendto(sock, packet, sizeof(struct icmphdr), 0, (struct sockaddr *)&dest, sizeof(dest));

            char *recv_buffer = (char *)malloc(packet_size);
            struct sockaddr_in recv_addr;
            socklen_t addr_len = sizeof(recv_addr);

            if (recvfrom(sock, recv_buffer, packet_size, 0, (struct sockaddr *)&recv_addr, &addr_len) >= 0)
            {
                // 计算经过时间
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

                char addr_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &recv_addr.sin_addr, addr_str, INET_ADDRSTRLEN);

                if (!received_any)
                {
                    std::cout << "  " << addr_str;
                    received_any = true;
                }
                std::cout << "  " << duration << "ms";
                if (recv_addr.sin_addr.s_addr == dest.sin_addr.s_addr)
                {
                    reach_dest = true;
                    break;
                }
            }
            else
            {
                std::cout << "  *";
            }
        }

        std::cout << std::endl;
    }

    close(sock);

    return 0;
}