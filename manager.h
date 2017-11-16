//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#ifndef LINKSTATE_MANAGER_H
#define LINKSTATE_MANAGER_H

#include "router.h"

#include <iostream>

using std::cout;
using std::endl;
using std::cin;
using std::cerr;

#include <fstream>

using std::ifstream;

#include <string>

using std::string;

#include <vector>

using std::vector;
using std::find;

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <cstdlib>

using std::system;

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXPENDING 10

class Manager {
public:
	int sock_in;
	int new_fd;
	vector<int> uniqRouters;	//used for Router creation
	vector<Route> routes;	//contains Router struct of (src dest cost)
	vector<Router> routers;	//contains routers that have established conTables
	vector<int> ports;

	void readFile(ifstream &inFile);
	void createRouters();	//create routers with conTables
	void routerSpinUp();	//fork processes
	void establishConnection(int port); //create tcp connection to router
	void createPorts(int numRouters);
	
};

#endif //LINKSTATE_MANAGER_H
