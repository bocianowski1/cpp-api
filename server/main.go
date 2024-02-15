package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
)

var CODE string

type CodeResponse struct {
	Code string `json:"code"`
}

func handleCallback(w http.ResponseWriter, r *http.Request) {
	log.Println("Received CALLBACK!!!")
	code := r.URL.Query().Get("code")

	if code == "" {
		http.Error(w, "Missing code parameter", http.StatusBadRequest)
		return
	}

	res := CodeResponse{
		Code: code,
	}

	log.Println("Global code was:", CODE)

	CODE = code

	log.Println("Global code is now:", CODE)

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(res)
}

func handleGetCode(w http.ResponseWriter, r *http.Request) {
	log.Println("Received get code request")
	res := CodeResponse{
		Code: CODE,
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(res)
}

func main() {
	http.HandleFunc("/callback", handleCallback)
	http.HandleFunc("/code", handleGetCode)

	fmt.Println("Starting server at port 3000")
	if err := http.ListenAndServe(":3000", nil); err != nil {
		log.Fatal(err)
	}
}
