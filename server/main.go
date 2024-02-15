package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
)

func handleGet(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintf(w, "This is a GET request. Welcome!")
}

func handlePost(w http.ResponseWriter, r *http.Request) {
	body, err := json.Marshal(r.Body)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	defer r.Body.Close()

	fmt.Fprintf(w, "This is a POST request. Received: %s", string(body))
}

func main() {
	http.HandleFunc("/get", handleGet)
	http.HandleFunc("/post", handlePost)

	fmt.Println("Starting server at port 8080")
	if err := http.ListenAndServe(":8080", nil); err != nil {
		log.Fatal(err)
	}
}
