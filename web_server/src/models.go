package main

import (
	"database/sql"
	"time"
)

type Thermo struct {
	Id        int       `json:"Id"`
	C         float32   `json:"c"`
	Timestamp time.Time `json:"timestamp"`
}

type App struct {
	DB *sql.DB
}
