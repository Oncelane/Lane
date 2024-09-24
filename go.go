package main

import "net/http"

func main() {
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte(`<html><head><title>404 Not Found
                </title></head><body><center><h1>404 Not Found</h1></center>
                <hr><center> golg </center></body></html>`))
	})
	http.ListenAndServe(":8020", nil)
}
