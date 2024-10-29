/* //////////////////////////////////////////////////////////////////////////////////////
 * imports
 */
package main

import (
	"fmt"
	"os"
	"runtime"
	"strconv"
	"sync"
	"time"
)

/* //////////////////////////////////////////////////////////////////////////////////////
 * globals
 */
const COUNT = 10000000 * 5

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementaiton
 */
var wg sync.WaitGroup

func switchtask(count int) {
	defer wg.Done()
	var i = 0
	for {

		i++
		if i > count {
			break
		}

		runtime.Gosched()
	}
}

/* //////////////////////////////////////////////////////////////////////////////////////
 * main
 */
func main() {

	// single cpu
	runtime.GOMAXPROCS(1)

	// get coroutine count
	cocount := 1000
	if len(os.Args) > 1 {
		cocount, _ = strconv.Atoi(os.Args[1])
	}

	// init duration
	var duration = time.Now().UnixNano()

	// create coroutine task
	var i = 0
	for {

		i++
		if i > cocount-1 {
			break
		}
		wg.Add(1)
		go switchtask(COUNT / cocount)
	}

	// in main goroutine
	switchtask(COUNT / cocount)
	wg.Wait()
	// computing time
	duration = (time.Now().UnixNano() - duration) / 1000000

	// trace
	fmt.Printf("switch[%d]: go: %d switches in %d ms, %d switches per second\n", cocount, COUNT, duration, (1000*COUNT)/duration)
}
