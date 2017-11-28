//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#include "manager.h"

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
//		Router router;
		int src = uniqRouters[i];
		for (int j = 0; j < signed(routes.size()); ++j) {
			if (routes[j].src == src) {
				for (int k = 0; k < signed(uniqRouters.size()); ++k) {
					if (routes[j].dest == uniqRouters[k]) {
						routes[j].destUDP = ports[i];
						conTable.push_back(routes[j]);
					}
				}
			}
		}
	}
	//For testing purposes
	/*
	for (int k = 0; k < signed(conTable.size()); ++k) {
			cout << "src: " << conTable[k].src
				 << " dest: " << conTable[k].dest
				 << " cost: " << conTable[k].cost
				 << " destUDP: " << conTable[k].destUDP << endl;
	}
	*/
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
//			cout << "child PID: " << getpid() << endl;
			char *argv[1000];
			memset(&argv, 0, sizeof(argv));
			argv[0] = strdup("router");	//router execution
			argv[1] = (char *) to_string(uniqRouters[i]).c_str();	//router ownAddr
			argv[2] = (char *) to_string(TCP_PORT).c_str();	//TCP port for router->manager
			argv[3] = (char *) to_string(ports[i]).c_str();	//UDP port for router->router
			argv[4] = (char *) table.c_str();	//src dest weight udpPort,src dest weight udpPort...
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
	//create "server" and open on port popped from the ports vector
	//do the system call with argv[1] being port # and make router connect to the port
	//signal(SIGINT, closeServSocks);	//needed for catching '^C'

	fd_set readfds;

	//sockaddr_in is for socket that a sstone will listen to for incoming connection
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);    //PORT is the sstones own port to listen to
	int tcpSocket;
	tcpSocket = socket(PF_INET, SOCK_STREAM, 0);    //incoming socket

	if (tcpSocket < 0) {
		printMessage("socket fail");
		perror("socket fail");
		exit(EXIT_FAILURE);
	}

	if (bind(tcpSocket, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
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

	while (1) {
		int numbytes;
		char packet[100];
		memset(&packet, 0, sizeof(packet));
		sin_size = sizeof their_addr;
		new_fd = accept(tcpSocket, (struct sockaddr *) &their_addr, &sin_size);    //socket to recieve on

		FD_ZERO(&readfds);
		FD_SET(tcpSocket, &readfds);

		if (new_fd == -1) {
			perror("new socket fail");
			exit(EXIT_FAILURE);
		}

		if ((numbytes = recv(new_fd, &packet, sizeof(packet), 0)) == -1) {
			perror("recv");
			exit(EXIT_FAILURE);
		}

		//currently router is just sending its UDP port
		cout << "message recieved was: " << packet << endl;
	}
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
	return table.substr(0, table.length()-1);
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
	thread managerRouterCreate(&Manager::routerSpinUp, &manager);	//execute n routers
	managerTCPCreate.join();
	managerRouterCreate.join();

	cout << "PID SIZE: " << manager.PIDs.size() << endl;

	manager.killProcesses();
	//system("killall manager"); //temporary to kill processes
}