package utils

import (
	// "encoding/json"
	"log"
	"testing"
)

const (
	BAIDU_IP = "39.156.66.10"
	BILI_IP  = "8.134.50.24"
	TEST_IP  = "153.99.227.184"
	FTP_IP   = "10.199.227.254"
)

// func TestTraceroute(t *testing.T) {
// 	outputChan, errChan := traceroute(IP1)

// 	// 读取输出
// 	for output := range outputChan {
// 		log.Println("Output:", output)
// 	}

// 	// 检查是否有错误发生
// 	if err := <-errChan; err != nil {
// 		log.Println("Error:", err)
// 	}

// }

func TestTraceroute(t *testing.T) {
	response,_ := Traceroute(BILI_IP, TraceRouteRequest{})
	for result := range response {
		log.Println(result)
	}

}
