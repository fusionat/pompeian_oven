package main

import (
	"database/sql"
	"fmt"
	"log"
	"net/http"

	_ "github.com/mattn/go-sqlite3"
)

func main() {
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
	http.HandleFunc("/get", app.get)
	http.HandleFunc("/add", app.post)

	fmt.Println("http://localhost:8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
