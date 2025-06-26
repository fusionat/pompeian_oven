package main

import (
	"context"
	"database/sql"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	_ "github.com/mattn/go-sqlite3"
)

func main() {
	sigChan := make(chan os.Signal, 2)
	signal.Notify(sigChan, os.Interrupt, syscall.SIGTERM)
	db, err := sql.Open("sqlite3", "./data.db")
	if err != nil {
		log.Fatalf("Failed to open database: %v", err)
	}

	defer func() {
		db.Close()
		fmt.Println("Connection has been closed!")
	}()

	if err = InitDB(db); err != nil {
		log.Fatalf("Failed to create table: %v", err)
	}

	app := &App{DB: db}
	server := &http.Server{
		Addr:    ":9090",
		Handler: nil,
	}

	http.HandleFunc("/get", app.get)
	http.HandleFunc("/add", app.post)
	go getThermo(sigChan, server)

	if err := server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
		log.Fatalf("ListenAndServe error: %v", err)
	}

}

func getThermo(sigChan chan os.Signal, server *http.Server) {
	period := time.NewTicker(1 * time.Second)
	for {
		select {
		case <-period.C:
			fmt.Println("TICK!")
		case <-sigChan:
			fmt.Println("Graceful shutdown.")
			ctx, _ := context.WithTimeout(context.Background(), 5*time.Second)
			server.Shutdown(ctx)
			return
		}
	}
}
