//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#include "router.h"

int ownAddr;
int tcpPort;
int udpPort;

int tcpSocket;
int udpSocket;

int uniqueNumRouters;

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

	if (udpSocket < 0) {
		printMessage("ERROR CREATING UDP SOCKET");
		cerr << "ERROR CREATING UDP SOCKET" << endl;
		exit(EXIT_FAILURE);
	}

	printMessage("UDP SOCKET CREATED");
	ss.str("");
	ss << "My UDP Port is: " << udpPort;
	printMessage(ss.str());

	sockaddr_in ServAddrUDP;
	ServAddrUDP.sin_family = AF_INET;
	ServAddrUDP.sin_addr.s_addr = htonl(INADDR_ANY);
	ServAddrUDP.sin_port = htons(udpPort);

	if (::bind(udpSocket, (struct sockaddr *) &ServAddrUDP, sizeof(ServAddrUDP)) < 0) {
		printMessage("bind failed");
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

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

	fd_set readfds;    // master file descriptor list
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
				ss << "Message received was: " << packet;
				printMessage(ss.str());
//				cout << ss.str() << endl;
				string msg = packet;
				vector <string> str;
				boost::split(str, msg, boost::is_any_of(" "));
				if (str[0].compare("START_LS_ACK") == 0) {
					ss.str("");
					ss << "Starting LS Protocol";
					printMessage(ss.str());
					ss.str("");
					ss << "Expected conTable size: " << str[1];
					printMessage(ss.str());
					uniqueNumRouters = stoi(str[2]);
					graph(uniqueNumRouters);
					ss.str("");
					ss << "Number of unique routers: " << uniqueNumRouters;
					printMessage(ss.str());
					if (startLinkState(stoi(string(str[1])))) {
						sendLSFinish(); //finish LS wait to start DIJKSTRA

						recvd = recv(tcpSocket, packet, sizeof(packet), 0);

						if (recvd < 0) {
							fprintf(stderr, "Issue with recv \n");
							printf("errno %d", errno);
							exit(EXIT_FAILURE);
						}

						ss.str("");
						ss << "Message received was: " << packet;
						printMessage(ss.str());
						cout << ss.str() << endl;
						msg = packet;
						str.clear();
						boost::split(str, msg, boost::is_any_of(" "));
						if (str[0].compare("START_DIJKSTRA_ACK") == 0) {
							ss.str("");
							ss << "Starting LS Protocol";
							printMessage(ss.str());
							for (int i = 0; i < signed(conTable.size()); ++i) {
								addEdge(conTable[i].src, conTable[i].dest, conTable[i].cost);
							}
							for (int i = 0; i < V; i++) {
								vector <SPT> temp = shortestPath(i);
								for (int i = 0; i < signed(temp.size()); ++i) {
									ShortPathTree.push_back(temp[i]);
								}
							}
							printSPT(ownAddr);
							sendDIJKSTRAFinish(); //finish DIJKSTRA wait to start traffic
							shortestPath();
						}
					}
				}
				/*
				//ADD lines for dijkstras
				recvd = recv(tcpSocket, packet, sizeof(packet), 0);

				if (recvd < 0) {
					fprintf(stderr, "Issue with recv \n");
					printf("errno %d", errno);
					exit(EXIT_FAILURE);
				}

				ss.str("");
				ss << "Message received was: " << packet;
				printMessage(ss.str());
				//cout << ss.str() << endl;
				msg = packet;
				//vector <string> str; might fuck up
				boost::split(str, msg, boost::is_any_of(" "));
				if (str[0].compare("START_DIJK_ACK") == 0) { 
					for (int i = 0; i < signed(conTable.size()); ++i) {
						addEdge(conTable[i].src,conTable[i].dest,conTable[i].cost);
					}
					for(int i = 0; i < V; i++){
						vector<SPT> temp = shortestPath(i);
						for (int i = 0; i < signed(temp.size()); ++i) {
							ShortPathTree.push_back(temp[i]);
							}
					printSPT(ownAddr);
					}
				}*/
			}
		}
	}
}

