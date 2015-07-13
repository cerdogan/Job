/**
 * @file a*.cpp
 * @author Can Erdogan
 * @date July 12, 2015
 * @brief Implementation of a* search for a Bucharest map (see data.png). 
 * book. Start state: Arad. Goal state: Bucharest.
 */

#include <assert.h>
#include <fstream>
#include <iostream>
#include <math.h>
#include <map>
#include <set>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <queue>

using namespace std;

/* ******************************************************************************************** */
struct State {
	string name;
	set <State*> neighs;
	map <State*, double> neighDists;
	double totalEstimatedCost;			// f
	double costFromStart;						// g
	double euclideanDist; 					// h
	State* prev;
};

map <string, State> states;

/* ******************************************************************************************** */
/// Comparison function for the priority queue
class CompareState {
  bool reverse;
public:
  CompareState (const bool& revparam=1) {reverse=revparam;}
  bool operator() (State*& lhs, State*& rhs) const {
    if (reverse) return (lhs->totalEstimatedCost>rhs->totalEstimatedCost);
    else return (lhs->totalEstimatedCost<rhs->totalEstimatedCost);
  }
};

/* ******************************************************************************************** */
/// Parses the edge lengths and the Euclidean distances in the map data.png.
void parseData () {

	// Open the file
	fstream file ("data.txt");
	assert(file.is_open());

	// First, read the edge lengths and create the map structure
	char line [256];
	char city1 [64], city2 [64], mapKey [256];
	double dist;
	for(size_t i = 0; i < 23; i++) {

		// Get the city names
		file.getline(line, 256);	
		sscanf(line, "%s %s %lf", city1, city2, &dist);

		// Add the information for the first city
		State& s1 = states[city1];
		s1.neighs.insert(&(states[city2]));
		s1.neighDists[(&(states[city2]))] = dist;
		s1.name = string(city1);

		// Add the information for the second city
		State& s2 = states[city2];
		s2.neighs.insert(&(states[city1]));
		s2.neighDists[(&(states[city1]))] = dist;
		s2.name = string(city2);
	}
	
	// Second, read the Euclidean distances
	file.getline(line, 256);	
	for(int i = 0; i < 20; i++) {

		// Get the city name and distance
		file.getline(line, 256);	
		sscanf(line, "%s %lf", city1, &dist);

		// Set the distance
		states[city1].euclideanDist = dist;
	}
}

/* ******************************************************************************************** */
/// Regenerates path from the search 
void printPath() {

	State* prev = &(states["Bucharest"]);
	printf("\nPath! Cost: %lf\n", prev->totalEstimatedCost);
	while(prev != NULL) {
		printf("%s\n", prev->name.c_str());
		prev = prev->prev;
	}
}

/* ******************************************************************************************** */
/// Implementation of A*
bool astar () {

	// Create the data structures to keep track of the search
	set <State*> visited;
	set <State*> in_queue;
	priority_queue <State*, vector <State*>, CompareState> queue;
	
	// Initialize data for the start state
	states["Arad"].costFromStart = 0.0;
	states["Arad"].totalEstimatedCost = 0.0 + states["Arad"].euclideanDist;
	queue.push(&states["Arad"]);
	in_queue.insert(&states["Arad"]);
	
	// Start the search
	while(!queue.empty()) {

		// Check if the top of the queue is the goal
		State* current = queue.top();
		printf("Expanding: '%s'\n", current->name.c_str());
		if(current->name.compare("Bucharest") == 0) {
			printPath();
			return true;
		}

		// Note that the state is seen
		queue.pop();
		in_queue.erase(in_queue.find(current));
		visited.insert(current);

		// Expand the search to the neighbors
		for(set <State*>::iterator n_it = current->neighs.begin(); n_it != current->neighs.end(); 
				n_it++) {
		
			// Skip if the neighbor is visited before. Assumes consistent heuristic!
			State* neigh = *n_it;
			if(visited.find(neigh) != visited.end()) continue;

			// Estimate the possible shortest cost
			double estCostFromStart = current->costFromStart + current->neighDists[neigh];
			
			// If the neighbor is not seen before or the cost can be decreased, update its standing
			if((in_queue.find(neigh) == in_queue.end()) || (estCostFromStart < neigh->costFromStart)) {

				// Update the neighbor information
				neigh->prev = current;
				neigh->costFromStart = estCostFromStart;
				neigh->totalEstimatedCost = neigh->costFromStart + neigh->euclideanDist;
				printf("\tneigh '%s' -> total cost: %lf\n", neigh->name.c_str(), neigh->totalEstimatedCost);
				
				// Add the neighbor to the queue (if already not there)
				if(in_queue.find(neigh) == in_queue.end()) {
					queue.push(neigh);
					in_queue.insert(neigh);
				}
	
			}
		}
	}

	printf("Failed.\n");
}

/* ******************************************************************************************** */
int main (int argc, char* argv[]) {

	// Get the data for the problem
	parseData();

	// Search
	astar();
}
/* ******************************************************************************************** */
