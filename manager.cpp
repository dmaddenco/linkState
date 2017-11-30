//
// Created by David Madden, Collin Kinchen on 11/7/17.
//

#include "manager.h"

/*
 * Reads in file, creates vector of unique routers,
 * and vector of Route structs that contain (src dest cost)
 */
void Manager::readFile(ifstream &inFile) {
	printMessage("STARTING METHOD: readFile()");
	string line;

	if (inFile.fail()) {
		printMessage("Error: Failed to open file.");
		cerr << "Error: Failed to open file." << endl;
		exit(1);
	}

	getline(inFile, line);
	int numRouters = stoi(line);

	while (getline(inFile, line) && line != "-1") {
		vector <string> strs;
		boost::split(strs, line, boost::is_any_of(" "));
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

	createPorts(numRouters);
}

/*
 * Create routers based on number of unique routers.
 * Routers will have their own address (ownAddr) and a
 * vector of Route structs for a connection table (conTable)
 */
void Manager::createRouters() {
	printMessage("STARTING METHOD: createRouters()");

	for (int i = 0; i < signed(uniqRouters.size()); ++i) {
//		Router router;
		int src = uniqRouters[i];
		for (int j = 0; j < signed(routes.size()); ++j) {
			if (routes[j].src == src) {
				for (int k = 0; k < signed(uniqRouters.size()); ++k) {
					if (routes[j].dest == uniqRouters[k]) {
						routes[j].destUDP = ports[k];
						conTable.push_back(routes[j]);
					}
				}
			}
		}
	}
	printMessage("Current Connection Table:");
	for (int k = 0; k < signed(conTable.size()); ++k) {
			stringstream ss;
			ss << "src: " << conTable[k].src
				 << " dest: " << conTable[k].dest
				 << " cost: " << conTable[k].cost
				 << " destUDP: " << conTable[k].destUDP;
	//		cout << ss.str() << endl; //For testing purposes
			printMessage(ss.str());

	}
}

/*
 * Fork n number of processes for n routers.
 * Execute router executable
 */
void Manager::routerSpinUp() {
	printMessage("STARTING METHOD: routerSpinUp()");
//    pid_t pid = getpid();
	printMessage("Parent PID: " + to_string(getpid()));
	pid_t childPid;
	int portIndex = 0;
//	int status;
	for (int i = 0; i < signed(uniqRouters.size()); ++i) {
		string table = conTableString(uniqRouters[i], conTable);
		childPid = fork();
		PIDs.push_back(childPid);
		if (!childPid) {
//			cout << "child PID: " << getpid() << endl;
			char *argv[1000];
			memset(&argv, 0, sizeof(argv));
			argv[0] = strdup("router");	//router execution
			argv[1] = (char *) to_string(uniqRouters[i]).c_str();	//router ownAddr
			argv[2] = (char *) to_string(TCP_PORT).c_str();	//TCP port for router->manager
			argv[3] = (char *) to_string(ports[i]).c_str();	//UDP port for router->router
			argv[4] = (char *) table.c_str();	//src dest weight udpPort,src dest weight udpPort...
			stringstream ss;
			ss << "STARTING Router: " << uniqRouters[i] << " for port " << ports[i];
			cout << ss.str() << endl;
			printMessage(ss.str());
			execv(argv[0], argv);
			break;    //don't let child fork again
		} else if (childPid > 0) {
//            wait(&status);	//wait until child executes
        }
		portIndex++;
	}
}

/*
 * Create n UDP port numbers for routers for
 * router->router communication
 */
void Manager::createPorts(int numRouters) {
	printMessage("STARTING METHOD: createPorts()");

	//create distinct ports for tcp connection with router
	for (int i = 0; i < numRouters; i++) {
		int portNum = 2000 + (i * 100);
		ports.push_back(portNum);
	}
}

/*
 * Create TCP connection that will accept from any address
 */
void Manager::establishConnection(int port) {
	printMessage("STARTING METHOD: establishConnection()");
	cout << "port: " << port << endl;

	//sockaddr_in is for socket that a sstone will listen to for incoming connection
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);    //PORT is the sstones own port to listen to
	int tcpSocket, routerTCPsocket;
	tcpSocket = socket(PF_INET, SOCK_STREAM, 0);    //incoming socket

	if (tcpSocket < 0) {
		printMessage("socket fail");
		perror("socket fail");
		exit(EXIT_FAILURE);
	}

	if (bind(tcpSocket, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
		printMessage("bind failed");
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(tcpSocket, MAXPENDING) < 0) {
		printMessage("listen failed");
		perror("listen failed");
		exit(EXIT_FAILURE);
	}

	cout << "Listening to PORT: " << ntohs(servAddr.sin_port) << endl;
	printMessage("Listening to PORT: " + to_string(port));

	sockaddr_in their_addr;    //for connecting to incoming connections socket
	socklen_t sin_size = sizeof(their_addr);

	fd_set readfds;	// master file descriptor list
//	int sd, n, sv;
	int n, sv;

	while (1) {
//	while (routerTcpSockets.size() != uniqRouters.size()) {
//		int numbytes;
		char packet[100];
		memset(&packet, 0, sizeof(packet));

//		sin_size = sizeof their_addr;
//		new_fd = accept(tcpSocket, (struct sockaddr *) &their_addr, &sin_size);    //socket to recieve on

		FD_ZERO(&readfds);
		FD_SET(tcpSocket, &readfds);

		n = tcpSocket + 1;
		sv = select(n, &readfds, NULL, NULL, NULL);

		if (sv == -1) {
			perror("select");
		} else {
			// one or both of the descriptors have data
			if (FD_ISSET(tcpSocket, &readfds)) {
				int recvd = -1;

				if ((routerTCPsocket = accept(tcpSocket,(struct sockaddr *) &their_addr, &sin_size))<0){
					perror("accept");
					exit(1);
				}

				recvd = recv(routerTCPsocket, packet, sizeof(packet), 0);

				if (recvd < 0) {
					fprintf(stderr, "Issue with recv \n");
					printf("errno %d", errno);
					exit(EXIT_FAILURE);
				}

				stringstream ss;
				ss << "Message recieved was: " << packet;
				printMessage(ss.str());
				cout << ss.str() << endl;
				routerTcpSockets.push_back(routerTCPsocket);

				if (routerTcpSockets.size() == uniqRouters.size()) {
					ss.str("");
					ss << "All routers are ready.";
					printMessage(ss.str());
					cout << ss.str() << endl;
					for (int i = 0; i < routerTcpSockets.size(); ++i) {
						char msg[100];
						ss.str("");
						ss << "Sending START_LS_ACK";
						printMessage(ss.str());
						ss.str("");
						ss << "START_LS_ACK " << conTable.size();
						strcpy(msg, ss.str().c_str());
						send(routerTcpSockets[i], &msg, sizeof(msg), 0);
					}
					bool lsDone = false;
					vector<int> responses;
//					while (routerTcpSockets.size() == uniqRouters.size()) {
					/*
					 * This while loop will loop over all routerTcpSockets that were created above.
					 * Once a response has been heard from all unique routers, the while loop exits.
					 */
					while (!lsDone) {
						for (int i = 0; i < signed(routerTcpSockets.size()); ++i) {
							memset(&packet, 0, sizeof(packet));
							recvd = recv(routerTcpSockets[i], packet, sizeof(packet), 0);

							if (recvd < 0) {
								fprintf(stderr, "Issue with recv \n");
								printf("errno %d", errno);
								exit(EXIT_FAILURE);
							}

							vector <string> r;
							boost::split(r, packet, boost::is_any_of(" "));

							stringstream ss;
							ss << "Message recieved was: " << r[0] << " from Router: " << r[1];
							printMessage(ss.str());
							cout << ss.str() << endl;
							bool contains = false;
							int router = stoi(r[1]);
							for (int j = 0; j < signed(responses.size()); ++j) {
								if (router == responses[j]) {
									contains = true;
								}
							}
							if (!contains) {
								responses.push_back(router);
							}
							if (responses.size() == uniqRouters.size()) {
								printMessage("All routers finished LS");
								lsDone = true;
							}
						}
					}
				}
			}
		}
	}
}

void Manager::printMessage(string message) {
	ofstream file;
	string filename = "manager.out";
	file.open(filename, ofstream::out | ofstream::app);
	file << currentDateTime() << ": " << message << "\n";
	file.close();
}

const string Manager::currentDateTime() {
//adapted from: https://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	return buf;
}

void Manager::killProcesses() {
	for (int i = 0; i < signed(PIDs.size()); ++i) {
		cout << "PIDs.at(" << i << "): " << PIDs.at(i) << endl;
		char syscall[100];
		char buf[100];
		sprintf(buf, "%d", PIDs.at(i));
		strcpy(syscall, "kill -9 ");
		strcat(syscall, buf);
		system(syscall);
		printMessage("RUNNING COMMAND: " + string(syscall));
	}
}

string Manager::conTableString(int src, vector <Route> routes) {
	stringstream ss;
	for (int i = 0; i < signed(routes.size()); ++i) {
		if (src == routes[i].src) {
			ss << routes[i].src << " " << routes[i].dest << " " << routes[i].cost << " " << routes[i].destUDP << ",";
		}
	}
	string table = ss.str();
	return table.substr(0, table.length()-1);
}

int main(int argc, char *argv[]) {
	Manager manager;

	manager.printMessage("STARTING MANAGER######################################");

	ifstream file(argv[1]);

	manager.readFile(file);
	manager.createRouters();

	//will first create TCP connection and wait for incomming connections
	//needed first, so routers have something to connect to
	thread managerTCPCreate(&Manager::establishConnection, &manager, TCP_PORT);
	thread managerRouterCreate(&Manager::routerSpinUp, &manager);	//execute n routers
	managerTCPCreate.join();
	managerRouterCreate.join();

	cout << "PID SIZE: " << manager.PIDs.size() << endl;

	manager.killProcesses();
	//system("killall manager"); //temporary to kill processes
}

/* THIS IS EVREYTHING NEEDED FOR Dijkstra’s

#include<bits/stdc++.h>
using namespace std;
# define INF 0x3f3f3f3f


struct SPT {    //Used to store hop info after Djikstras
	int src;    
	int dest;
	int hop;
};

vector<SPT> ShortPathTree; //Used to store hop info after Djikstras
void printSPT(int ownAddr);

#######THIS WAS IN MAIN#####################
	int V = 10; //10 is just the number or routers to run it on
    Graph g(V);
	
	for (int i = 0; i < signed(manager.conTable.size()); ++i) {
		g.addEdge(manager.conTable[i].src,manager.conTable[i].dest,manager.conTable[i].cost);
	}
	
	for(int i = 0; i < V; i++){
		vector<SPT> temp = g.shortestPath(i);
		for (int i = 0; i < signed(temp.size()); ++i) {
		manager.ShortPathTree.push_back(temp[i]);
		}

	}
		for(int i = 0; i < V; i++){
		cout << "SPT FOR ROUTER " << i << endl;
		manager.printSPT(i);                      //when in router only need to do this for the specific router you want the tree from
		}
###############################################

class Graph
{
    int V;    // No. of vertices
 
    // In a weighted graph, we need to store vertex 
    // and weight pair for every edge
    list< pair<int, int> > *adj;
 
public:
    Graph(int V);  // Constructor
 
    // function to add an edge to graph
    void addEdge(int u, int v, int w);
 
    // prints shortest path from s
    vector<SPT> shortestPath(int s);
};
 
// Allocates memory for adjacency list
Graph::Graph(int V)
{
    this->V = V;
    adj = new list< pair<int, int> >[V];
}
 
void Graph::addEdge(int u, int v, int w)
{
    adj[u].push_back(make_pair(v, w));
    adj[v].push_back(make_pair(u, w));
}
 
// Prints shortest paths from src to all other vertices
vector<SPT> Graph::shortestPath(int src)
{
	vector<SPT> nextHops;
    // Create a set to store vertices that are being
    // prerocessed
    set< pair<int, int> > setds;
	int parent[V];
 
    // Create a vector for distances and initialize all
    // distances as infinite (INF)
    vector<int> dist(V, INF);
 
    // Insert source itself in Set and initialize its
    // distance as 0.
    setds.insert(make_pair(0, src));
    dist[src] = 0;
 
    /* Looping till all shortest distance are finalized
       then setds will become empty 
    while (!setds.empty())
    {
        // The first vertex in Set is the minimum distance
        // vertex, extract it from set.
        pair<int, int> tmp = *(setds.begin());
        setds.erase(setds.begin());
 
        // vertex label is stored in second of pair (it
        // has to be done this way to keep the vertices
        // sorted distance (distance must be first item
        // in pair)
        int u = tmp.second;
 
        // 'i' is used to get all adjacent vertices of a vertex
        list< pair<int, int> >::iterator i;
        for (i = adj[u].begin(); i != adj[u].end(); ++i)
        {
            // Get vertex label and weight of current adjacent
            // of u.
            int v = (*i).first;
            int weight = (*i).second;
 
            //  If there is shorter path to v through u.
            if (dist[v] > dist[u] + weight)
            {
                /*  If distance of v is not INF then it must be in
                    our set, so removing it and inserting again
                    with updated less distance.  
                    Note : We extract only those vertices from Set
                    for which distance is finalized. So for them, 
                    we would never reach here.  
                if (dist[v] != INF)
                    setds.erase(setds.find(make_pair(dist[v], v)));
 
                // Updating distance of v
                dist[v] = dist[u] + weight;
				parent[v] = u;
                setds.insert(make_pair(dist[v], v));
            }
        }
    }
 
    // Print shortest distances stored in dist[]
    //printf("Vertex\t   Distance\tPath\n");
    for (int i = 0; i < V; ++i){
		SPT spt;
			spt.src = i;
			spt.dest = src;
			spt.hop = parent[i];
		if (i == src){spt.hop = -1;}
		nextHops.push_back(spt);
       // printf("%d \t\t %d\t\t%d\n", i, dist[i],parent[i]);
	}

	return nextHops;
}

	void Manager::printSPT(int ownAddr){
		stringstream ss;
		ss.str("");
		//go through all shortest paths
		for (int i = 0; i < signed(ShortPathTree.size()); ++i) {
			//find the ones for the table you want
			if (ShortPathTree[i].dest == ownAddr && ShortPathTree[i].hop == -1){
				ss << "( " << ShortPathTree[i].dest << " , 0 , " << ShortPathTree[i].dest << " )\n";
			}
			else if (ShortPathTree[i].src == ownAddr){
				ss << "( " << ShortPathTree[i].dest << " , ";
				for (int k = 0; k < signed(conTable.size()); ++k) { 
					if ((conTable[k].src == ShortPathTree[i].src && conTable[k].dest == ShortPathTree[i].hop) || (conTable[k].dest == ShortPathTree[i].src && conTable[k].src == ShortPathTree[i].hop)){
						ss << conTable[k].cost << " , ";
					}

				}	
				ss << ShortPathTree[i].hop << " )\n";
				
			}
		}
		cout << ss.str() << endl;
		ss.str("");
	}
		


*/