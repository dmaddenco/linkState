//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#ifndef LINKSTATE_ROUTER_H
#define LINKSTATE_ROUTER_H

#include "logger.h"

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

#include <iostream>
#include <fstream>
#include <cstdlib>

struct Route {	//will be used for conTable construction
	int src;	//ownAddr
	int dest;
	int cost;
};

class Router {
public:
	int ownAddr;
	string filename = "router";;
	vector< Route > conTable;
	void client(char* port);
	void printMessage(string message);
	const string currentDateTime();
	void createFileName(char* argv1);
};

#endif //LINKSTATE_ROUTER_H
