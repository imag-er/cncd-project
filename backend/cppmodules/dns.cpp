#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>  // For memset
#include <unistd.h> // For close
#include <chrono>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <sstream>

struct DNS_HEADER
{
    unsigned short id;
    unsigned char rd : 1;
    unsigned char tc : 1;
    unsigned char aa : 1;
    unsigned char opcode : 4;
    unsigned char qr : 1;
    unsigned char rcode : 4;
    unsigned char cd : 1;
    unsigned char ad : 1;
    unsigned char z : 1;
    unsigned char ra : 1;
    unsigned short q_count;
    unsigned short ans_count;
    unsigned short auth_count;
    unsigned short add_count;
};

struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
};

struct RES_RECORD
{
    std::string name;
    std::string type;
    std::string data;
    unsigned int ttl;
};
std::map<std::string, std::string> dns_cache;
void ChangeFormat(unsigned char *dns, const std::string &host)
{
    int lock = 0;
    for (size_t i = 0; i <= host.length(); i++)
    {
        if (i == host.length() || host[i] == '.')
        {
            *dns++ = i - lock;
            for (size_t j = lock; j < i; j++)
            {
                *dns++ = host[j];
            }
            lock = i + 1;
        }
    }
    *dns++ = 0;
}

unsigned char *decompress_name(unsigned char *reader, unsigned char *buffer, std::string &out_name)
{
    unsigned char *original_reader = reader;
    bool jumped = false;
    int len;

    while (*reader)
    {
        if ((*reader & 0xC0) == 0xC0)
        {
            if (!jumped)
                original_reader = reader + 2;
            jumped = true;

            int offset = ntohs(*reinterpret_cast<unsigned short *>(reader)) & 0x3FFF;
            reader = buffer + offset;
            if (reader >= buffer + 65536)
            {
                std::cerr << "Pointer越界" << std::endl;
                return nullptr;
            }
        }
        else
        {
            len = *reader++;
            if (reader + len > buffer + 65536)
            {
                std::cerr << "Name segment越界" << std::endl;
                return nullptr;
            }
            out_name.append(reinterpret_cast<char *>(reader), len);
            out_name.append(".");
            reader += len;
        }
    }
    if (!jumped)
    {
        reader++;
    }
    else
    {
        reader = original_reader;
    }
    return reader;
}
// DNS解析
std::vector<RES_RECORD> parse_dns(unsigned char *buffer, int query_count, int answer_count, int auth_count, int add_count, std::ostringstream &json_response)
{
    std::vector<RES_RECORD> records;
    unsigned char *reader = buffer + sizeof(DNS_HEADER);

    // 解析QUESTION部分
    for (int i = 0; i < query_count; ++i)
    {
        std::string name;
        reader = decompress_name(reader, buffer, name);
        reader += sizeof(QUESTION);
        json_response << "\"response\": \"" << name << "\", \"record-type\": \"A";
    }

    // 解析 ANSWER AUTHORITY ADDITIONAL 部分
    for (int i = 0; i < answer_count + auth_count + add_count; ++i)
    {
        RES_RECORD record;
        reader = decompress_name(reader, buffer, record.name);
        unsigned short type = ntohs(*reinterpret_cast<unsigned short *>(reader));
        reader += 2;
        reader += 2;
        unsigned int ttl = ntohl(*reinterpret_cast<unsigned int *>(reader));
        record.ttl = ttl;
        reader += 4;
        unsigned short data_len = ntohs(*reinterpret_cast<unsigned short *>(reader));
        reader += 2;

        if (type == 1)
        { // A记录
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, reader, ip, INET_ADDRSTRLEN);
            record.data = ip;
            record.type = "A";
        }
        else if (type == 28)
        { // AAAA记录
            char ipv6[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, reader, ipv6, INET6_ADDRSTRLEN);
            record.data = ipv6;
            record.type = "AAAA";
        }
        else if (type == 2)
        { // NS记录
            decompress_name(reader, buffer, record.data);
            record.type = "NS";
        }
        else if (type == 5)
        { // CNAME记录
            decompress_name(reader, buffer, record.data);
            record.type = "CNAME";
        }
        reader += data_len;
        records.push_back(record);
    }
    return records;
}

