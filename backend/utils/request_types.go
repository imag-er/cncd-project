package utils

type WebSocketRequest struct {
	Type string `json:"type"`
	Target string `json:"target"`
	Traceroute TraceRouteRequest `json:"traceroute"`
	Portscan PortScanRequest `json:"portscan"`
	DNSQuery DNSQueryRequest `json:"dnsquery"`
}



type TraceRouteRequest struct {
	First_ttl int `json:"first_ttl"`
	Max_ttl int `json:"max_ttl"`
	WaitTime int `json:"wait_time"`
	MaxTimeout int `json:"max_timeout"`
	Nqueries int `json:"nqueries"`

	PacketSize int `json:"packet_size"`
}

type PortScanRequest struct {
	Start int `json:"start"`
	End int `json:"end"`
	Timeout uint `json:"timeout"`
}

type DNSQueryRequest struct {

}



type TraceRouteResult struct {
	Hop     int    `json:"hop"`
	Address string `json:"address"`
	Timeout []int  `json:"timeout"`
}


type ActiveScanRequest struct {
	mask string // CIDR或者子网掩码
}