package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
)

func (a *App) get(rw http.ResponseWriter, req *http.Request) {
	if req.Method != http.MethodGet {
		http.Error(rw, "OOOOPS", http.StatusMethodNotAllowed)
		return
	}

	rows, err := a.DB.Query("SELECT id, c, timestamp FROM records ORDER BY timestamp DESC")
	if err != nil {
		http.Error(rw, "DB Query field", http.StatusInternalServerError)
		log.Fatalf("DB Query field %v", rw)
	}
	defer rows.Close()

	var results []Thermo
	for rows.Next() {
		var thermo Thermo
		if err := rows.Scan(&thermo.Id, &thermo.C, &thermo.Timestamp); err != nil {
			http.Error(rw, "Row scan failed", http.StatusInternalServerError)
			log.Fatal(err)
			return
		}
		results = append(results, thermo)
	}

	rw.Header().Set("Content-Type", "application/json")
	json.NewEncoder(rw).Encode(results)
}

func (a *App) post(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method Not Allowed", http.StatusMethodNotAllowed)
		return
	}

	var t Thermo
	err := json.NewDecoder(r.Body).Decode(&t)
	if err != nil {
		http.Error(w, "Bad Request", http.StatusBadRequest)
		return
	}

	// вставка строки
	_, err = a.DB.Exec("INSERT INTO records (c, timestamp) VALUES (?, ?)", fmt.Sprintf("%.2f", t.C), t.Timestamp)
	if err != nil {
		http.Error(w, "DB insert failed", http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusCreated)
	fmt.Fprintf(w, "Inserted!")
}
