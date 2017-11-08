//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#include "manager.h"

void Manager::readFile(ifstream &inFile) {
	string line;
	getline(inFile, line);
	int numRouters = stoi(line);
	while (getline(inFile, line) && routers.size() != numRouters) {
		vector<string> strs;
		boost::split(strs,line,boost::is_any_of(" "));
		if (find(routers.begin(), routers.end(), stoi(strs[0])) == routers.end()) {
			routers.push_back(stoi(strs[0]));
		}
		if (find(routers.begin(), routers.end(), stoi(strs[1])) == routers.end()) {
			routers.push_back(stoi(strs[1]));
		}
	}
}

int main(int argc, char *argv[]) {
	Manager manager;
	ifstream file(argv[1]);
	manager.readFile(file);
}