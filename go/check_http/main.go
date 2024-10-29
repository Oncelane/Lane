// package main

// import (
// 	"fmt"
// 	"io"
// 	"net/http"
// )

// func main() {
// 	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
// 		// 打印请求的方法、URL和协议版本
// 		fmt.Printf("Received request:\nMethod: %s\nURL: %s\nProto: %s\n", r.Method, r.URL, r.Proto)

// 		// 读取请求体
// 		body, err := io.ReadAll(r.Body)
// 		if err != nil {
// 			http.Error(w, "Error reading request body", http.StatusInternalServerError)
// 			return
// 		}
// 		defer r.Body.Close()

// 		// 打印请求体
// 		fmt.Printf("Body: %s\n", body)

// 		// 发送响应
// 		fmt.Fprintf(w, "Request received and printed!")
// 	})

//		fmt.Println("Server is listening on :8020")
//		if err := http.ListenAndServe(":8020", nil); err != nil {
//			fmt.Printf("Error starting server: %s\n", err)
//		}
//	}
package main

import (
	"fmt"
	"log"
	"net/http"
	"runtime"
)

func main() {
	// 限制为1个CPU
	runtime.GOMAXPROCS(1)

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprint(w, "Hello, world.")
	})

	log.Fatal(http.ListenAndServe(":8020", nil))
}
