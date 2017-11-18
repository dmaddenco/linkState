//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#include "router.h"

int udpPort;
int tcpPort;

int udpSocket;
int tcpSocket;

void Router::client() {
	printMessage("START METHOD: client()");
	//int buffSize = 500;
	//char buff[buffSize];
	printMessage("CREATING TCP SOCKET");
	tcpSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (tcpSocket < 0) {
		printMessage("ERROR CREATING TCP SOCKET");
		cerr << "ERROR CREATING TCP SOCKET" << endl;
		exit(EXIT_FAILURE);
	}

	printMessage("TCP SOCKET CREATED");

	struct sockaddr_in ServAddr;
	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = INADDR_ANY;    //used INADDR_ANY because i think thats local addresses
	ServAddr.sin_port = htons(tcpPort);

	printMessage("CREATING UDP SOCKET");
	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if (tcpSocket < 0) {
		printMessage("ERROR CREATING UDP SOCKET");
		cerr << "ERROR CREATING UDP SOCKET" << endl;
		exit(EXIT_FAILURE);
	}

	printMessage("UDP SOCKET CREATED");

	cout << "Connecting to server..." << endl;
	printMessage("CONNECTING TO SERVER THROUGH TCP PORT");

	if (connect(tcpSocket, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0) {
		printMessage("ERROR IN CONNECTING TO SERVER THROUGH TCP PORT");
		cerr << "ERROR IN CONNECT" << endl;
		close(tcpSocket);
		exit(EXIT_FAILURE);
	}

	cout << "Connected on port: " << tcpPort << endl;
	printMessage("Connected on port: " + to_string(tcpPort));

	char routerInfo[100];
//	strcat(routerInfo, to_string(udpPort).c_str());
	strcpy(routerInfo, to_string(udpPort).c_str());
	send(tcpSocket, &routerInfo, sizeof(routerInfo), 0);    //sends routers UDP port to manager
}

void Router::printMessage(string message) {
	ofstream file;
	file.open(filename, ofstream::out | ofstream::app);
	file << currentDateTime() << ": " << message << "\n";
	file.close();
}

const string Router::currentDateTime() {
//adapted from: https://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	return buf;
}

void Router::createFileName(char *argv1) {
	string temp = string(argv1);
	filename += temp;
	filename += ".out";
}

int main(int argc, char *argv[]) {
	Router router;
	router.createFileName(argv[3]);
	router.printMessage("STARTING ROUTER###########################################");

	tcpPort = atoi(argv[2]);
	udpPort = atoi(argv[3]);

	stringstream ss;
	router.ownAddr = atoi(argv[1]);
	ss << "ROUTER: " << router.ownAddr;
	string message = ss.str();
	router.printMessage(message);
//	cout << "udp: " << argv[3] << endl;
	router.client();    //call client with given port number
}