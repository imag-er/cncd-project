package handler
import (
	"log"
	"github.com/gorilla/websocket"
	"cncd-backend/utils"
	"encoding/json"
)

func handlePortScan(conn *websocket.Conn, target string, request utils.PortScanRequest) {
	log.Println("parameters: ", request)

	ports := utils.Portscan(target,request)
	for port := range ports {
		portData := map[string]int{"port": port}
		jsonData, err := json.Marshal(portData)
		log.Println("Port: ", port)
		if err != nil {
			log.Println(err)
			return
		}
		if err := conn.WriteMessage(websocket.TextMessage, []byte(string(jsonData))); err != nil {
			log.Println(err)
			return
		}
	}
}