//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#include "manager.h"

int tcpSocket;

/*
 * Reads in file, creates vector of unique routers,
 * and vector of Route structs that contain (src dest cost)
 */
void Manager::readFile(ifstream &inFile) {
	printMessage("STARTING METHOD: readFile()");
	string line;

	if (inFile.fail()) {
		printMessage("Error: Failed to open file.");
		cerr << "Error: Failed to open file." << endl;
		exit(1);
	}

	getline(inFile, line);
	int numRouters = stoi(line);

	while (getline(inFile, line) && line != "-1") {
		vector <string> strs;
		boost::split(strs, line, boost::is_any_of(" "));
		Route route;

		int src = stoi(strs[0]);
		int dest = stoi(strs[1]);
		int cost = stoi(strs[2]);

		route.src = src;
		route.dest = dest;
		route.cost = cost;

		routes.push_back(route);

		if (find(uniqRouters.begin(), uniqRouters.end(), src) == uniqRouters.end()) {
			uniqRouters.push_back(src);
		}
		if (find(uniqRouters.begin(), uniqRouters.end(), dest) == uniqRouters.end()) {
			uniqRouters.push_back(dest);
		}

	}

	while (getline(inFile, line) && line != "-1") {
		vector <string> strs;
		boost::split(strs, line, boost::is_any_of(" "));
		Path path;

		int src = stoi(strs[0]);
		int dest = stoi(strs[1]);

		path.src = src;
		path.desireDest = dest;

		wantedPaths.push_back(path);

	}
	cout << wantedPaths.size() << endl;

	createPorts(numRouters);
}

/*
 * Create routers based on number of unique routers.
 * Routers will have their own address (ownAddr) and a
 * vector of Route structs for a connection table (conTable)
 */
void Manager::createRouters() {
	printMessage("STARTING METHOD: createRouters()");

	for (int i = 0; i < signed(uniqRouters.size()); ++i) {
		int src = uniqRouters[i];
		for (int j = 0; j < signed(routes.size()); ++j) {
			if (routes[j].src == src) {
				for (int k = 0; k < signed(uniqRouters.size()); ++k) {
					if (routes[j].dest == uniqRouters[k]) {
						routes[j].destUDP = ports[k];
						conTable.push_back(routes[j]);
					}
				}
			}
		}
	}
	printMessage("Current Connection Table:");
	for (int k = 0; k < signed(conTable.size()); ++k) {
		stringstream ss;
		ss << "src: " << conTable[k].src
		   << " dest: " << conTable[k].dest
		   << " cost: " << conTable[k].cost
		   << " destUDP: " << conTable[k].destUDP;
		//		cout << ss.str() << endl; //For testing purposes
		printMessage(ss.str());

	}
}

/*
 * Fork n number of processes for n routers.
 * Execute router executable
 */
void Manager::routerSpinUp() {
	printMessage("STARTING METHOD: routerSpinUp()");
//    pid_t pid = getpid();
	printMessage("Parent PID: " + to_string(getpid()));
	pid_t childPid;
	int portIndex = 0;
//	int status;
	for (int i = 0; i < signed(uniqRouters.size()); ++i) {
		string table = conTableString(uniqRouters[i], conTable);
		childPid = fork();
		PIDs.push_back(childPid);
		if (!childPid) {
			char *argv[1000];
			memset(&argv, 0, sizeof(argv));
			argv[0] = strdup("router");    //router execution
			argv[1] = (char *) to_string(uniqRouters[i]).c_str();    //router ownAddr
			argv[2] = (char *) to_string(TCP_PORT).c_str();    //TCP port for router->manager
			argv[3] = (char *) to_string(ports[i]).c_str();    //UDP port for router->router
			argv[4] = (char *) table.c_str();    //src dest weight udpPort,src dest weight udpPort...
			stringstream ss;
			ss << "STARTING Router: " << uniqRouters[i] << " for port " << ports[i];
			cout << ss.str() << endl;
			printMessage(ss.str());
			execv(argv[0], argv);
			break;    //don't let child fork again
		} else if (childPid > 0) {
//            wait(&status);	//wait until child executes
		}
		portIndex++;
	}
}

