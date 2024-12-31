package utils

import (
	"fmt"
	"net"
	"sync"
	"time"
)

// PortScanner 用于扫描TCP端口
type portScanner struct {
	Host    string
	Results chan int
	Params  PortScanRequest
}

// NewPortScanner 创建一个新的PortScanner实例
func newPortScanner(host string,params PortScanRequest) *portScanner {
	return &portScanner{
		Host:    host,
		Results: make(chan int),
		Params:  params,
	}
}

// Scan 扫描指定范围内的端口
func (ps *portScanner) scan() {
	var wg sync.WaitGroup
	for port := ps.Params.Start; port <= ps.Params.End; port++ {
		wg.Add(1)
		go func(p int) {
			defer wg.Done()
			if ps.isPortOpen(p) {
				ps.Results <- p
			}
		}(port)
	}
	wg.Wait()
	close(ps.Results)
}

// isPortOpen 检查端口是否开放
func (ps *portScanner) isPortOpen(port int) bool {
	address := fmt.Sprintf("%s:%d", ps.Host, port)
	conn, err := net.DialTimeout("tcp", address, time.Duration(ps.Params.Timeout)*time.Second)
	if err != nil {
		return false
	}
	defer conn.Close()
	return true
}

func Portscan(target string,params PortScanRequest) chan int {
	ps := newPortScanner(target,params)
	go ps.scan()
	return ps.Results
}


