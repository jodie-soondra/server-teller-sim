#include <iostream>
#include <fstream>

using namespace std;

//data structs
struct Server {
	bool idle;								//true or false to store is server busy
	short unsigned int customersServed;		//counts the number of customers served
	float serviceEndAt;						//stores the time that the current/last service will end at
	float idleTime;							//stores total time that the srver has been idle
};

struct Customer {
	float arrivalTime;						//struct stores data read from sample time
	float serviceDuration;
	short unsigned int priority;
};

Customer queue[100];						//the two heaps
Server servers[4];

short int queueLength = -1;					//variables to initialise and track the simulation
short unsigned int maxQueueLength = 0;
short unsigned int totalCustomers = 0;
short unsigned int numServers = 0;

float simTime = 0.0000;
float totalServiceTime = 0.0000;
float totalWaitTime = 0.0000;

float nextArrival = 0.0000;
float nextService = 0.0000;
short unsigned int nextPriority;


void processArrival();
void processService();
void serverSiftDown(int, int);
void queueSiftUp(int, int);
void queueSiftDown(int, int);
void custSwap(int, int);
void serverSwap(int, int);


int main() {
	char fileName[20];
	ifstream inFile;

	cout << "Please enter the number of servers for the simulation (integer 1 - 4): ";
	cin >> numServers;

	cout << "Please enter the name of the file: ";
	cin >> fileName;
	inFile.open(fileName);

	//initialise servers
	for (int i = 0; i < numServers; i++) {
		servers[i].idle = true;
		servers[i].customersServed = 0;
		servers[i].serviceEndAt = 0.0000;
		servers[i].idleTime = 0.0000;
	}

	if (inFile.is_open()) {
		while (!inFile.eof()) {
			//read in next arrival	
			if (!inFile.eof()) {
				inFile >> nextArrival >> nextService >> nextPriority;
				totalServiceTime += nextService;
				totalCustomers++;
				cout << "\n\nReading customer: " << nextArrival << " : " << nextService << " : " << nextPriority << endl;
			}
			
			//main loop
			//if the server is busy, check for current customer
			//continue processing customers in the queue until the next arrival
			//then process arrival and continue
			if (!servers[0].idle) {
				if (nextArrival != 0) {
					while (!servers[0].idle && servers[0].serviceEndAt < nextArrival) {
						processService();
					}
					processArrival();
				}
				else {
					while (queueLength > 0) {
						processService();
					}
				}	
			}
			else{
				processArrival();
			}

		}

		//serve last customers-0
		for (int i = numServers - 1; i >= 0; i--) {
			//heapsort basically, swap last and first element, decrease size of heap and restore heap property
			if (!servers[i].idle) {
				processService();
			}
			simTime = servers[0].serviceEndAt;
			serverSwap(0, i);
			serverSiftDown(0, i - 1);
		}

		for (int i = 0; i < numServers; i++) {
			//print out server stats
			cout << "--------------------------------------------------------" << endl;
			cout << "Server #" << i+1 << endl;
			cout << "Customers Served: " << servers[i].customersServed << endl;
			cout << "Idle time: " << servers[i].idleTime << endl;
			cout << "Idle rate (total idle time / total simulation time): " << servers[i].idleTime / simTime << endl;
			cout << "--------------------------------------------------------" << endl;

		}

		//output results
		cout << "Total simulation time: " << simTime << endl;
		cout << "Total customers: " << totalCustomers-1 << endl;

		cout << "Average service time per customer: " << totalServiceTime / (totalCustomers-1) << endl;
		cout << "Average wait time per customer: " << totalWaitTime / totalCustomers << endl;
		cout << "Maximum length of queue: " << maxQueueLength+1 << endl;
		cout << "Average length of queue (total queuing time / time last service completed): " << totalWaitTime / servers[0].serviceEndAt << endl;
	}
	else {
		cout << "[ FILE ERROR ]\n";
		return -1;
	}

	return 0;
}

void processArrival() {
	//time = next arrival
	if (nextArrival == 0) {
		return;
	}
	simTime = nextArrival;

	cout << "\n\nProcessing ARRIVAL, sim time: " << simTime << endl;

	//check for busy, if not, serve next customer in queue
	if (servers[0].idle) {

		cout << "Server idle, serving customer: " << nextArrival << " : " << nextService << " : " << nextPriority << endl;

		//update server idle time
		servers[0].idleTime += (simTime - servers[0].serviceEndAt);
		//set server to busy
		servers[0].idle = false;
		//update service end time
		servers[0].serviceEndAt = simTime + nextService;
		//increase numcust served
		servers[0].customersServed++;

		cout << "\nUpdated .... " << endl;
		cout << "servers[0].idleTime: " << servers[0].idleTime << endl;
		cout << "Service end: " << servers[0].serviceEndAt << endl;
		cout << "Customers served: " << servers[0].customersServed << endl;

		//update server heap so next server to be finished will become new root
		serverSiftDown(0, numServers - 1);
	}
	else {
		queueLength++;
		//enter into queue i.e create customer
		queue[queueLength].arrivalTime = nextArrival;
		queue[queueLength].serviceDuration = nextService;
		queue[queueLength].priority = nextPriority;
		cout << "No servers free, entering queue: " << queue[queueLength].arrivalTime << " : " << queue[queueLength].serviceDuration << " : " << queue[queueLength].priority << endl;
		//else server is busy and customer enters queue
		queueSiftUp(queueLength, queueLength);
		//check for max queue length
		if (queueLength > maxQueueLength) {
			maxQueueLength = queueLength;
		}

	}
}