void Router::shortestPath() {
	fd_set readfds;    // master file descriptor list
//	int sd, n, sv;
	int n, sv;
	while (1) {
		//print to log file "reciving Node information from manager"
		char packet[1000];
		memset(&packet, 0, sizeof(packet));
//		char msg[1000];
//		memset(&msg, 0, sizeof(msg));
		FD_ZERO(&readfds);
		FD_SET(udpSocket, &readfds);
		FD_SET(tcpSocket, &readfds);

		n = udpSocket + 1;
		sv = select(n, &readfds, NULL, NULL, NULL);

		if (sv == -1) {
			perror("select");
		} else {
			// one or both of the descriptors have data
			if (FD_ISSET(tcpSocket, &readfds)) {
				int recvd = -1;

				ss.str("");
				ss << "receiving Node information from manager";
				printMessage(ss.str());
				recvd = recv(tcpSocket, packet, sizeof(packet), 0);

				if (recvd < 0) {
					fprintf(stderr, "Issue with recv \n");
					printf("errno %d", errno);
					exit(EXIT_FAILURE);
				}

				ss.str("");
				ss << "Message received was: " << packet;
				printMessage(ss.str());
				cout << ss.str() << endl;
				vector <string> str;
				str.clear();
				boost::split(str, packet, boost::is_any_of(" "));
				if (str[0].compare("QUIT") == 0) {
					sendQUITFinish();
					ss.str("");
					ss << "Router " << ownAddr << " sutting down, Goodbye!";
					printMessage(ss.str());
//					done = true;
				} else {
					//send to next place
					int dest = stoi(str[0]);
					if (dest != ownAddr) {
						int next;
						int port;
						for (int i = 0; i < signed(finSPTable.size()); ++i) {
							if (dest == finSPTable[i].dest) {
								next = finSPTable[i].nextHop;
								break;
							}
						}
						for (int i = 0; i < signed(conTable.size()); ++i) {
							if (next == conTable[i].dest) {
								port = conTable[i].destUDP;
								break;
							}
						}
						//send message to router
						ss.str("");
						ss << "Packed is destined for Router " << dest
						   << ", Next hop is Router "
						   << next;
						printMessage(ss.str());
						
						cout << "REC FROM TCP -- SENDING TO " << next << " ON PORT " << port << endl; 

						memset(&packet, 0, sizeof(packet));
						struct sockaddr_in neighborUdpSocket;
						strcpy(packet, to_string(dest).c_str());
						neighborUdpSocket.sin_family = AF_INET;
						neighborUdpSocket.sin_addr.s_addr = INADDR_ANY;    //used INADDR_ANY because i think thats local addresses
						neighborUdpSocket.sin_port = htons(port);
						sendto(udpSocket, &packet, sizeof(packet), 0,
							   (struct sockaddr *) &neighborUdpSocket,
							   sizeof(neighborUdpSocket));
					} else {
						sendMessageFinish();
					}
				}
			}
			if (FD_ISSET(udpSocket, &readfds)) {
				int recvd = -1;

//				ss.str("");
//				ss << "receiving information from router";
//				printMessage(ss.str());
				recvd = recv(udpSocket, packet, sizeof(packet), 0);

				if (recvd < 0) {
					fprintf(stderr, "Issue with recv \n");
					printf("errno %d", errno);
					exit(EXIT_FAILURE);
				}
				vector <string> str;
				boost::split(str, packet, boost::is_any_of(" "));
				if (str.size() > 1) {
					ss.str("");
					ss << "Limited Broadcast Data. UDP Packet dropped: " << packet;
					printMessage(ss.str());
				} else {
					ss.str("");
					ss << "Receiving information from router";
					printMessage(ss.str());
					ss.str("");
					ss << "Message received was: " << packet;
					printMessage(ss.str());
					cout << ss.str() << endl;
					int dest = stoi(str[0]);
					if (dest != ownAddr) {
						int next;
						int port;
						for (int i = 0; i < signed(finSPTable.size()); ++i) {
							if (dest == finSPTable[i].dest) {
								next = finSPTable[i].nextHop;
								break;
							}
						}
						for (int i = 0; i < signed(conTable.size()); ++i) {
							if (next == conTable[i].dest) {
								port = conTable[i].destUDP;
								break;
							}
						}
						//send message to router
						ss.str("");
						ss << "Packed is destined for Router " << dest
						   << ", Next hop is Router "
						   << next;
						printMessage(ss.str());
						
						cout << "UDP REC -- SENDING TO " << next << " ON PORT " << port << endl; 

						memset(&packet, 0, sizeof(packet));
						struct sockaddr_in neighborUdpSocket;
						strcpy(packet, to_string(dest).c_str());
						neighborUdpSocket.sin_family = AF_INET;
						neighborUdpSocket.sin_addr.s_addr = INADDR_ANY;    //used INADDR_ANY because i think thats local addresses
						neighborUdpSocket.sin_port = htons(port);
						sendto(udpSocket, &packet, sizeof(packet), 0,
							   (struct sockaddr *) &neighborUdpSocket,
							   sizeof(neighborUdpSocket));
					} else {
						sendMessageFinish();
					}
				}
			}
		}
	}
}

