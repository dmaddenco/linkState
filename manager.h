//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#ifndef LINKSTATE_MANAGER_H
#define LINKSTATE_MANAGER_H

#include "router.h"
#include "logger.h"

#include <iostream>

using std::cout;
using std::endl;
using std::cin;
using std::cerr;

#include <fstream>

using std::ifstream;

#include <string>

using std::string;
using std::to_string;

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
#include <sys/time.h>
#include <stdio.h>

#include <thread>

using std::thread;

#define MAXPENDING 10
#define TCP_PORT 5000

class Manager {
public:
	int sock_in;
	int new_fd;
	vector<int> uniqRouters;    //used for Router creation
	vector <Route> routes;    //contains Router struct of (src dest cost)
	vector <Router> routers;    //contains routers that have established conTables
	vector<int> ports;
	vector<int> PIDs;

	void readFile(ifstream &inFile);

	void createRouters();    //create routers with conTables
	void routerSpinUp();    //fork processes
	void establishConnection(int port); //create tcp connection to router
	void createPorts(int numRouters);

	void printMessage(string message);

	const string currentDateTime();

	void killProcesses();

};

#endif //LINKSTATE_MANAGER_H
