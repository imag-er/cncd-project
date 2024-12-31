package main

import (
	"log"
	"github.com/gin-gonic/gin"
	"cncd-backend/handler"
)

func main() {
	log.Println("Started")

	r := gin.Default()
	r.GET("/ws", handler.WSHandler)
	r.GET("/ping", handler.PingHandler)

	err := r.Run("localhost:8000")
	if err != nil {
		log.Fatalf("Failed to run server: %v", err)
	}

	log.Println("Exited")
}
