//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#include "router.h"

void Router::client(char* port) {
	int clientSock;
	//int buffSize = 500;
	//char buff[buffSize];

	clientSock = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSock < 0) {
		cerr << "ERROR CREATING CLIENT SOCKET" << endl;
		exit(EXIT_FAILURE);
	}

//	cout << "Client socket created" << endl;

	struct sockaddr_in ServAddr;
	ServAddr.sin_family = AF_INET;
	ServAddr.sin_addr.s_addr = INADDR_ANY;//used INADDR_ANY because i think thats local addresses
	ServAddr.sin_port = htons(atoi(port));

	cout << "Connecting to server..." << endl;
	if (connect(clientSock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0) {
		cerr << "ERROR IN CONNECT" << endl;
		close(clientSock);
		exit(EXIT_FAILURE);
	}

	cout << "Connected on port: " <<  port << endl;
//	sstones.erase(sstones.begin() + index);
//	ConInfo info;
//	string temp = address;
//	info.parentPort = AWGET_PORT;
//	strcpy(info.url, url.c_str());
//	strcpy(info.sstones, serialize().c_str());
//	send(clientSock, &info, sizeof(info), 0);
}


int main(int argc, char *argv[]) {
	
	Router router;
	
	router.client(argv[1]);//call client with given port number
	
	//cout << "Started router process: " << argv[1] << endl;
	
}