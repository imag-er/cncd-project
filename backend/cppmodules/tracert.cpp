#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <chrono>

struct icmphdr
{
    uint8_t type; /* message type */
    uint8_t code; /* type sub-code */
    uint16_t checksum;
    union
    {
        struct
        {
            uint16_t id;
            uint16_t sequence;
        } echo;           /* echo datagram */
        uint32_t gateway; /* gateway address */
        struct
        {
            uint16_t __glibc_reserved;
            uint16_t mtu;
        } frag; /* path mtu discovery */
    } un;
};

#define ICMP_ECHOREPLY 0       /* Echo Reply			*/
#define ICMP_DEST_UNREACH 3    /* Destination Unreachable	*/
#define ICMP_SOURCE_QUENCH 4   /* Source Quench		*/
#define ICMP_REDIRECT 5        /* Redirect (change route)	*/
#define ICMP_ECHO 8            /* Echo Request			*/
#define ICMP_TIME_EXCEEDED 11  /* Time Exceeded		*/
#define ICMP_PARAMETERPROB 12  /* Parameter Problem		*/
#define ICMP_TIMESTAMP 13      /* Timestamp Request		*/
#define ICMP_TIMESTAMPREPLY 14 /* Timestamp Reply		*/
#define ICMP_INFO_REQUEST 15   /* Information Request		*/
#define ICMP_INFO_REPLY 16     /* Information Reply		*/
#define ICMP_ADDRESS 17        /* Address Mask Request		*/
#define ICMP_ADDRESSREPLY 18   /* Address Mask Reply		*/
#define NR_ICMP_TYPES 18

static unsigned first_ttl = 1;
static unsigned max_ttl = 30;
static unsigned wait_time = 3;
static unsigned packet_num = 3;
static unsigned packet_size = 60;
static char *dest_ip = NULL;

static unsigned check_arg(const char *str)
{
    char *ptr = NULL;
    unsigned val = strtoul(str, &ptr, 10);
    if (ptr == str || *ptr != '\0')
    {
        fprintf(stderr, "%s is a bad arg", str);
        exit(1);
    }
    return val;
}

static void parse_args(int argc, char *argv[])
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
                        "traceroute [ -f first_ttl ] [ -m max_ttl ] [ -w wait ] [ -q queries ] host [ packetlen ]\n");
        exit(1);
    }
    if (optind == argc - 2)
    {
        packet_size = check_arg(argv[optind + 1]);
    }
    dest_ip = argv[optind];
}

// 计算校验和
static unsigned short check_sum(void *b, int len)
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

int main(int argc, char *argv[])
{
    // 处理命令行参数
    parse_args(argc, argv);

    // 将ip或域名转化为网络字节序二进制值
    in_addr_t ip = inet_addr(dest_ip);
    if ((int)ip == -1)
    {
        hostent *host = gethostbyname(dest_ip);
        if (host == NULL)
        {
            perror("gethostbyname");
            return 1;
        }
        ip = *(unsigned *)host->h_addr_list[0];
    }

    // 创建协议类型为ICMP的原始套接字
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1)
    {
        perror("socket");
        return 1;
    }

    // 设置目的地址
    sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(0);
    dest.sin_addr.s_addr = ip;

    // 创建ICMP包
    char *packet = (char *)malloc(packet_size);
    icmphdr *icmp = (icmphdr *)packet;

    // 填写ICMP包数据
    icmp->type = 8; // 回显请求
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->un.echo.sequence = 0;

    uint16_t pid = getpid(); // 进程ID作为标识符
    icmp->un.echo.id = pid;

    uint16_t seq = 0;
    char *recv_buff = (char *)malloc(1024);

    timeval timeout;
    timeout.tv_sec = wait_time; // 设置最大等待时间
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    char recv_buffer[1024];
    sockaddr_in recv_addr;
    socklen_t len = sizeof(recv_addr);

    for (int ttl = first_ttl; ttl <= max_ttl; ttl++)
    {
        printf("%d", ttl);
            fflush(stdout);

        // 将TTL字段设置为ttl
        setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        int *result = (int *)malloc(packet_num);
        in_addr cur_addr;
        bool ok = false;
        for (int i = 0; i < packet_num; i++)
        {
            // 记录开始时间
            auto start = std::chrono::high_resolution_clock::now();

            icmp->un.echo.sequence = seq++;
            icmp->checksum = 0;
            icmp->checksum = check_sum(packet, sizeof(icmphdr));

            int send_result = sendto(sock, packet, sizeof(icmphdr), 0, (struct sockaddr *)&dest, sizeof(dest));
            if (send_result == -1)
            {
                perror("sendto");
                return 1;
            }

            int recv_result = recvfrom(sock, recv_buff, 1024, 0, (sockaddr *)&recv_addr, &len);
            if (recv_result == -1)
            {
                result[i] = -1;
                continue;
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            result[i] = duration;

            ok = true;
            cur_addr = recv_addr.sin_addr;
        }

        if (ok)
        {
            printf("  %s", inet_ntoa(cur_addr));
        }
        for (int i = 0; i < packet_num; i++)
        {
            if (result[i] == -1)
            {
                printf("  *");
            }
            else
            {
                printf("  %dms", result[i]);
            }
        }
        fflush(stdout);

        printf("\n");
            fflush(stdout);


        if (cur_addr.s_addr == ip)
        {
            break;
        }
    }

    close(sock);

    return 0;
}