void processService() {
	//time = service end
	simTime = servers[0].serviceEndAt;
	cout << "\nProcess SERVICE, sim time: " << simTime << endl;

	//if queue empty, idle = true
	if (queueLength < 0) {
		cout << "Server now idle, service ended at: " << servers[0].serviceEndAt << endl;
		servers[0].idle = true;
	}
	//else, serve next customer
	else{
		//update customer's total wait time
		totalWaitTime += (simTime - queue[0].arrivalTime);

		//set them to busy
		servers[0].idle = false;
		//update service end
		servers[0].serviceEndAt = simTime + queue[0].serviceDuration;
		//increase num customers served
		servers[0].customersServed++;

		cout << "Serving customer: " << queue[0].arrivalTime << " : " << queue[0].serviceDuration << " : " << queue[0].priority << endl;
		cout << "Updated .... " << endl;
		cout << "Idle time: " << servers[0].idleTime << endl;
		cout << "Service end: " << servers[0].serviceEndAt << endl;
		cout << "Customers served: " << servers[0].customersServed << endl;

		//update server heap so next server to be finished will become new root
		serverSiftDown(0, numServers - 1);

		//fix queue, so swap first and last, siftdown, then queue--
		custSwap(0, queueLength);
		queueLength--;
		queueSiftDown(0, queueLength);
	}
}

//min heap based on service end time
void serverSiftDown(int current, int heapSize) {
	int child = 2 * current + 1;

	if (child > heapSize) {    //if at leaf, return
		return;
	}

	//check right child is within array bounds
	//check left child < right, compare the smaller with parent
	if (child + 1 < heapSize && servers[child].serviceEndAt >= servers[child + 1].serviceEndAt) {
		child++;
	}

	if (servers[current].serviceEndAt >= servers[child].serviceEndAt) {
		//swap
		serverSwap(current, child);
		//siftDown
		serverSiftDown(child, heapSize);
	}

	return;
}


//max heap based on priority and secondarily arrival time (in ascending --> min heap)
void queueSiftUp(int current, int heapSize) {
	int child = current / 2;

	if (child < 0 || child == current) {    //if at root, return
		return;
	}

	//check right child is within array bounds
	//check left child < right, compare the smaller with parent
	if (child + 1 < heapSize && queue[child].priority > queue[child + 1].priority) {
		child++;
	}

	//check if children bigger than parent
	//if yes, swap and then sift down recursively
	//if they are equal, swap 
	if (queue[current].priority > queue[child].priority) {
		//swap
		custSwap(current, child);
		//siftDown
		queueSiftUp(child, heapSize);
	}
	return;

}

void queueSiftDown(int current, int heapSize) {
	int child = current * 2 + 1;


	if (child > heapSize) {    //if at root, return
		return;
	}

	//check right child is within array bounds
	//check left child < right, compare the smaller with parent
	if (child + 1 < heapSize && queue[child].priority < queue[child + 1].priority) {
		child++;
	}

	//check if children bigger than parent
	//if yes, swap and then sift down recursively
	//if they are equal, swap 
	if (queue[current].priority <= queue[child].priority) {
		//swap
		custSwap(current, child);
		//siftDown
		queueSiftDown(child, heapSize);
	}

	return;
}


//swaps servers for sift function
void serverSwap(int current, int child) {
	bool tempIdle;
	short unsigned int tempServed;
	float tempService, tempIdleTime;

	tempIdle = servers[current].idle;
	tempServed = servers[current].customersServed;
	tempService = servers[current].serviceEndAt;
	tempIdleTime = servers[current].idleTime;

	servers[current].idle = servers[child].idle;
	servers[current].customersServed = servers[child].customersServed;
	servers[current].serviceEndAt = servers[child].serviceEndAt;
	servers[current].idleTime = servers[child].idleTime;

	servers[child].idle = tempIdle;
	servers[child].customersServed = tempServed;
	servers[child].serviceEndAt = tempService;
	servers[child].idleTime = tempIdleTime;
}

//swaps customers for sift functions
void custSwap(int current, int child) {
	float tempArrival, tempService;
	short unsigned int tempPriority;

	tempArrival = queue[current].arrivalTime;
	tempPriority = queue[current].priority;
	tempService = queue[current].serviceDuration;

	queue[current].arrivalTime = queue[child].arrivalTime;;
	queue[current].priority = queue[child].priority;;
	queue[current].serviceDuration = queue[child].serviceDuration;;

	queue[child].arrivalTime = tempArrival;
	queue[child].priority = tempPriority;
	queue[child].serviceDuration = tempService;
}