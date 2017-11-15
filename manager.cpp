//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#include "manager.h"

/*
 * Reads in file, creates vector of unique routers,
 * and vector of Route structs that contain (src dest cost)
 */
void Manager::readFile(ifstream &inFile) {
	string line;

	if (inFile.fail()) {
		cerr << "Error: Failed to open file." << endl;
		exit(1);
	}

	getline(inFile, line);
	int numRouters = stoi(line);

	while (getline(inFile, line) && line != "-1") {
		vector<string> strs;
		boost::split(strs,line,boost::is_any_of(" "));
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
	for (int i = 0; i < uniqRouters.size(); ++i) {
		Router router;
		int src = uniqRouters[i];
		router.ownAddr = src;
		for (int j = 0; j < routes.size(); ++j) {
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
	cout << "parent PID: " << getpid() << endl;
	pid_t childPid;
	int portIndex = 0;
	for (int i = 0; i < uniqRouters.size(); ++i) {
		childPid = fork();
		if (!childPid) {
			cout << "child PID: " << getpid() << endl;
			//####
			// char syscall[100];
			// char buf[100];
			// sprintf(buf,"%d",getpid());
			// strcpy(syscall,"./router ");
			// strcat(syscall,buf);
			// system(syscall);
			
			//####
			//instead of comment section im thinking call establishConnection(ports.at(portIndex)); portIndex++;
			establishConnection(ports.at(portIndex)); 
			
			break;	//don't let child fork again
		}
		portIndex++;
	}
}

void Manager::createPorts(int numRouters) {
	//create distinct ports for tcp connection with router
	for(int i =0; i < numRouters; i++){
		int portNum = 2000 + (i * 100);
		ports.push_back(portNum);
	}
	
	for(int i=0; i < ports.size(); i++){
		cout << "ports[" << i << "]: " << ports.at(i) << endl;
	}
}

void Manager::establishConnection(int port) {
	cout << "port: " << port << endl;
	//create "server" and open on port popped from the ports vector
	//do the system call with argv[1] being port # and make router connect to the port
	//signal(SIGINT, closeServSocks);	//needed for catching '^C'

	//sockaddr_in is for socket that a sstone will listen to for incoming connection
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);	//PORT is the sstones own port to listen to

	sock_in = socket(PF_INET, SOCK_STREAM, 0);	//incoming socket

	if (sock_in < 0) {
		perror("socket fail");
		exit(EXIT_FAILURE);
	}

	if (bind(sock_in, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	
			char syscall[100];
			char buf[100];
			sprintf(buf,"%d",port);
			strcpy(syscall,"./router ");
			strcat(syscall,buf);
			system(syscall);

	if (listen(sock_in, MAXPENDING) < 0) {
		perror("listen failed");
		exit(EXIT_FAILURE);
	}

	//struct hostent *he;	//for getting incoming connections ip address
	//struct in_addr **addr_list;
	//char hostname[128];

	//char inIpAddress[INET6_ADDRSTRLEN];	//stores incoming connections ip address

	//gethostname(hostname, sizeof hostname);
	//he = gethostbyname(hostname);
	//addr_list = (struct in_addr **) he->h_addr_list;

	//cout << "My Ip Address: " << inet_ntoa(*addr_list[0]) << endl;
	cout << "Listening to PORT: " << ntohs(servAddr.sin_port) << endl;
}

int main(int argc, char *argv[]) {
	Manager manager;
	ifstream file(argv[1]);
	manager.readFile(file);
	manager.createRouters();
	manager.routerSpinUp();
}