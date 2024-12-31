package handler
import (
	"log"
	"github.com/gorilla/websocket"
	"cncd-backend/utils"

)
func handleTraceroute(conn *websocket.Conn, target string, request utils.TraceRouteRequest) {
	log.Println("parameters: ", request)

	params := request
	jsonData, errChan := utils.Traceroute(target, params)

	defer log.Println("Traceroute request finished")

	for {
		select {
		case err := <-errChan:
			if err != nil {
				log.Println("Error generating traceroute JSON:", err)
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