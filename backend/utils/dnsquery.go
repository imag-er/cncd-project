package utils
import (
	"os/exec"
	"log"
	"encoding/json"
	"bufio"
)


func DNSQuery(target string, params DNSQueryRequest) (responseChan chan []byte, errChan chan error) {
	outputChan, errChan := dnsquery(target, params)
	responseChan = make(chan []byte)

	go func() {
		for result := range outputChan {
			log.Println("On command output: ", result)

			jsonContent, err := json.Marshal(result)
			if err != nil {
				log.Println(err)
				continue
			}
			responseChan <- jsonContent
		}
	}()

	return responseChan, errChan
}



func dnsquery(target string,params DNSQueryRequest) (outputChan chan string,errChan chan error) {
	outputChan = make(chan string)
	errChan = make(chan error, 1)
	go func() {
		defer close(outputChan)
		defer close(errChan)

		// path := filepath.Join(getCurrentFileDirname(), "traceroute")
		cmd := exec.Command("cppmodules/dnsquery")

		log.Println(cmd.String())

		stdinPipe, _ := cmd.StdinPipe()
		stdoutPipe, err := cmd.StdoutPipe()

		if err != nil {
			errChan <- err
			return
		}

		if err := cmd.Start(); err != nil {
			errChan <- err
			return
		}

		stdinPipe.Write([]byte(target + "\n"))

		scanner := bufio.NewScanner(stdoutPipe)

		for scanner.Scan() {
			line := scanner.Text()
			outputChan <- line
		}

		if err := scanner.Err(); err != nil {
			errChan <- err
			return
		}

		if err := cmd.Wait(); err != nil {
			errChan <- err
			return
		}
	}()

	return outputChan, errChan
}
