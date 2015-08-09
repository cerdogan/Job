/** 
 * @file lcs.cpp
 * @author Can Erdogan
 * @date 2015-08-08.cpp
 * @brief Implementation of dynamic programming to solve the "Longest Common Subsequence" problem.
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

int* memory;
int N, M;

/* ******************************************************************************************** */
void recover (const char* S, int n, int m, string& s) {
	static const bool dbg = 0;
	if(dbg) printf("Call: (%d, %d)\n", n, m);
	if(n < 0 || m < 0) return;
	else if(n > 0 && (memory[n * M + m] == memory[(n-1) * M + m])) recover(S, n-1, m, s); 
	else if(m > 0 && (memory[n * M + m] == memory[n * M + m - 1])) recover(S, n, m-1, s); 
	else {
		if(dbg) printf("\tappend: '%c'\n", S[n]);
		s += S[n];
		recover(S, n-1, m-1, s);
	}
}

/* ******************************************************************************************** */
int lcs (const char* S, int n, const char* T, int m) {	

	static const bool dbg = 0;
	if(dbg) printf("Call: (%d, %d)\n", n, m);

	// Default
	if(n == -1 || m == -1) return 0;

	// Check memory
	if(memory[n * M + m] != -1) {
		if(dbg) printf("\tmemory: %d\n", memory[n * M + m]);
		return memory[n * M + m];
	}

	// Recursive call
	int res;
	if(S[n] == T[m]) res = (1 + lcs(S, n-1, T, m-1));
	else res = max(lcs(S, n-1, T, m), lcs(S, n, T, m-1));

	// Set memory
	memory[n * M + m] = res;
	if(dbg) printf("\tres: %d\n", res);
	return res;
}

/* ******************************************************************************************** */
int main (int argc, char* argv[] ) {
	assert(argc > 2 && "Need string arguments for subsequence");
	N = strlen(argv[1]), M = strlen(argv[2]);
	memory = new int [N * M];
	for(int i = 0; i < N * M; i++) memory[i] = -1;
	int res = lcs(argv[1], N - 1, argv[2], M - 1);
	string subr;
	recover(argv[1], N-1, M-1, subr);
	string sub (subr.rbegin(), subr.rend());
	printf("res: %d, sub: %s\n", res, sub.c_str());
}
/* ******************************************************************************************** */
