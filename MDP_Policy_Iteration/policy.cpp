/**
 * @file policy.cpp
 * @author Can Erdogan
 * @date July 16, 2015
 * @brief Implementation of policy iteration for a MDP based on map.txt. If the agent attempts to
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

/* ********************************************************************************************* */
// int offsets [] = {-1, 0, 0, 1, 1, 0, 0, -1};
int offsets [] = {0, -1, -1, 0, 0, 1, 1, 0};
int map [7][7];

/* ********************************************************************************************* */
// Compute the expected utility of neighbors given an action
double expectedNeighborUtility (int i, int j, int a, double U[][7], bool dbg = false) {
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

		if(dbg) printf("(%d, %d) vs. (%d, %d), U: %lf, tp: %lf\n", i, j, ni, nj, U[ni][nj], tp);

		// Accumulate the utility	
		expectedU += tp * (U)[ni][nj];
	}
	return expectedU;
}

/* ********************************************************************************************* */
int main (int argc, char* argv[]) {

	// Read the file
	ifstream file ("map.txt");
	char line [256];
	for(int i = 0; i < 7; i++) {
		file.getline(line, 256);
		stringstream ss (line);
		for(int j = 0; j < 7; j++) ss >> map[i][j];
	}
	
	// Initialize the policy randomly
	srand(time(NULL));
	int P [7][7];
	for(int i = 0; i < 7; i++) 
		for(int j = 0; j < 7; j++) 
			P[i][j] = rand() % 4;


	// Do the iterations
	static const double gamma = 0.9;
	for(int iter = 0; iter < 250; iter++) {

		bool unchanged = true;

		// Print the policy
		getchar();
		printf("======= Iter: %d ==========================================\n", iter);
		for(int i = 0; i < 7; i++) {
			for(int j = 0; j < 7; j++) {
				if(map[i][j] == 1) printf("x ");
				else printf("%d ", P[i][j]);
			}
			printf("\n");
		}
		printf("\n");

		// Compute the values given the policy --------------------------
		double U [7][7], Un [7][7];
		{
			// Initialize the utilities to zero
			memset(U, 0, sizeof(U));
			memset(Un, 0, sizeof(Un));

			// Iteratively compute the values
			for(int iterU = 0; iterU < 25; iterU++) {
			
				// Update the utilities to the previous iteration's
				memcpy(U, Un, sizeof(U));

				// Compute the new values from last iteration
				double maxDiff = -100.0;
				for(int i = 0; i < 7; i++) {
					for(int j = 0; j < 7; j++) {

						// Compute the expected utility of neighbors
						double expectedU = expectedNeighborUtility(i, j, P[i][j], U);

						// Update the U value
						Un[i][j] = ((map[i][j] == 2) ? 1.0 : -0.04) + gamma * expectedU;

						// Check if the values have converged
						double diff = fabs(Un[i][j] - U[i][j]);
						if(maxDiff < diff) maxDiff = diff;
					}
				}

				// Stop if the values converge
				if(maxDiff < 1e-3) {
					printf("Values converge at %d\n", iterU);
					break;
				}
			}

			printf("-------------------------------------------------------\n", iter);
			for(int i = 0; i < 7; i++) {
				for(int j = 0; j < 7; j++) {
					printf("%+3lf ", U[i][j]);
				}
				printf("\n");
			}
			printf("\n");
		}
		
		// For each state, compare the current value and the result if the best action is chosen
		for(int i = 0; i < 7; i++) {
			for(int j = 0; j < 7; j++) {

				if(map[i][j] == 1) continue;
				bool dbg = 0 && (i == 6) && (j == 0);

				// Compute the value with the best action
				double maxExpectedU = -100.0;
				int bestA = -1;
				for(int a = 0; a < 4; a++) {
					double expectedU = expectedNeighborUtility(i, j, a, U, dbg);
					if(expectedU > (maxExpectedU + 1e-4)) {
						bestA = a;
						maxExpectedU = expectedU;
					}
					if(dbg) printf("U[%d,%d]: %lf, a: %d, expectedU: %lf, maxExpectedU: %lf, bestA: %d\n", i, j, U[i][j], a, expectedU, maxExpectedU, bestA);
				}

				// Compare against the current neighborhood value
				double policyExpectedU = expectedNeighborUtility(i, j, P[i][j], U);
				if(maxExpectedU > policyExpectedU) {
					P[i][j] = bestA;
					unchanged = false;
				}
			}
		}

		if(unchanged) break;
	}

	printf("Policy converged!\n");
}
/* ********************************************************************************************* */
