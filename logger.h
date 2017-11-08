//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#ifndef LINKSTATE_LOGGER_H
#define LINKSTATE_LOGGER_H

#include <iostream>
using std::ofstream;
using std::cout;
using std::endl;

#include <fstream>

using std::ifstream;
#include <cstdlib>
#include <fstream>

using std::system;

#include <sys/time.h>
#include <stdio.h>
#include <string.h>

using std::string;

class Logger{
public:
	void printMessage(string filename, string message);
	const string currentDateTime();
	
};
#endif //LINKSTATE_LOGGER_H
