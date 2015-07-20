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
		positions[P1] = 5*8 + 3;
		//positions[K+16] = 4*8 + 4;
		positions[P2+16] = 6*8 + 4;
		//positions[P3] = 5*8 + 3;
		//positions[C1+16] = 2*8 + 2;
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

		else if((i == 10) || (i == 15)) {	// Knight 

			// Generate the 8 moves and check availability
			static const int knight_xs [] = {1, 2, 2, 1, -1, -2, -2, -1};
			static const int knight_ys [] = {-2, -1, 1, 2, 2, 1, -1, -2};
			for(int move_idx = 0; move_idx < 8; move_idx++) {
				
				// Check if out of bounds
				int new_px = px + knight_xs[move_idx], new_py = py + knight_ys[move_idx];
				if((new_px < 0) || (new_px > 7) || (new_py < 0) || (new_py > 7)) continue;
				
				// Check if in collision with own side
				bool ownCollide = (!white && (board[new_px][new_py] > 16)) ||
					(white && ((board[new_px][new_py] != 0) && (board[new_px][new_py] < 17)));
				if(ownCollide) continue;
				
				// Add to the moves
				moves.push_back(make_pair(index, new_px*8+new_py));
			}
		}

		else if(((i == 9) || (i == 16)) || ((i == 11) || (i == 14)) || (i == 12) || (i == 13))	{ 


			// Check if it can move in the four directions
			vector <int> xs, ys;
			int MAX_MOVE = 8;
			if((i == 9) || (i == 16)) {
				int castle_xs [] = {1, 0, -1, 0}, castle_ys [] = {0, 1, 0, -1};
				xs = vector <int> (castle_xs, castle_xs + 4), ys = vector <int> (castle_ys, castle_ys + 4);
			}
			else if((i == 11) || (i == 14)) {
				int bishop_xs [] = {1, 1, -1, -1}, bishop_ys [] = {-1, 1, 1, -1};
				xs = vector <int> (bishop_xs, bishop_xs + 4), ys = vector <int> (bishop_ys, bishop_ys + 4);
			}
			else if(i == 12) {
				int queen_xs [] = {1, 1, -1, -1, 1, 0, -1, 0}, queen_ys [] = {-1, 1, 1, -1, 0, 1, 0, -1};
				xs = vector <int> (queen_xs, queen_xs + 8), ys = vector <int> (queen_ys, queen_ys + 8);
			}
			else if(i == 13) {
				int queen_xs [] = {1, 1, -1, -1, 1, 0, -1, 0}, queen_ys [] = {-1, 1, 1, -1, 0, 1, 0, -1};
				xs = vector <int> (queen_xs, queen_xs + 8), ys = vector <int> (queen_ys, queen_ys + 8);
				MAX_MOVE = 2;
			}

			for(int angle_idx = 0; angle_idx < xs.size(); angle_idx++) {
				for(int move_idx = 1; move_idx < MAX_MOVE; move_idx++) {

					// Get the new location
					int new_px = px + move_idx * xs[angle_idx];
					int new_py = py + move_idx * ys[angle_idx];

					// Check if it is out of bounds
					if((new_px < 0) || (new_px > 7) || (new_py < 0) || (new_py > 7)) break;

					// Check collisions
					bool ownCollide = (!white && (board[new_px][new_py] > 16)) ||
						(white && ((board[new_px][new_py] != 0) && (board[new_px][new_py] < 17)));
					bool theirCollide = (white && (board[new_px][new_py] > 16)) ||
						(!white && ((board[new_px][new_py] != 0) && (board[new_px][new_py] < 17)));

					// If collision with own, stop. If not, add and then stop.
					if(ownCollide) break;
					else {
						moves.push_back(make_pair(index, new_px*8+new_py));
						if(theirCollide) break;
					}
				}
			}
		}
	}
}

/* ******************************************************************************************** */
void makeMove (const pair<int,int>& move) {

	// Make the changes to the position list for the mover (possibly attacker)
	int currPos = positions[move.first];
	board[currPos/8][currPos%8] = 0;
	positions[move.first] = move.second;

	// Check if there is a defender
	if(board[move.second/8][move.second%8] != 0) {
		positions.erase(board[move.second/8][move.second%8]);
	}
	
	// Update the move on the board
	board[move.second/8][move.second%8] = move.first;
	printf("#positions: %lu\n", positions.size());
}

/* ******************************************************************************************** */
int main (int argc, char* argv[]) {

	srand(time(NULL));
	initBoard();
	printBoard();
	vector <pair <int, int> > moves;
	createMoves(moves, 1);
	printf("#moves: %lu\n", moves.size());
	for(int i = 0; i < moves.size(); i++) printf("(%d, %d)\n", moves[i].first, moves[i].second);
	//makeMove(moves[1]);
	printBoard();

	while(true) {

		// Get user input
		char line[256];
		char* p = gets (line);  /* Uh-oh. Don't panic. See below. */
		int prevPos = (p[0] - 'a') + (p[1]-'1') * 8;
		int nextPos = (p[2] - 'a') + (p[3]-'1') * 8;
		int piece = board[(p[1]-'1')][p[0]-'a'];
		printf("'%s': %d: %d -> %d\n", p, piece, prevPos, nextPos);

		// Make the user move
		pair <int, int> move = make_pair(piece, nextPos);
		makeMove(move);
		printBoard();
		printf("===========================================\n");

		// Let computer make a random move
		vector <pair <int, int> > moves;
		createMoves(moves, 0);
		int random = rand() % moves.size();
		makeMove(moves[random]);
		printBoard();
		printf("===========================================\n");
	}
}
/* ******************************************************************************************** */
