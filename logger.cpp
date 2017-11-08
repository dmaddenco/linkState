//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#include "logger.h"

void Logger::printMessage(string filename, string message){
	ofstream file;
	file.open (filename,ofstream::out | ofstream::app);
	file << currentDateTime() << ": " << message << "\n";
	file.close();
	
}

const string Logger::currentDateTime() {
//adapted from: https://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    return buf;
}

int main(int argc, char *argv[]) {
	

}