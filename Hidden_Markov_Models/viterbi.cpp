/** 
 * @file viterbi.cpp
 * @author Can Erdogan
 * @date 2015-08-09
 * @brief Implementation of the viterbi algorithm. Note the similarity to dynamic programming.
 * Example: Didier Gonze's lecture notes with DNA decoding: 
 * 							http://homepages.ulb.ac.be/~dgonze/TEACHING/viterbi.pdf
 * Note that probabilities are logged.
 */

#include <assert.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <queue>
#include <map>
#include <set>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

using namespace std;

enum Gene {A = 0, C, G, T};
Gene seq [9] = {G, G, C, A, C, T, G, A, A};
double probH [9], probL [9];
double trans [9] = {0.0, -1.0, -1.0, 0.0, -1.0, -1.0, 0.0, -1.322, -0.737}; // transition probs.
double obsH [4] = {-2.322, -1.737, -1.737, -2.322};	// observation probs at high
double obsL [4] = {-1.737, -2.322, -2.322, -1.737};	// observation probs at low 

/* ******************************************************************************************** */
double viterbi (int n, bool high) {	

	// Initial transfer from start
	if(n == 0) {
		if(high) {
			if(probH[0] > 0) return probH[0];
			probH[0] = -1 + obsH[seq[n]];
			return probH[0];
		}
		else {
			if(probL[0] > 0) return probL[0];
			probL[0] = -1 + obsL[seq[n]];
			return probL[0];
		}
	}

	// Check memory
	if(high && probH[n] > 0) return probH[n];
	else if(!high && probL[n] > 0) return probL[n];

	// Recursive call
	if(high) {
		probH[n] = obsH[seq[n]] + max(viterbi(n-1,1)-1, viterbi(n-1,0)-1.322);
		return probH[n];
	}
	else {
		probL[n] = obsL[seq[n]] + max(viterbi(n-1,1)-1, viterbi(n-1,0)-0.737);
		return probL[n];
	}
}

/* ******************************************************************************************** */
int main (int argc, char* argv[] ) {
	int maxLevel = 8;
	double resH = viterbi(maxLevel, true);
	double resL = viterbi(maxLevel, false);
	printf("Most likely path: '");
	for(int i = 0; i <= maxLevel; i++) {
		if(probH[i] > probL[i] + 1e-2) printf("H");
		else printf("L");
	}
	printf("'\n");
}
