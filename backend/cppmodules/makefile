CC_ARGS = -Wall -std=c++17


ALL: traceroute dnsquery


traceroute: tracert.cpp
	g++ -o $@ $^ $(CC_ARGS)

dnsquery: dns_query.cpp
	g++ -o $@ $^ $(CC_ARGS)

@PHONY: clean
clean:
	rm -f traceroute dnsquery