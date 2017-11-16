//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#include "router.h"

int udpPort;
int tcpPort;

void Router::client(int tcpPort) {
	printMessage("START METHOD: client()");
	int clientSock;
	//int buffSize = 500;
	//char buff[buffSize];

	clientSock = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSock < 0) {
		printMessage("ERROR CREATING CLIENT SOCKET");
		cerr << "ERROR CREATING CLIENT SOCKET" << endl;
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in ServAddr;
	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = INADDR_ANY;//used INADDR_ANY because i think thats local addresses
	ServAddr.sin_port = htons(tcpPort);

	cout << "Connecting to server..." << endl;
	if (connect(clientSock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0) {
		cerr << "ERROR IN CONNECT" << endl;
		close(clientSock);
		exit(EXIT_FAILURE);
	}

	cout << "Connected on port: " <<  tcpPort << endl;
	printMessage("Connected on port: " + to_string(tcpPort));

	char routerInfo[100];
	strcat(routerInfo, to_string(udpPort).c_str());
	send(clientSock, &routerInfo, sizeof(routerInfo), 0);	//sends routers UDP port to manager
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

void Router::createFileName(char* argv1){
	string temp = string(argv1);
	filename += temp;
	filename += ".out";
}

int main(int argc, char *argv[]) {
	
	Router router;
	router.createFileName(argv[1]);
	router.printMessage("STARTING ROUTER###########################################");

	tcpPort = atoi(argv[1]);
	udpPort = tcpPort + 1000;

	router.client(tcpPort);//call client with given port number

	
}