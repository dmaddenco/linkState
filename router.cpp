//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#include "router.h"

int ownAddr;
int tcpPort;
int udpPort;

int tcpSocket;
int udpSocket;

stringstream ss;

void Router::client() {
	printMessage("START METHOD: client()");
	//int buffSize = 500;
	//char buff[buffSize];
	ss.str("");
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
	ss.str("");
	ss << "My UDP Port is: " << udpPort;
	printMessage(ss.str());

	cout << "Connecting to server..." << endl;
	printMessage("CONNECTING TO SERVER THROUGH TCP PORT");

	if (connect(tcpSocket, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0) {
		printMessage("ERROR IN CONNECTING TO SERVER THROUGH TCP PORT");
		cerr << "ERROR IN CONNECT" << endl;
		close(tcpSocket);
		exit(EXIT_FAILURE);
	}

	cout << "Connected on port: " << tcpPort << endl;
	ss.str("");
	ss << "Connected to Manager on TCP Port: " << tcpPort;
	printMessage(ss.str());

	char routerInfo[100];
	memset(&routerInfo, 0, sizeof(routerInfo));
	ss.str("");
	ss << "Router: " << ownAddr << " ready. UDP Port: " << udpPort;
	strcpy(routerInfo, ss.str().c_str());
	send(tcpSocket, &routerInfo, sizeof(routerInfo), 0);    //sends ready msg to manager

	fd_set readfds;	// master file descriptor list
//	int sd, n, sv;
	int n, sv;

	while (1) {
		char packet[100];
		memset(&packet, 0, sizeof(packet));
		FD_ZERO(&readfds);
		FD_SET(udpSocket, &readfds);
		FD_SET(tcpSocket, &readfds);

		n = tcpSocket + 1;
		sv = select(n, &readfds, NULL, NULL, NULL);

		if (sv == -1) {
			perror("select");
		} else {
			// one or both of the descriptors have data
			if (FD_ISSET(tcpSocket, &readfds)) {
				int recvd = -1;

//				if ((tcpSocket = accept(tcpSocket,(struct sockaddr *) &their_addr, &sin_size))<0){
//					perror("accept");
//					exit(1);
//				}

				recvd = recv(tcpSocket, packet, sizeof(packet), 0);

				if (recvd < 0) {
					fprintf(stderr, "Issue with recv \n");
					printf("errno %d", errno);
					exit(EXIT_FAILURE);
				}

				stringstream ss;
				ss << "Message recieved was: " << packet;
				printMessage(ss.str());
				cout << ss.str() << endl;
			}
		}
	}
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

vector<Route> Router::createConTable(string table) {
	vector<Route> Con;
	ss.str("");
	if (table != "") {
		vector <string> r;
		boost::split(r, table, boost::is_any_of(","));
		Route route;
		for (int i = 0; i < signed(r.size()); i++) {
			vector <string> strs;
			string line = r[i];
			boost::split(strs, line, boost::is_any_of(" "));
			for (int j = 0; j < signed(strs.size()); j += 4) {
				route.src = stoi(strs[0]);
				route.dest = stoi(strs[1]);
				route.cost = stoi(strs[2]);
				route.destUDP = stoi(strs[3]);
				Con.push_back(route);
			}
		}
		ss << "My imediate neighbors are: | ";
		for (int j = 0; j < Con.size(); ++j) {
			ss << "src: " << Con[j].src
				 << " dest: " << Con[j].dest
				 << " cost: " << Con[j].cost
				 << " destUDP: " << Con[j].destUDP << " | ";
		}
	}
	printMessage(ss.str());
	ss.str("");
	return Con;
}

string Router::compressConTable(){ //puts all the Routes in string delimited by ","
	stringstream ss;
	for (int i = 0; i < signed(conTable.size()); ++i) {
			ss << conTable[i].src << " " << conTable[i].dest << " " << conTable[i].cost << " " << conTable[i].destUDP << ",";
	}
	string table = ss.str();
	return table.substr(0, table.length()-1);
}

void Router::compare(){

	for (int i = 0; i < signed(tempconTable.size()); ++i) {
		for (int j = 0; j < signed(conTable.size()); ++j) {
			if (tempconTable[i].src == conTable[j].src){
				if (tempconTable[i].dest == conTable[j].dest){
					tempconTable.erase(tempconTable.begin() + i);
				}
			}
		}
	}
	for (int i = 0; i < signed(tempconTable.size()); ++i) {
		conTable.push_back(tempconTable[i]);
	}
	tempconTable.clear();
	

}

int main(int argc, char *argv[]) {
	ss.str("");
	Router router;
	router.createFileName(argv[3]);
	router.printMessage("STARTING ROUTER###########################################");

	ownAddr = atoi(argv[1]);
	tcpPort = atoi(argv[2]);
	udpPort = atoi(argv[3]);
	router.conTable = router.createConTable(argv[4]);

	ss << "Router: " << ownAddr;
//	cout << ss << endl;
	router.printMessage(ss.str());
	router.client();    //call client with given port number

}