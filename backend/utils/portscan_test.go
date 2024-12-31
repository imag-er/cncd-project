package utils

import (
	"log"
	"testing"
)

func TestPortscan(t *testing.T) {
	// portscan()
	c := ACKScan("127.0.0.1")
	for i := range c {
		log.Println("port: ", i)
	}

	log.Println("TestPortscan")

}
