//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#ifndef LINKSTATE_ROUTER_H
#define LINKSTATE_ROUTER_H

#include <iostream>

using std::cout;
using std::endl;
using std::cin;
using std::cerr;

#include <vector>

using std::vector;

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>

using std::string;
using std::to_string;
using std::to_string;

#include <iostream>
using std::ofstream;
using std::cout;
using std::endl;
#include <fstream>
#include <cstdlib>

#include <sstream>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using std::stringstream;

struct Route {    //will be used for conTable construction
	int src;    //ownAddr
	int dest;
	int cost;
	int destUDP;
};

class Router {
public:
	vector <Route> conTable;
	vector <Route> tempconTable;
	vector <int> udpPorts;
	string filename = "router";

	void printMessage(string message);

	const string currentDateTime();

	void createFileName(char *argv1);

	void client();
	vector<Route> createConTable(string table);
	string compressConTable();
	void compare();
	void createUdpVector();
};

#endif //LINKSTATE_ROUTER_H
