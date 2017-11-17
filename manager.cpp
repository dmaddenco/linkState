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
		Router router;
		int src = uniqRouters[i];
		router.ownAddr = src;
		for (int j = 0; j < signed(routes.size()); ++j) {
			if (routes[j].src == src) {
				router.conTable.push_back(routes[j]);
			}
		}
		routers.push_back(router);
	}
	//For testing purposes
	/*
	for (int k = 0; k < routers.size(); ++k) {
		for (int i = 0; i < routers[k].conTable.size(); ++i) {
			cout << "src: " << routers[k].conTable[i].src << " dest: " << routers[k].conTable[i].dest << " cost: " << routers[k].conTable[i].cost << endl;
		}
	}
	 */
}

/*
 * Fork n number of processes for n routers
 */
void Manager::routerSpinUp() {
	printMessage("STARTING METHOD: routerSpinUp()");

	printMessage("Parent PID: " + to_string(getpid()));
	pid_t childPid;
	int portIndex = 0;
	int status;
	for (int i = 0; i < signed(uniqRouters.size()); ++i) {
		childPid = fork();
		PIDs.push_back(childPid);
		if (!childPid) {
			cout << "child PID: " << getpid() << endl;
			//instead of comment section im thinking call establishConnection(ports.at(portIndex)); portIndex++;
//			establishConnection(ports.at(portIndex));
			char *argv[1000];
			argv[0] = strdup("router");
			argv[1] = (char *) to_string(TCP_PORT).c_str();
			argv[2] = (char *) to_string(ports[i]).c_str();
			printMessage("STARTING Router for port " + to_string(ports[i]));
			execv(argv[0], argv);
//			break;    //don't let child fork again
		} else if (childPid > 0) {
			wait(&status);
		}
		portIndex++;
	}
}

void Manager::createPorts(int numRouters) {
	printMessage("STARTING METHOD: createPorts()");

	//create distinct ports for tcp connection with router
	for (int i = 0; i < numRouters; i++) {
		int portNum = 2000 + (i * 100);
		ports.push_back(portNum);
	}
}

void Manager::establishConnection(int port) {
	printMessage("STARTING METHOD: establishConnection()");
	vector<int> udpPorts;
	cout << "port: " << port << endl;
	//create "server" and open on port popped from the ports vector
	//do the system call with argv[1] being port # and make router connect to the port
	//signal(SIGINT, closeServSocks);	//needed for catching '^C'

	//sockaddr_in is for socket that a sstone will listen to for incoming connection
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);    //PORT is the sstones own port to listen to

	sock_in = socket(PF_INET, SOCK_STREAM, 0);    //incoming socket

	if (sock_in < 0) {
		printMessage("socket fail");
		perror("socket fail");
		exit(EXIT_FAILURE);
	}

	if (bind(sock_in, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		printMessage("bind failed");
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(sock_in, MAXPENDING) < 0) {
		printMessage("listen failed");
		perror("listen failed");
		exit(EXIT_FAILURE);
	}

	cout << "Listening to PORT: " << ntohs(servAddr.sin_port) << endl;
	printMessage("Listening to PORT: " + to_string(port));

	sockaddr_in their_addr;    //for connecting to incoming connections socket
	socklen_t sin_size = sizeof(their_addr);

	while (1) {
		sin_size = sizeof their_addr;
		new_fd = accept(sock_in, (struct sockaddr *) &their_addr, &sin_size);    //socket to recieve on

		if (new_fd == -1) {
			perror("new socket fail");
			exit(EXIT_FAILURE);
		}

		int numbytes;

		// inet_ntop(their_addr.sin_family,
		// get_in_addr((struct sockaddr *) &their_addr),
		// inIpAddress, sizeof inIpAddress);

		char packet[100];

		if ((numbytes = recv(new_fd, &packet, sizeof(packet), 0)) == -1) {
			perror("recv");
			exit(EXIT_FAILURE);
		}

		cout << "message recieved was: " << packet << endl;
		udpPorts.push_back(atoi(packet));
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

int main(int argc, char *argv[]) {


	Manager manager;
	manager.printMessage("STARTING MANAGER######################################");
	ifstream file(argv[1]);
	manager.readFile(file);
	manager.createRouters();
	manager.routerSpinUp();
	cout << "PID SIZE: " << manager.PIDs.size() << endl;
	manager.killProcesses();
	//system("killall manager"); //temporary to kill processes
}