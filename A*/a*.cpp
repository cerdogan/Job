/**
 * @file a*.cpp
 * @author Can Erdogan
 * @date July 12, 2015
 * @brief Implementation of a* search for a Bucharest map (see data.png). 
 * book. Start state: Oradea. Goal state: Bucharest.
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

using namespace std;

/* ******************************************************************************************** */
struct State {
	map <string, State*> neighs;
	map <string, double> neighDists;
	double euclideanDist;
};

map <string, State> states;

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
		s1.neighs[city2] = &(states[city2]);
		s1.neighDists[city2] = dist;

		// Add the information for the second city
		State& s2 = states[city2];
		s2.neighs[city1] = &(states[city1]);
		s2.neighDists[city1] = dist;
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
void astar () {

}

/* ******************************************************************************************** */
int main (int argc, char* argv[]) {

	// Get the data for the problem
	parseData();
}
/* ******************************************************************************************** */