/*
 * Create n UDP port numbers for routers for
 * router->router communication
 */
void Manager::createPorts(int numRouters) {
	printMessage("STARTING METHOD: createPorts()");

	//create distinct ports for tcp connection with router
	for (int i = 0; i < numRouters; i++) {
		int portNum = 2000 + (i * 100);
		ports.push_back(portNum);
	}
}

/*
 * Create TCP connection that will accept from any address
 */
void Manager::establishConnection(int port) {
	printMessage("STARTING METHOD: establishConnection()");
	cout << "port: " << port << endl;

	//sockaddr_in is for socket that a sstone will listen to for incoming connection
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);    //PORT is the sstones own port to listen to
	int routerTCPsocket;
	tcpSocket = socket(PF_INET, SOCK_STREAM, 0);    //incoming socket

	if (tcpSocket < 0) {
		printMessage("socket fail");
		perror("socket fail");
		exit(EXIT_FAILURE);
	}

	if (::bind(tcpSocket, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		printMessage("bind failed");
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(tcpSocket, MAXPENDING) < 0) {
		printMessage("listen failed");
		perror("listen failed");
		exit(EXIT_FAILURE);
	}

	cout << "Listening to PORT: " << ntohs(servAddr.sin_port) << endl;
	printMessage("Listening to PORT: " + to_string(port));

	sockaddr_in their_addr;    //for connecting to incoming connections socket
	socklen_t sin_size = sizeof(their_addr);

	fd_set readfds;    // master file descriptor list
	int n, sv;

	bool dijk = false;
	while (!dijk) {
		char packet[100];
		memset(&packet, 0, sizeof(packet));

		FD_ZERO(&readfds);
		FD_SET(tcpSocket, &readfds);

		n = tcpSocket + 1;
		sv = select(n, &readfds, NULL, NULL, NULL);

		if (sv == -1) {
			perror("select");
		} else {
			// one or both of the descriptors have data
			if (FD_ISSET(tcpSocket, &readfds)) {
				int recvd = -1;

				if ((routerTCPsocket = accept(tcpSocket, (struct sockaddr *) &their_addr, &sin_size)) < 0) {
					perror("accept");
					exit(1);
				}

				recvd = recv(routerTCPsocket, packet, sizeof(packet), 0);

				if (recvd < 0) {
					fprintf(stderr, "Issue with recv \n");
					printf("errno %d", errno);
					exit(EXIT_FAILURE);
				}

				stringstream ss;
				ss << "Message received was: " << packet;
				printMessage(ss.str());
				cout << ss.str() << endl;
				routerTcpSockets.push_back(routerTCPsocket);

				if (routerTcpSockets.size() == uniqRouters.size()) {
					ss.str("");
					ss << "All routers are ready.";
					printMessage(ss.str());
					cout << ss.str() << endl;
					for (int i = 0; i < signed(routerTcpSockets.size()); ++i) {
						char msg[100];
						ss.str("");
						ss << "Sending START_LS_ACK";
						printMessage(ss.str());
						ss.str("");
						ss << "START_LS_ACK " << conTable.size() << " " << uniqRouters.size();
						strcpy(msg, ss.str().c_str());
						send(routerTcpSockets[i], &msg, sizeof(msg), 0);
					}
					bool lsDone = false;
					vector<int> responses;
					/*
					 * This while loop will loop over all routerTcpSockets that were created above.
					 * Once a response has been heard from all unique routers, the while loop exits.
					 */
					while (!lsDone) {
						for (int i = 0; i < signed(routerTcpSockets.size()); ++i) {
							memset(&packet, 0, sizeof(packet));
							recvd = recv(routerTcpSockets[i], packet, sizeof(packet), 0);

							if (recvd < 0) {
								fprintf(stderr, "Issue with recv \n");
								printf("errno %d", errno);
								exit(EXIT_FAILURE);
							}

							vector <string> r;
							boost::split(r, packet, boost::is_any_of(" "));

							stringstream ss;
							ss << "Message received was: " << r[0] << " from Router: " << r[1];
							printMessage(ss.str());
							cout << ss.str() << endl;

							Translate translate;
							translate.port = stoi(r[1]);
							translate.sock = routerTcpSockets[i];
							translations.push_back(translate);

							bool contains = false;
							int router = stoi(r[1]);
							for (int j = 0; j < signed(responses.size()); ++j) {
								if (router == responses[j]) {
									contains = true;
								}
							}
							if (!contains) {
								responses.push_back(router);
							}
							if (responses.size() == uniqRouters.size()) {
								printMessage("All routers finished LS");
								lsDone = true;
							}
						}
					}//finsih LS while loop
					for (int i = 0; i < signed(routerTcpSockets.size()); ++i) {
						char msg[100];
						memset(&msg, 0, sizeof(msg));
						ss.str("");
						ss << "Sending START_DIJKSTRA_ACK";
						printMessage(ss.str());
						ss.str("");
						ss << "START_DIJKSTRA_ACK " << conTable.size();
						strcpy(msg, ss.str().c_str());
						send(routerTcpSockets[i], &msg, sizeof(msg), 0);
					}
					lsDone = false;
					responses.clear();
					while (!lsDone) {
						for (int i = 0; i < signed(routerTcpSockets.size()); ++i) {
							memset(&packet, 0, sizeof(packet));
							recvd = recv(routerTcpSockets[i], packet, sizeof(packet), 0);

							if (recvd < 0) {
								fprintf(stderr, "Issue with recv \n");
								printf("errno %d", errno);
								exit(EXIT_FAILURE);
							}

							vector <string> r;
							boost::split(r, packet, boost::is_any_of(" "));

							stringstream ss;
							ss << "Message received was: " << r[0] << " from Router: " << r[1];
							printMessage(ss.str());
							cout << ss.str() << endl;
							bool contains = false;
							int router = stoi(r[1]);
							for (int j = 0; j < signed(responses.size()); ++j) {
								if (router == responses[j]) {
									contains = true;
								}
							}
							if (!contains) {
								responses.push_back(router);
							}
							if (responses.size() == uniqRouters.size()) {
								printMessage("All routers finished DIJKSTRA");
								lsDone = true;
								dijk = true;
							}
						}
					}//finsih DIJKSTRA while loop
					printMessage("START METHOD: findPath()");
					while (1) {
						for (int k = 0; k < signed(wantedPaths.size()); ++k) {
							for (int i = 0; i < signed(uniqRouters.size()); ++i) {
								if (uniqRouters[i] == wantedPaths[k].src) {
									char msg[100];
									memset(&msg, 0, sizeof(msg));
									stringstream ss;
									ss << wantedPaths[k].desireDest;
									strcpy(msg, ss.str().c_str());
									ss.str("");
									ss << "Sending: " << wantedPaths[k].desireDest << " to Router: " << uniqRouters[i];
									printMessage(ss.str());
									send(translate(uniqRouters[i]), &msg, sizeof(msg), 0);
									bool received = false;
									while (!received) {
										char packet[100];
										memset(&packet, 0, sizeof(packet));
										int recvd = -1;

										cout << "here" << endl;
										recvd = recv(translate(uniqRouters[i]), packet, sizeof(packet), 0);
										cout << "now here" << endl;

										if (recvd < 0) {
											fprintf(stderr, "Issue with recv \n");
											printf("errno %d", errno);
											exit(EXIT_FAILURE);
										}

										stringstream ss;
										ss << "Message from router was: " << packet;
										printMessage(ss.str());
										cout << ss.str() << endl;

										vector <string> strs;
										boost::split(strs, packet, boost::is_any_of(" "));
										if (strs[0].compare("Destination") == 0) {
											received = true;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
//	findPath();
}

void Manager::findPath() {
	printMessage("START METHOD: findPath()");
	for (int k = 0; k < signed(wantedPaths.size()); ++k) {
		for (int i = 0; i < signed(uniqRouters.size()); ++i) {
			if (uniqRouters[i] == wantedPaths[k].src) {
				char msg[100];
				memset(&msg, 0, sizeof(msg));
				stringstream ss;
				ss << wantedPaths[k].desireDest;
				strcpy(msg, ss.str().c_str());
				ss.str("");
				ss << "Sending: " << wantedPaths[k].desireDest << " to Router: " << uniqRouters[i];
				printMessage(ss.str());
				send(translate(uniqRouters[i]), &msg, sizeof(msg), 0);
				bool received = false;
//				unsigned int microseconds = 1000;
//				usleep(microseconds);

//				fd_set readfds;	// master file descriptor list]
//				int n, sv;
//
//				sockaddr_in their_addr;    //for connecting to incoming connections socket
//				socklen_t sin_size = sizeof(their_addr);
				while (!received) {
					char packet[100];
					memset(&packet, 0, sizeof(packet));
					for (int j = 0; j < signed(routerTcpSockets.size()); ++j) {

						int recvd = -1;

						cout << "here" << endl;
						recvd = recv(routerTcpSockets[j], packet, sizeof(packet), 0);
						cout << "now here" << endl;

						if (recvd < 0) {
							fprintf(stderr, "Issue with recv \n");
							printf("errno %d", errno);
							exit(EXIT_FAILURE);
						}

						stringstream ss;
						ss << "Message from router was: " << packet;
						printMessage(ss.str());
						cout << ss.str() << endl;

						vector <string> strs;
						boost::split(strs, packet, boost::is_any_of(" "));
						cout << strs[0] << endl;
						if (strs[0].compare("Destination") == 0) {
							received = true;
						}
					}
				}
			}
		}
	}
}

int Manager::translate(int port) {
	for (int i = 0; i < signed(translations.size()); ++i) {
		if (translations[i].port == port) {
			return translations[i].sock;
		}
	}
	return port;
}

void Manager::printMessage(string message) {
	ofstream file;
	string filename = "manager.out";
	file.open(filename, ofstream::out | ofstream::app);
	file << currentDateTime() << ": " << message << "\n";
	file.close();
}

const string Manager::currentDateTime() {
//adapted from: https://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	return buf;
}

void Manager::killProcesses() {
	for (int i = 0; i < signed(PIDs.size()); ++i) {
		cout << "PIDs.at(" << i << "): " << PIDs.at(i) << endl;
		char syscall[100];
		char buf[100];
		sprintf(buf, "%d", PIDs.at(i));
		strcpy(syscall, "kill -9 ");
		strcat(syscall, buf);
		system(syscall);
		printMessage("RUNNING COMMAND: " + string(syscall));
	}
}

string Manager::conTableString(int src, vector <Route> routes) {
	stringstream ss;
	for (int i = 0; i < signed(routes.size()); ++i) {
		if (src == routes[i].src) {
			ss << routes[i].src << " " << routes[i].dest << " " << routes[i].cost << " " << routes[i].destUDP << ",";
		}
	}
	string table = ss.str();
	return table.substr(0, table.length() - 1);
}

int main(int argc, char *argv[]) {
	Manager manager;

	manager.printMessage("STARTING MANAGER######################################");

	ifstream file(argv[1]);

	manager.readFile(file);
	manager.createRouters();

	//will first create TCP connection and wait for incomming connections
	//needed first, so routers have something to connect to
	thread managerTCPCreate(&Manager::establishConnection, &manager, TCP_PORT);
	thread managerRouterCreate(&Manager::routerSpinUp, &manager);    //execute n routers
	managerTCPCreate.join();
	managerRouterCreate.join();

	cout << "PID SIZE: " << manager.PIDs.size() << endl;

	manager.killProcesses();
	//system("killall manager"); //temporary to kill processes
}