void Router::sendMessageFinish() {
	printMessage("Message reached destination, sending Message_ACK");
	char routerInfo[100];
	memset(&routerInfo, 0, sizeof(routerInfo));
	stringstream lsFinish;
	lsFinish << "Destination Router " << ownAddr << " has received the packet.";
	strcpy(routerInfo, lsFinish.str().c_str());
	send(tcpSocket, &routerInfo, sizeof(routerInfo), 0);    //sends ready msg to manager
}

void Router::sendQUITFinish() {
	printMessage("Finished traffic, sending QUIT_ACK");
	char routerInfo[100];
	memset(&routerInfo, 0, sizeof(routerInfo));
	stringstream lsFinish;
	lsFinish << "QUIT_ACK " << ownAddr;
	strcpy(routerInfo, lsFinish.str().c_str());
	send(tcpSocket, &routerInfo, sizeof(routerInfo), 0);    //sends ready msg to manager

}

void Router::sendDIJKSTRAFinish() {
	printMessage("Finished DIJKSTRA, sending FINISH_DIJKSTRA_ACK");
	char routerInfo[100];
	memset(&routerInfo, 0, sizeof(routerInfo));
	stringstream lsFinish;
	lsFinish << "FINISH_DIJKSTRA_ACK " << ownAddr;
	strcpy(routerInfo, lsFinish.str().c_str());
	send(tcpSocket, &routerInfo, sizeof(routerInfo), 0);    //sends ready msg to manager
}

void Router::sendLSFinish() {
	printMessage("Finished LS, sending FINISH_LS_ACK");
	char routerInfo[100];
	memset(&routerInfo, 0, sizeof(routerInfo));
	stringstream lsFinish;
	lsFinish << "FINISH_LS_ACK " << ownAddr;
	strcpy(routerInfo, lsFinish.str().c_str());
	send(tcpSocket, &routerInfo, sizeof(routerInfo), 0);    //sends ready msg to manager
}

