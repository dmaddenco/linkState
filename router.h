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

#include<bits/stdc++.h>
using namespace std;
# define INF 0x3f3f3f3f

using std::stringstream;

struct Route {    //will be used for conTable construction
	int src;    //ownAddr
	int dest;
	int cost;
	int destUDP;
};

struct Message {
	char table[1000];
	int srcUDP;
};

struct SPT {    //Used to store hop info after Djikstras
	int src;    
	int dest;
	int hop;
};

struct spTable{ 
	int dest;
	int cost;
	int nextHop;
};

struct Path {
	int src;
	int nextHop;
	int desireDest;
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
	
	vector<SPT> ShortPathTree; //Used to store hop info after Djikstras
	vector<spTable> finSPTable; //final Shortest Path Tree for router
	void printSPT(int ownAddr);
	list< pair<int, int> > *adj;
	int V;
	void graph(int V);
	void addEdge(int u, int v, int w);
	vector<SPT> shortestPath(int src);

	void client();
	vector<Route> createConTable(string table);
	string compressConTable();
	void compare();
	void createUdpVector();
	bool startLinkState(int expectedConTableSize);
	void sendLSFinish();
	void sendDIJKSTRAFinish();
};

#endif //LINKSTATE_ROUTER_H
