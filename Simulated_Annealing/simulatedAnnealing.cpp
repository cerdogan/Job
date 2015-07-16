/**
 * @file simulatedAnnealing.cpp
 * @author Can Erdogan
 * @date July 13, 2015
 * @brief Implementation of simulated annealing for the n-queens example. The description in 
 * http://yuval.bar-or.org/index.php?item=9 is used:
 *
 *		The queens are initially placed in random rows and columns such that each queen
 *		is in a different row and a different column. During each turn, an attacked
 *		queen is chosen and a random column is picked for that queen. If moving the
 *		queen to the new column will reduce the number of attacked queens on the board,
 *		the move is taken. Otherwise, the move is taken only with a certain probability,
 *		which decreases over time. Hence early on the algorithm will tend to take moves
 *		even if they don't improve the situation. Later on, the algorithm will only make
 *		moves which improve the situation on the board. The temperature function used
 *		is: T(n) = 100 / n where n is the round number. The probability function used to
 *		decide if a move with a negative impact on the score is allowed is:
 *		P(dE)=exp(dE/T) where dE is the difference in "energy" (the difference in the
 *		number of attacked queens) and T is the temperature for the round.
 */

#include <assert.h>
#include <iostream>
#include <math.h>
#include <queue>
#include <set>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

//static const int numQueens = 24;
int** board;
int numQueens;

/* ********************************************************************************************* */
/// Prints the board where 1 is filled by a queen and 0 is empty
void printBoard () {
	for(size_t i = 0; i < numQueens; i++) {
		for(size_t j = 0; j < numQueens; j++) 
			printf("%d ", board[i][j]);
		printf("\n");
	}
}

/* ********************************************************************************************* */
/// Computes the number of collisions between queens
int numCollisions (set <int>* attacked = NULL) {

	// Check if the queens in two different rows are on the same column or diagonally in collision
	int numColls = 0;
	for(size_t i = 0; i < numQueens; i++) {

		// Find the location of the queen in this row
		int loc1 = 0;
		for(size_t k = 0; k < numQueens; k++) {
			if(board[i][k] == 1) {
				loc1 = k;
				break;
			}
		}

		// Check against the next row
		for(size_t j = i+1; j < numQueens; j++) {
				
			// Find the location of the queen in this row
			int loc2 = 0;
			for(size_t k = 0; k < numQueens; k++) {
				if(board[j][k] == 1) {
					loc2 = k;
					break;
				}
			}

			// Check if they are on the same column
			if(loc1 == loc2) {
				if(attacked) attacked->insert(i);
				if(attacked) attacked->insert(j);
				numColls++;
			}
			
			// Check if they are diagonally in collision
			if(((j - i) == (loc2 - loc1)) || ((j - i) == (loc1 - loc2))) {
				if(attacked) attacked->insert(i);
				if(attacked) attacked->insert(j);
				numColls++;
			}
		}
	}

	return numColls;
}

/* ********************************************************************************************* */
/// Set the queens in random position on the board
void initializeBoard () {
	
	// Set all the cells to 0 for empty
	for(size_t i = 0; i < numQueens; i++) 
		for(size_t j = 0; j < numQueens; j++) 
			board[i][j] = 0;

	// For each row, choose a random column to place the queen
	srand(time(NULL));
	for(size_t i = 0; i < numQueens; i++) {
		size_t col_idx = (rand() % numQueens);
		board[i][col_idx] = 1;
	}
}

/* ********************************************************************************************* */
int numChanceJumps [10];
int MAX = 1000;
int simulatedAnnealing () {

	bool dbg = 0;
	for(int n = 0; n < MAX; n++) {

		// getchar();
		if(dbg) printf("\n\n=== %d ====================================\n", n);
		if(dbg) printBoard();

		// Find the attacked queens
		set <int> attacked;
		int numColls = numCollisions(&attacked);
		if(dbg) printf("numColls: %d, numAttacked: %d\n", numColls, attacked.size());
		if(numColls == 0) {
			printf("Success for %d queens! Num iters: %d\n", numQueens, n);
			return n;
		}

		// Choose one of the attacked queens arbitrarily
		int numAttacked = attacked.size();
		int rand_idx = rand() % numAttacked;
		set <int>::iterator it = attacked.begin();
		advance(it, rand_idx);
		int q_idx = *it;
		
		// Find the current column of the chosen queen
		int prevCol = 0;
		for(int i = 0; i < numQueens; i++) {
			if(board[q_idx][i] == 1) {
				prevCol = i;
				break;
			}
		}
	
		// Check if moving the queen to an arbitrary column helps; if so make the move
		int newCol = rand() % numQueens;
		board[q_idx][prevCol] = 0;
		board[q_idx][newCol] = 1;
		int numColls2 = numCollisions();
		if(numColls2 <= numColls) {
			if(dbg) printf("Good move\n");
			continue;
		}

		// If the move is not helpful, make it with some probability
		double tn = 100.0 / (n + 1);
		double P = exp((numColls-numColls2)/tn);
		double random = ((double) rand()) / RAND_MAX;
		if(random < P) {
			numChanceJumps[n / 50] += 1;
			if(dbg) printf("Lucky bad move: P: %lf, tn: %lf\n", P, tn);
			if(dbg) 
				printf("\tnumColls2: %d, numColls: %d, diff: %d\n", numColls2, numColls, numColls-numColls2);
			continue;
		}

		// If chance doesn't help the bad move, undo it
		board[q_idx][prevCol] = 1;
		board[q_idx][newCol] = 0;
		if(dbg) printf("No move\n");
	}

	printf("Fail.\n");
	return MAX;
}

/* ********************************************************************************************* */
int main (int argc, char* argv[]) {

	// Get the number of queens
	assert(argc > 1 && "Need # of queens");
	numQueens = atoi(argv[1]);
	board = new int* [numQueens];
	for(int i = 0; i < numQueens; i++) 
		board[i] = new int [numQueens];

	// Initialize the board
	initializeBoard();
	memset(numChanceJumps , 0, sizeof(numChanceJumps));

	// Do the annealing
	int numIters = simulatedAnnealing();

	// Print debugging data
	printf("Number of chance jumps every %d iters:\n\t", MAX / 10);
	for(int i = 0; i < numIters / 50; i++) printf("%d ", numChanceJumps[i]);
	printf("\n\n\n");
	printBoard();
	
}
/* ********************************************************************************************* */
