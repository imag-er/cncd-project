#include <iostream>
#include <sys/socket.h>
// #include <netinet/ip_icmp.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <chrono>

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

#define PACKET_SIZE 64
#define MAX_HOPS 30

// 计算校验和
unsigned short checksum(void *b, int len)
{
    unsigned short *buf = (unsigned short *)b;
    unsigned int sum = 0;
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

void traceroute(const char *target_ip)
{
    // 创建协议类型为ICMP的原始套接字
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    inet_pton(AF_INET, target_ip, &dest.sin_addr);

    char packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);

    icmphdr *icmp = (icmphdr *)packet;
    icmp->type = ICMP_ECHO; // 报文类型为回显请求
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->un.echo.id = getpid(); // 进程ID作为标识符
    icmp->un.echo.sequence = 1;

    timeval timeout;
    timeout.tv_sec = 5; // 设置超时为5秒
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    for (int ttl = 1; ttl <= MAX_HOPS; ttl++)
    {
        // 将TTL字段值设置为ttl
        setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        auto start = std::chrono::high_resolution_clock::now();

        // 设置校验和
        icmp->checksum = checksum(packet, sizeof(icmphdr));

        // 向目标ip发送数据包
        sendto(sock, packet, sizeof(icmphdr), 0, (sockaddr *)&dest, sizeof(dest));

        char recv_buffer[PACKET_SIZE];
        sockaddr_in recv_addr;
        socklen_t addr_len = sizeof(recv_addr);

        // 接受返回的报文，若发生错误则输出'*'
        if (recvfrom(sock, recv_buffer, PACKET_SIZE, 0, (sockaddr *)&recv_addr, &addr_len) < 0)
        {
            std::cout << ttl << " *\n";
            std::flush(std::cout);
            continue;
        }

        // 计算经过的时间
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        // 将接受到的二进制ip转化为点分十进制形式并输出
        char addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &recv_addr.sin_addr, addr_str, INET_ADDRSTRLEN);
        std::cout << ttl << " " << addr_str << " " << duration << "ms\n";
        std::flush(std::cout);

        // 若返回的地址等于目标地址，则退出循环
        if (recv_addr.sin_addr.s_addr == dest.sin_addr.s_addr)
        {
            break;
        }
    }

    close(sock);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <target_ip>\n";
        return 1;
    }
    traceroute(argv[1]);
    return 0;
}

/*
#include ...

struct icmphdr
{
  ...
};

#define ...


// 计算校验和
unsigned short checksum(void *b, int len)
{
   ...
    return result;
}

void traceroute(const char *target_ip)
{

    for (int ttl = 1; ttl <= MAX_HOPS; ttl++)
    {
        ...

        if (recvfrom(sock, recv_buffer, PACKET_SIZE, 0, (sockaddr *)&recv_addr, &addr_len) < 0)
        {
            std::cout << ttl << " *\n";
            continue;
        }
        ...
        std::cout << ttl << " " << addr_str << " " << duration << "ms\n";
        if (recv_addr.sin_addr.s_addr == dest.sin_addr.s_addr)
        {
            break;
        }
    }

    close(sock);
}

int main(int argc, char *argv[])
{
    ...
    traceroute(argv[1]);
    return 0;
}

*/
