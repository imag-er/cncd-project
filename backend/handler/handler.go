package handler

import (
	"cncd-backend/utils"
	"encoding/json"
	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
	"log"
	"net/http"
)

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

func WSHandler(c *gin.Context) {
	conn, err := upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		log.Println(err)
		return
	}
	defer conn.Close()
	defer log.Println("Connection closed")

	log.Println("Connection established")

	_, message, err := conn.ReadMessage()
	if err != nil {
		log.Println(err)
		return
	}

	
	// var request map[string]interface{}
	var request utils.WebSocketRequest
	if err := json.Unmarshal(message, &request); err != nil {
		log.Println(err)
		return
	}
	
	log.Println("message_type: ", request.Type)
	log.Println("target: ", request.Target)

	switch request.Type {
	case "dnsquery":
		handleDNSQuery(conn, request.Target, request.DNSQuery)
	case "portscan":
		handlePortScan(conn, request.Target, request.Portscan)
	case "traceroute":
		handleTraceroute(conn, request.Target, request.Traceroute)
	default:
		log.Println("Unknown request type")
	}

}







