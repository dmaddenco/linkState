//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#ifndef LINKSTATE_ROUTER_H
#define LINKSTATE_ROUTER_H

#include <iostream>

using std::cout;
using std::endl;
using std::cin;

#include <vector>

using std::vector;

class Router {
public:
	int ownAddr;
	vector< vector<int> > conTable;

	int getAddr();
	void setAddr(int addr);
};

#endif //LINKSTATE_ROUTER_H
