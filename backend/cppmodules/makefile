CC_ARGS = -Wall -std=c++17


ALL: traceroute dnsquery

traceroute: tr.cpp
	g++ -o $@ $^ $(CC_ARGS)

dnsquery: dns.cpp
	g++ -o $@ $^ $(CC_ARGS)

@PHONY: clean
clean:
	rm -f traceroute dnsquery