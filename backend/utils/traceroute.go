package utils

import (
	"bufio"
	"encoding/json"
	"errors"
	"fmt"
	"log"
	"os/exec"
	"strconv"
	"strings"
)

// parseLine 解析单行输入的字符串并返回TraceRouteResult
func parseLine(line string) (TraceRouteResult, error) {
	var result TraceRouteResult
	var err error

	// Split the line by whitespace
	parts := strings.Fields(line)

	// Check if the line has enough parts
	if len(parts) < 2 {
		return result, errors.New("invalid line format")
	}

	// Parse the hop number
	result.Hop, err = strconv.Atoi(parts[0])
	if err != nil {
		return result, fmt.Errorf("invalid hop number: %v", err)
	}

	// Check for timeout indicated by '*'
	if parts[1] == "*" {
		result.Address = ""
		return result, nil
	}

	// Parse the address
	result.Address = parts[1]

	// Parse the timeout values
	result.Timeout = make([]int, 0, len(parts)-2)
	for _, timeoutStr := range parts[2:] {
		timeout, err := strconv.Atoi(strings.TrimSuffix(timeoutStr, "ms"))
		if err != nil {
			return result, fmt.Errorf("invalid timeout value: %v", err)
		}
		result.Timeout = append(result.Timeout, timeout)
	}

	return result, nil
}


// GenerateTracerouteJSON 生成Traceroute的JSON格式
func Traceroute(target string, params TraceRouteRequest) (responseChan chan []byte, errChan chan error) {
	outputChan, errChan := traceroute(target, params)
	responseChan = make(chan []byte)

	go func() {
		log.Println("GeneratingTracerouteJSON")
		for result := range outputChan {
			log.Println("On command output: ", result)
			content, err := parseLine(result)
			if err != nil {
				log.Println(err)
				continue
			}

			jsonContent, err := json.Marshal(content)
			if err != nil {
				log.Println(err)
				continue
			}
			responseChan <- jsonContent
		}
	}()

	return responseChan, errChan
}

func traceroute(target string, param TraceRouteRequest) (outputChan chan string, errChan chan error) {
	outputChan = make(chan string)
	errChan = make(chan error, 1)

	go func() {
		defer close(outputChan)
		defer close(errChan)

		// path := filepath.Join(getCurrentFileDirname(), "traceroute")
		cmd := exec.Command("sudo", "-S", "cppmodules/traceroute",
			"-f", strconv.Itoa(param.First_ttl), "-m", strconv.Itoa(param.Max_ttl),
			"-w", strconv.Itoa(param.WaitTime), "-q", strconv.Itoa(param.Nqueries),
			target, strconv.Itoa(param.PacketSize))

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

		stdinPipe.Write([]byte("''''\n"))

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
