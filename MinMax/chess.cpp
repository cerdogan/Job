/**
 * @file chess.cpp
 * @author Can Erdogan
 * @date July 20, 2015
 * @brief Implementation of min-max algorithm with alpha-beta pruning for a chess game.
 */

#include <assert.h>
#include <iostream>
#include <math.h>
#include <queue>
#include <set>
#include <map>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

using namespace std;

/* ******************************************************************************************** */
enum Pieces {
	P1 = 1, P2, P3, P4, P5, P6, P7, P8,
	C1, K1, B1, Q, K, B2, K2, C2
};

map <int, int> positions;
int board [8][8];

/* ******************************************************************************************** */
void initBoard (bool default_ = true) {

	// Setup the default board
	if(default_) {

		// Set the pawns
		for(int i = 1; i < 9; i++) {
			positions[i] = i + 8 - 1;
			positions[i+16] = i + 8 * 6 - 1;
		}

		// Set the others
		for(int i = 9; i < 17; i++) {
			positions[i] = i - 9;
			positions[i+16] = i + 8 * 6 - 1;
		}
	}

	// Debugging purposes
	else {
		positions[16] = 48;
		positions[19] = 50;
		positions[1] = 52;
		positions[2] = 24;
//		positions[17] = 61;
	}

	// Fill in the board representation
	memset(board, 0, sizeof(board));
	map <int, int>::iterator it = positions.begin();
	for(; it != positions.end(); it++) board[it->second/8][it->second%8] = it->first;
}

/* ******************************************************************************************** */
void printBoard () {

	static const char KNRM [] = "\x1B[0m";	
	static const char KRED [] = "\x1B[31m";	
	static const char KBLU [] = "\x1B[34m";	
	static const char* const KPIECES [17] = 
		{"P", "P", "P", "P", "P", "P", "P", "P", "C", "K", "B", "Q", "K", "B", "K", "C"};
	
	// Print the information
	for(int i = 7; i >= 0; i--) {
		for(int j = 0; j < 8; j++) {
			if(board[i][j] == 0) printf("- ");
			else if(board[i][j] < 17) printf("%s%s%s ", KRED, KPIECES[board[i][j]-1], KNRM);
			else printf("%s%s%s ", KBLU, KPIECES[board[i][j]-17], KNRM);
		}
		printf("\n");
	}
}

/* ******************************************************************************************** */
void createMoves (vector <pair <int,int> >& moves, bool white) {
	
	// For each piece, generate the possibilities
	for(int i = 1; i < 17; i++) {

		// Get the piece position if it exists
		int index = i + (white ? 0 : 16);
		int positionBad;
		map <int, int>::iterator it = positions.find(index);
		if(it == positions.end()) continue;
		else positionBad = it->second;
		int px = positionBad / 8, py = positionBad % 8;
		
		// Create the position based on the piece
		if(i < 9) {		// Pawn
			continue;
	
			// Check if it can move forward if white
			int forw_offset = white ? 1 : -1, new_vert = px + forw_offset;
			if(((new_vert >= 0) && (new_vert < 8)) && (board[new_vert][py] == 0)) moves.push_back(make_pair(index, new_vert*8+py));

			// Check if it can attack an adversary to the board left
			if((new_vert >= 0) && (new_vert < 8) && (py > 0)) {
				bool opponent = (white && (board[new_vert][py-1] > 16)) ||
					(!white && ((board[new_vert][py-1] != 0) && (board[new_vert][py-1] < 17)));
				if(opponent) moves.push_back(make_pair(index, new_vert*8+py-1));
			}

			// Check if it can attack an adversary to the board right
			if((new_vert >= 0) && (new_vert < 8) && (py < 7)) {
				bool opponent = (white && (board[new_vert][py+1] > 16)) ||
					(!white && ((board[new_vert][py+1] != 0) && (board[new_vert][py+1] < 17)));
				if(opponent) moves.push_back(make_pair(index, new_vert*8+py+1));
			}
		}

		else if((i == 9) || (i == 16)) {	// Castle
			
			// Check if it can move board left from its position. Break if there is a collision.
			for(int new_py = py - 1; new_py >= 0; new_py--) {
				bool ownCollide = (!white && (board[px][new_py] > 16)) ||
					(white && ((board[px][new_py] != 0) && (board[px][new_py] < 17)));
				if(ownCollide) break;
				else moves.push_back(make_pair(index, px*8+new_py));
			}

			// Check if it can move board right from its position. Break if there is a collision.
			for(int new_py = py + 1; new_py < 8; new_py++) {
				bool ownCollide = (!white && (board[px][new_py] > 16)) ||
					(white && ((board[px][new_py] != 0) && (board[px][new_py] < 17)));
				if(ownCollide) break;
				else moves.push_back(make_pair(index, px*8+new_py));
			}

			// Check if it can move board up from its position. Break if there is a collision.
			for(int new_px = px + 1; new_px < 8; new_px++) {
				bool ownCollide = (!white && (board[new_px][py] > 16)) ||
					(white && ((board[new_px][py] != 0) && (board[new_px][py] < 17)));
				if(ownCollide) break;
				else moves.push_back(make_pair(index, new_px*8+py));
			}

			// Check if it can move board bottom from its position. Break if there is a collision.
			for(int new_px = px - 1; new_px >= 0; new_px--) {
				bool ownCollide = (!white && (board[new_px][py] > 16)) ||
					(white && ((board[new_px][py] != 0) && (board[new_px][py] < 17)));
				if(ownCollide) break;
				else moves.push_back(make_pair(index, new_px*8+py));
			}
		}

		else if((i == 10) || (i == 15)) {	// Knight 

			// Generate the 8 moves and check availability
			
		}
		else if((i == 11) || (i == 14)) {	// Bishop 
		}
		else if(i == 12) {	// Queen 
		}
		else if(i == 13) {	// King 
		}

		// printf("%d: (%d, %d)\n", index, px, py);
	}
	
}

/* ******************************************************************************************** */
int main (int argc, char* argv[]) {

	initBoard(0);
	printBoard();
	vector <pair <int, int> > moves;
	createMoves(moves, 1);
	printf("#moves: %lu\n", moves.size());
	for(int i = 0; i < moves.size(); i++) printf("(%d, %d)\n", moves[i].first, moves[i].second);
}
/* ******************************************************************************************** */
