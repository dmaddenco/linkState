//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#include "manager.h"

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
}

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
}

void Manager::routerSpinUp() {
	cout << "parent PID: " << getpid() << endl;
	pid_t pid;
//	for (int i = 0; i < uniqRouters.size(); ++i) {
//		pid = fork();
//		if (pid) {
//			cout << "child PID #" << i << ": " << getpid() << endl;
//			continue;
//		} else if (pid == 0) {
//			break;
//		} else {
//			printf("fork error\n");
//			exit(1);
//		}
//	}
}

int main(int argc, char *argv[]) {
	Manager manager;
	ifstream file(argv[1]);
	manager.readFile(file);
	manager.createRouters();
	manager.routerSpinUp();
}