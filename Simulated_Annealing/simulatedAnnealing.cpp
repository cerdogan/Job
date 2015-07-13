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

static const int numQueens = 4;
int board [numQueens][numQueens];

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
int numCollisions () {

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
			if(loc1 == loc2) numColls++;
			
			// Check if they are diagonally in collision
			if(((j - i) == (loc2 - loc1)) || ((j - i) == (loc1 - loc2))) numColls++;
		}
	}

	return numColls;
}

/* ********************************************************************************************* */
/// Set the queens in random position on the board
void initializeBoard () {
	
	// Set all the cells to 0 for empty
	memset(board, 0, sizeof(board));

	// For each row, choose a random column to place the queen
	srand(time(NULL));
	for(size_t i = 0; i < numQueens; i++) {
		size_t col_idx = (rand() % numQueens);
		board[i][col_idx] = 1;
	}
}

/* ********************************************************************************************* */
int main (int argc, char* argv[]) {

	// Initialize the board
	initializeBoard();

	printBoard();
	printf("\n# of colls: %d\n", numCollisions());
}
/* ********************************************************************************************* */
