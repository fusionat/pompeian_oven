package main

import "database/sql"

func InitDB(db *sql.DB) error {
	_, err := db.Exec(`CREATE TABLE IF NOT EXISTS records (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        c TEXT,
        timestamp INTEGER
    )`)
	return err
}
