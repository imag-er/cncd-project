package handler
import (
	"log"
	"github.com/gorilla/websocket"
	"cncd-backend/utils"

)
func handleDNSQuery(conn *websocket.Conn, target string, request utils.DNSQueryRequest) {
	log.Println("parameters: ", request)

	params := request

	jsonData, errChan := utils.DNSQuery(target, params)

	defer log.Println("DNSQuery request finished")

	for {
		select {
		case err := <-errChan:
			if err != nil {
				log.Println("Error parsing dns json", err)
				close(jsonData)
				return
			}

		case content := <-jsonData:
			log.Println("On content: ", content)
			if err := conn.WriteMessage(websocket.TextMessage, content); err != nil {
				log.Println(err)
				return
			}

		}
	}

}