bool query_dns(const std::string &server, const std::string &domain, std::vector<RES_RECORD> &responses, std::ostringstream &json_response)
{
    int sockfd;
    struct sockaddr_in dest;
    unsigned char buf[65536];
    socklen_t addr_len = sizeof(dest);
    int receivedBytes;

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        std::cerr << "Socket创建失败\n";
        return false;
    }

    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    inet_pton(AF_INET, server.c_str(), &dest.sin_addr);

    DNS_HEADER *dns = reinterpret_cast<DNS_HEADER *>(buf);
    dns->id = htons(getpid());
    dns->qr = 0;
    dns->opcode = 0;
    dns->aa = 0;
    dns->tc = 0;
    dns->rd = 1;
    dns->ra = 0;
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->q_count = htons(1);
    dns->ans_count = 0;
    dns->auth_count = 0;
    dns->add_count = 0;

    unsigned char *qname = buf + sizeof(DNS_HEADER);
    ChangeFormat(qname, domain);

    QUESTION *qinfo = reinterpret_cast<QUESTION *>(qname + strlen(reinterpret_cast<const char *>(qname)) + 1);
    qinfo->qtype = htons(1);
    qinfo->qclass = htons(1);
    if (sendto(sockfd, reinterpret_cast<char *>(buf), sizeof(DNS_HEADER) + strlen(reinterpret_cast<const char *>(qname)) + 1 + sizeof(QUESTION), 0, (struct sockaddr *)&dest, addr_len) < 0)
    {
        perror("发送失败");
        close(sockfd);
        return false;
    }

    receivedBytes = recvfrom(sockfd, reinterpret_cast<char *>(buf), sizeof(buf), 0, (struct sockaddr *)&dest, &addr_len);
    if (receivedBytes < 0)
    {
        std::cerr << "服务器无响应: " << server << std::endl;
        close(sockfd);
        return false;
    }
    responses = parse_dns(buf, ntohs(dns->q_count), ntohs(dns->ans_count), ntohs(dns->auth_count), ntohs(dns->add_count), json_response);

    close(sockfd);
    return true;
}
void recursive_query(const std::string &domain, std::set<std::string> &visited, std::ostringstream &json_output, int depth_limit = 30)
{
    if (depth_limit <= 0)
    {
        json_output << "{\"达到最大递归深度\": \"" << domain << "\"}" << std::endl;
        return;
    }

    if (visited.find(domain) != visited.end())
    {
        json_output << "{\"域名已查询\": \"" << domain << "\"}" << std::endl;
        return;
    }

    if (dns_cache.find(domain) != dns_cache.end())
    {
        json_output << "{\"缓存结果\": {\"域名\": \"" << domain << "\", \"ip\": \"" << dns_cache[domain] << "\"}}" << std::endl;
        return;
    }

    visited.insert(domain);
    std::string current_server = "198.41.0.4"; // 根服务器

    while (true)
    {
        std::ostringstream json_response;
        json_response << "{\"type\":\"query" << "\"," << "\"ip\": \"" << current_server << "\",";

        std::vector<RES_RECORD> responses;
        if (!query_dns(current_server, domain, responses, json_response))
        {
            json_output << "{\"查询失败\": \"" << current_server << "\"}" << std::endl;
            return;
        }

        json_response << "\"}";
        json_output << json_response.str() << std::endl;

        std::string next_server;
        bool resolved = false;
        std::vector<std::string> ipv4_results;
        std::map<std::string, std::string> authority_to_ip;

        for (auto &record : responses)
        {
            if (record.type == "A" && record.name == domain + ".")
            {
                ipv4_results.push_back(record.data);
                resolved = true;
            }
            else if (record.type == "CNAME")
            {
                if (record.data.back() == '.')
                {
                    record.data.pop_back(); // 移除点
                }
                recursive_query(record.data, visited, json_output, depth_limit - 1);
                return;
            }
            else if (record.type == "NS")
            {
                authority_to_ip[record.data] = "";
            }
            else if (record.type == "A")
            {
                authority_to_ip[record.name] = record.data;
            }
        }

        if (resolved)
        {
            // 输出IPv4结果
            json_output << "{\"type\": \"result" << "\"," << "\"results\": [";
            for (size_t i = 0; i < ipv4_results.size(); ++i)
            {
                if (i > 0)
                    json_output << ',';
                json_output << "{\"ip\": \"" << ipv4_results[i] << "\", \"TTL\": \"" << responses[i].ttl << "\"}";
            }
            json_output << "]"
                           "}";

            // 缓存第一个结果
            if (!ipv4_results.empty())
            {
                dns_cache[domain] = ipv4_results[0];
            }

            return;
        }

        for (const auto &ns_pair : authority_to_ip)
        {
            if (!ns_pair.second.empty())
            {
                next_server = ns_pair.second;
                break;
            }
        }

        if (next_server.empty())
        {
            for (const auto &ns_pair : authority_to_ip)
            {
                if (ns_pair.second.empty())
                {
                    recursive_query(ns_pair.first, visited, json_output, depth_limit - 1);
                    return;
                }
            }
        }
        current_server = next_server;
    }
}

int main()
{
    std::string domain;
    std::cin >> domain; // 输入域名
    std::ostringstream json_output;
    std::set<std::string> visited;
    recursive_query(domain, visited, json_output); // DNS查询
    std::cout << json_output.str() << std::endl;

    return 0;
}
