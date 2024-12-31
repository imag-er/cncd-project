package mockdata

var DNSQueryData = []string{
	`{"server": "8.8.8.8", "hostname": "example.com", "status": "Resolved"}`,
    `{"server": "8.8.4.4", "hostname": "example.com", "status": "Resolved"}`,
    `{"server": "1.1.1.1", "hostname": "example.com", "status": "Resolved"}`,
    `{"server": "9.9.9.9", "hostname": "example.com", "status": "Resolved"}`,
}

var PortScanData = []string{
	`{"port": 280, "status": "open", "local_address": "1.2.3.4","foreign_addresss": "5.6.7.8"}`,
	`{"port": 380, "status": "open", "local_address": "1.2.3.4","foreign_addresss": "5.6.7.8"}`,
	`{"port": 480, "status": "close", "local_address": "1.2.3.4","foreign_addresss": "5.6.7.8"}`,
	`{"port": 580, "status": "close", "local_address": "1.2.3.4","foreign_addresss": "5.6.7.8"}`,
	`{"port": 680, "status": "open", "local_address": "1.2.3.4","foreign_addresss": "5.6.7.8"}`,
}

var TracerouteData = []string{
	`{"address": "112.64.195.1","delay": [25.145, 68.583, 105.536]}`,
	`{"address": "139.226.196.17","delay": [102.354,  32.490,  15.819]}`,
	`{"address": "139.226.223.253","delay": [25.145, 68.583, 105.536]}`,
	`{"address": "219.158.107.6","delay": [312.897]}`,
}
