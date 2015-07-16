/**
 * @file value.cpp
 * @author Can Erdogan
 * @date July 16, 2015
 * @brief Implementation of value iteration for a MDP based on map.txt. If the agent attempts to
 * move forward, with %80 chance it will move forward, with %10 chance left and with %10 chance 
 * right. If there is a wall in front of it, it will remain stationary. We assume the map is 
 * 7x7. Reward is 1.0 for goal states (digit 2 in map) and -0.04 for others.
 */

#include <assert.h>
#include <fstream>
#include <sstream>
#include <math.h>
#include <queue>
#include <set>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

double T [7][7][4];		///< Transition matrix 7x7x4 -> The 4 dimensions is for each action.

/* ********************************************************************************************* */
int main (int argc, char* argv[]) {

	// Read the file
	int map [7][7];
	ifstream file ("map.txt");
	char line [256];
	for(int i = 0; i < 7; i++) {
		file.getline(line, 256);
		stringstream ss (line);
		for(int j = 0; j < 7; j++) ss >> map[i][j];
	}
	
	// Initialize the utilities to zero
	double U [7][7], Un [7][7];
	memset(U, 0, sizeof(U));
	memset(Un, 0, sizeof(Un));

	// Do the iterations
	static const double gamma = 0.9;
	int offsets [] = {-1, 0, 0, 1, 1, 0, 0, -1};
	for(int iter = 0; iter < 250; iter++) {

		// Update the utilities to the previous iteration's
		memcpy(U, Un, sizeof(U));

		// Print utilities
		printf("======= Iter: %d ==========================================\n", iter);
		for(int i = 0; i < 7; i++) {
			for(int j = 0; j < 7; j++) {
				printf("%+0.3lf ", U[i][j]);
			}
			printf("\n");
		}
		printf("\n");

		// For each state, update the utility based on neighbors
		double maxDiff = 0.0;
		for(int i = 0; i < 7; i++) {
			for(int j = 0; j < 7; j++) {

				// Debug the symmetry case
				bool dbg = false;
				// if((j == 6) && ((i == 2) || (i == 4))) dbg = true;
				if(dbg) printf("\nstate: (%d, %d)\n", i, j);

				// Skip if a obstacle
				if(map[i][j] == 1) continue;

				// Compute the maximum expected utility in this state
				double maxExpectedU = -100.0;
				for(int a = 0; a < 4; a++) {
						if(dbg) printf("action %d\n", a);
				
					// Compute the expected utility of neighbors
					double expectedU = 0.0;
					for(int n = 0; n < 4; n++) {

						// Compute the neighbor location
						int io = offsets[2*n], jo = offsets[2*n+1];
						int ni = i + io, nj = j + jo;
						if((ni < 0) || (nj < 0) || (ni > 6) || (nj > 6)) {		// Check if it bumps to boundary
							ni = i, nj = j;
						}
						if(map[ni][nj] == 1) {																// Check if it bumps to obstacle
							ni = i, nj = j;
						}
						
						// Compute the transition probability 
						int diff = fabs(n - a);
						double tp = 0.8;
						if((diff == 1) || (diff == 3)) tp = 0.1;
						else if(diff == 2) tp = 0.0;
						if(dbg) printf("neighbor %d, tp: %lf, prev U: %lf\n", n, tp, U[ni][nj]);

						// Accumulate the utility	
						expectedU += tp * U[ni][nj];
					}

					// Update the maximum utility
					if(expectedU > maxExpectedU) maxExpectedU = expectedU;
				}

				// Get reward, see if this state is a goal object
				double reward = -0.04;
				if(map[i][j] == 2) reward = 1.0;

				// Compute the new utility
				Un[i][j] = reward + gamma * maxExpectedU;
	
				// Compute max difference to stop computation
				double diff = fabs(Un[i][j] - U[i][j]);
				if(maxDiff < diff) maxDiff = diff;
			}
		}

		if(maxDiff < 1e-5) break;
	}
}
/* ********************************************************************************************* */