bool Router::startLinkState(int expectedConTableSize) {
	printMessage("START METHOD: startLinkState()");
	vector <sockaddr_in> sockets;
	for (int i = 0; i < signed(udpPorts.size()); ++i) {
		char msg[100];
		memset(&msg, 0, sizeof(msg));
		struct sockaddr_in neighborUdpSocket;
		neighborUdpSocket.sin_family = AF_INET;
		neighborUdpSocket.sin_addr.s_addr = INADDR_ANY;    //used INADDR_ANY because i think thats local addresses
		neighborUdpSocket.sin_port = htons(udpPorts[i]);
		sockets.push_back(neighborUdpSocket);
		Message message;
		message.srcUDP = udpPort;
		strcpy(message.table, compressConTable().c_str());
		string table = compressConTable();
		strcpy(msg, table.c_str());
		ss.str("");
		ss << "Sending connection table to port: " << udpPorts[i];
		printMessage(ss.str());
		sendto(udpSocket, &message, sizeof(message), 0, (struct sockaddr *) &neighborUdpSocket,
			   sizeof(neighborUdpSocket));
	}

//	sockaddr_in their_addr;    //for connecting to incoming connections socket
//	socklen_t sin_size = sizeof(their_addr);

	fd_set readfds;    // master file descriptor list
//	int sd, n, sv;
	int n, sv;

	while (signed(conTable.size()) != expectedConTableSize) {
		char packet[1000];
		memset(&packet, 0, sizeof(packet));
		FD_ZERO(&readfds);
		FD_SET(udpSocket, &readfds);
		Message message;

		n = udpSocket + 1;
		sv = select(n, &readfds, NULL, NULL, NULL);

		if (sv == -1) {
			perror("select");
		} else {
			// one or both of the descriptors have data
			if (FD_ISSET(udpSocket, &readfds)) {
				int recvd = -1;
				recvd = recv(udpSocket, &message, sizeof(message), 0);

				if (recvd < 0) {
					fprintf(stderr, "Issue with recv \n");
					printf("errno %d", errno);
					exit(EXIT_FAILURE);
				}

				stringstream ss;
				ss << "Message recieved was: " << message.table;
				printMessage(ss.str());
				tempconTable = createConTable(message.table);
				compare();
				int src = message.srcUDP;
				bool exists = false;
				for (int j = 0; j < signed(udpPorts.size()); ++j) {
					if (src == udpPorts[j]) {
						exists = true;
					}
				}
				if (!exists) {
					udpPorts.push_back(src);
					struct sockaddr_in neighborUdpSocket;
					neighborUdpSocket.sin_family = AF_INET;
					neighborUdpSocket.sin_addr.s_addr = INADDR_ANY;    //used INADDR_ANY because i think thats local addresses
					neighborUdpSocket.sin_port = htons(src);
					sockets.push_back(neighborUdpSocket);
//					cout << ownAddr << " pushed back: " << src << endl;
				}
				for (int i = 0; i < signed(sockets.size()); ++i) {
					Message temp;
					temp.srcUDP = udpPort;
					strcpy(temp.table, compressConTable().c_str());
					ss.str("");
					ss << "Sending connection table to port: " << udpPorts[i];
					printMessage(ss.str());
					sendto(udpSocket, &temp, sizeof(temp), 0, (struct sockaddr *) &sockets[i],
						   sizeof(sockets[i]));
				}
			}
		}
	}
	ss.str("");
	ss << "Table is now complete.";
	printMessage(ss.str());
	return true;
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

vector <Route> Router::createConTable(string table) {
	vector <Route> Con;
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
		for (int j = 0; j < signed(Con.size()); ++j) {
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

string Router::compressConTable() { //puts all the Routes in string delimited by ","
	stringstream ss;
	for (int i = 0; i < signed(conTable.size()); ++i) {
		ss << conTable[i].src << " " << conTable[i].dest << " " << conTable[i].cost << " " << conTable[i].destUDP
		   << ",";
	}
	string table = ss.str();
	return table.substr(0, table.length() - 1);
}

void Router::compare() {
	printMessage("START METHOD: compare()");
	for (int i = 0; i < signed(tempconTable.size()); ++i) {
		for (int j = 0; j < signed(conTable.size()); ++j) {
			if (tempconTable[i].src == conTable[j].src) {
				if (tempconTable[i].dest == conTable[j].dest) {
//					tempconTable.erase(tempconTable.begin() + i);
					tempconTable[i].dest = -1;
				}
			}
		}
	}
//	printMessage("done with first loop");
	for (int i = 0; i < signed(tempconTable.size()); ++i) {
		if (tempconTable[i].dest != -1) {
			conTable.push_back(tempconTable[i]);
		}
	}
	tempconTable.clear();
	printMessage("Connection Table Updated");
	ss.str("");
	ss << "Table: ";
	for (int j = 0; j < signed(conTable.size()); ++j) {
		ss << "src: " << conTable[j].src
		   << " dest: " << conTable[j].dest
		   << " cost: " << conTable[j].cost
		   << " destUDP: " << conTable[j].destUDP << " | ";
	}
	printMessage(ss.str());
}

void Router::createUdpVector() {
	ss.str("");
	ss << "Creating vector of UDP ports";
	printMessage(ss.str());
	for (int i = 0; i < signed(conTable.size()); ++i) {
		udpPorts.push_back(conTable[i].destUDP);
	}
	ss.str("");
	ss << "UDP port vector size: " << udpPorts.size();
	printMessage(ss.str());
	ss.str("");
}

/*class Graph
{
    int V;    // No. of vertices
 
    // In a weighted graph, we need to store vertex 
    // and weight pair for every edge
    list< pair<int, int> > *adj;
 
public:
    Graph(int V);  // Constructor
 
    // function to add an edge to graph
    void addEdge(int u, int v, int w);
 
    // prints shortest path from s
    vector<SPT> shortestPath(int s);
};*/

// Allocates memory for adjacency list
void Router::graph(int V) {
	this->V = V;
	adj = new list <pair<int, int>>[V];
}

void Router::addEdge(int u, int v, int w) {
	adj[u].push_back(make_pair(v, w));
	adj[v].push_back(make_pair(u, w));
}

void Router::printSPT(int ownAddr) {
	stringstream ss;
	ss.str("");
	//go through all shortest paths
	for (int i = 0; i < signed(ShortPathTree.size()); ++i) {
		spTable table;
		//find the ones for the table you want
		if (ShortPathTree[i].dest == ownAddr && ShortPathTree[i].hop == -1) {
			ss << "( " << ShortPathTree[i].dest << " , 0 , " << ShortPathTree[i].dest << " )\n";
			table.dest = ShortPathTree[i].dest;
			table.cost = 0;
			table.nextHop = ShortPathTree[i].dest;
			finSPTable.push_back(table);
		} else if (ShortPathTree[i].src == ownAddr) {
			ss << "( " << ShortPathTree[i].dest << " , ";
			table.dest = ShortPathTree[i].dest;
			for (int k = 0; k < signed(conTable.size()); ++k) {
				if ((conTable[k].src == ShortPathTree[i].src && conTable[k].dest == ShortPathTree[i].hop) ||
					(conTable[k].dest == ShortPathTree[i].src && conTable[k].src == ShortPathTree[i].hop)) {
					ss << conTable[k].cost << " , ";
					table.cost = conTable[k].cost;
				}

			}
			ss << ShortPathTree[i].hop << " )\n";
			table.nextHop = ShortPathTree[i].hop;
			finSPTable.push_back(table);
		}

	}
	printMessage(ss.str());
	//cout << ss.str() << endl;
	ss.str("");
}

// Prints shortest paths from src to all other vertices
vector <SPT> Router::shortestPath(int src) {
	vector <SPT> nextHops;
	// Create a set to store vertices that are being
	// prerocessed
	set <pair<int, int>> setds;
	int parent[V];

	// Create a vector for distances and initialize all
	// distances as infinite (INF)
	vector<int> dist(V, INF);

	// Insert source itself in Set and initialize its
	// distance as 0.
	setds.insert(make_pair(0, src));
	dist[src] = 0;

	/* Looping till all shortest distance are finalized
	   then setds will become empty */
	while (!setds.empty()) {
		// The first vertex in Set is the minimum distance
		// vertex, extract it from set.
		pair<int, int> tmp = *(setds.begin());
		setds.erase(setds.begin());

		// vertex label is stored in second of pair (it
		// has to be done this way to keep the vertices
		// sorted distance (distance must be first item
		// in pair)
		int u = tmp.second;

		// 'i' is used to get all adjacent vertices of a vertex
		list < pair < int, int > > ::iterator
		i;
		for (i = adj[u].begin(); i != adj[u].end(); ++i) {
			// Get vertex label and weight of current adjacent
			// of u.
			int v = (*i).first;
			int weight = (*i).second;

			//  If there is shorter path to v through u.
			if (dist[v] > dist[u] + weight) {
				/*  If distance of v is not INF then it must be in
					our set, so removing it and inserting again
					with updated less distance.
					Note : We extract only those vertices from Set
					for which distance is finalized. So for them,
					we would never reach here. */
				if (dist[v] != INF)
					setds.erase(setds.find(make_pair(dist[v], v)));

				// Updating distance of v
				dist[v] = dist[u] + weight;
				parent[v] = u;
				setds.insert(make_pair(dist[v], v));
			}
		}
	}

	// Print shortest distances stored in dist[]
	//printf("Vertex\t   Distance\tPath\n");
	for (int i = 0; i < V; ++i) {
		SPT spt;
		spt.src = i;
		spt.dest = src;
		spt.hop = parent[i];
		if (i == src) { spt.hop = -1; }
		nextHops.push_back(spt);
		// printf("%d \t\t %d\t\t%d\n", i, dist[i],parent[i]);
	}

	return nextHops;
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
	router.createUdpVector();
	ss << "Router: " << ownAddr;
//	cout << ss << endl;
	router.printMessage(ss.str());

	//int V = 10; //10 is just the number or routers to run it on
	//Graph g(V);
	//router.graph(10); //need to find where we know how many routers there are and change number
	router.client();    //call client with given port number

}