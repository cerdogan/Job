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

struct State {
	map <int, int> positions;
	int board [8][8];
	vector <int> removed;
	State () {}
	State (const State& s) {
		positions = s.positions;
		removed = s.removed;
		memcpy(board, s.board, sizeof(s.board));
	}
};

/* ******************************************************************************************** */
int evaluateBoard (const State& state, bool white) {

	int total = 0;
	int vals [] = {1, 1, 1, 1, 1, 1, 1, 1, 5, 3, 3, 9, 1000, 3, 3, 5};
	for(int i = 0; i < state.removed.size(); i++) {
		int index = state.removed[i]-1;
		bool white = index >= 16;
		if(white) index-=16;
		int val = vals[index];
		if(white) total -= val;
		else total += val;
	}

//	if(!white) 
//	total *= -1;
	return total;
}

/* ******************************************************************************************** */
void initBoard (State& state, bool default_ = true) {

	map <int, int>& positions = state.positions;

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
	memset(state.board, 0, sizeof(state.board));
	map <int, int>::iterator it = positions.begin();
	for(; it != positions.end(); it++) state.board[it->second/8][it->second%8] = it->first;
}

/* ******************************************************************************************** */
void printBoard (const State& state) {

	static const char KNRM [] = "\x1B[0m";	
	static const char KRED [] = "\x1B[31m";	
	static const char KBLU [] = "\x1B[34m";	
	static const char* const KPIECES [16] = 
		{"P", "P", "P", "P", "P", "P", "P", "P", "C", "K", "B", "Q", "K", "B", "K", "C"};
	
	// Print the information
	for(int i = 7; i >= 0; i--) {
		for(int j = 0; j < 8; j++) {
			if(state.board[i][j] == 0) printf("- ");
			else if(state.board[i][j] < 17) printf("%s%s%s ", KRED, KPIECES[state.board[i][j]-1], KNRM);
			else printf("%s%s%s ", KBLU, KPIECES[state.board[i][j]-17], KNRM);
		}
		printf("\n");
	}
	printf("Value: %d =================================\n", evaluateBoard(state, 1));
}

/* ******************************************************************************************** */
void createMoves (const State& s, vector <pair <int,int> >& moves, bool white) {
	
	// For each piece, generate the possibilities
	// for(int i = 1; i < 17; i++) {
	for(int i = 1; i < 17; i++) {

		// Get the piece position if it exists
		int index = i + (white ? 0 : 16);
		int positionBad;
		map <int, int>::const_iterator it = s.positions.find(index);
		if(it == s.positions.end()) continue;
		else positionBad = it->second;
		int px = positionBad / 8, py = positionBad % 8;
		
		// Create the position based on the piece
		if(i < 9) {		// Pawn
	
			// Check if it can move forward if white
			int forw_offset = white ? 1 : -1, new_vert = px + forw_offset;
			if(((new_vert >= 0) && (new_vert < 8)) && (s.board[new_vert][py] == 0)) 
				moves.push_back(make_pair(index, new_vert*8+py));

			// Check if it can attack an adversary to the s.board left
			if((new_vert >= 0) && (new_vert < 8) && (py > 0)) {
				bool opponent = (white && (s.board[new_vert][py-1] > 16)) ||
					(!white && ((s.board[new_vert][py-1] != 0) && (s.board[new_vert][py-1] < 17)));
				if(opponent) moves.push_back(make_pair(index, new_vert*8+py-1));
			}

			// Check if it can attack an adversary to the s.board right
			if((new_vert >= 0) && (new_vert < 8) && (py < 7)) {
				bool opponent = (white && (s.board[new_vert][py+1] > 16)) ||
					(!white && ((s.board[new_vert][py+1] != 0) && (s.board[new_vert][py+1] < 17)));
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
				bool ownCollide = (!white && (s.board[new_px][new_py] > 16)) ||
					(white && ((s.board[new_px][new_py] != 0) && (s.board[new_px][new_py] < 17)));
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
				int king_xs [] = {1, 1, -1, -1, 1, 0, -1, 0}, king_ys [] = {-1, 1, 1, -1, 0, 1, 0, -1};
				xs = vector <int> (king_xs, king_xs + 8), ys = vector <int> (king_ys, king_ys + 8);
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
					bool ownCollide = (!white && (s.board[new_px][new_py] > 16)) ||
						(white && ((s.board[new_px][new_py] != 0) && (s.board[new_px][new_py] < 17)));
					bool theirCollide = (white && (s.board[new_px][new_py] > 16)) ||
						(!white && ((s.board[new_px][new_py] != 0) && (s.board[new_px][new_py] < 17)));

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
void makeMove (const State& s, const pair<int,int>& move, State& s2) {

	// Copy the data into new state
	s2 = State(s);

	// Make the changes to the position list for the mover (possibly attacker)
	int currPos = s2.positions[move.first];
	s2.board[currPos/8][currPos%8] = 0;
	s2.positions[move.first] = move.second;

	// Check if there is a defender
	if(s2.board[move.second/8][move.second%8] != 0) {
		s2.positions.erase(s2.board[move.second/8][move.second%8]);
		s2.removed.push_back(s2.board[move.second/8][move.second%8]);
	}
	
	// Update the move on the board
	s2.board[move.second/8][move.second%8] = move.first;
}

/* ******************************************************************************************** */
void printMove (const pair <int, int>& move) {
	static const char* const KPIECES [16] = 
		{"P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8", "C1", "K1", "B1", "Q", "K", "B2", "K2", "C2"};
	int index = move.first;
	if(index > 16) index -= 16;
	printf("%s: %s to (%d, %d)\n", move.first > 16 ? "Black" : "White", KPIECES[index-1], 
		move.second/8, move.second%8);
}

/* ******************************************************************************************** */
struct Result {
	int val;
	vector <pair <int,int> > moves;
};

/* ******************************************************************************************** */
int MAX_LEVEL = 5;
Result* maxState (State* s, bool white, int level);
Result* minState (State* s, bool white, int level);

void minMax (State* s, bool white, pair <int,int>& bestMove) {
	Result* res = maxState(s, white, 0);
	bestMove = res->moves[res->moves.size()-1];
	for(int i = 0; i < res->moves.size(); i++) {
		printMove(res->moves[i]);
	}
}

bool dbg = 0;
/* ******************************************************************************************** */
Result* maxState (State* s, bool white, int level) {

	// If terminal state, evaluate it
	if(level == MAX_LEVEL) {
		int val = evaluateBoard (*s, white);
		// if(dbg) printf("\tmax terminal: %d\n", val);
		Result* res = new Result;
		res->val = val;
		return res;
	}

	if(dbg) printf("MAXIMUM STATE %d vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n", level);

	// Get the possible states
	vector <pair <int, int> > moves;
	createMoves(*s, moves, white);

	// Find the state with the maximum value
	int maxVal = -10000;
	pair <int, int> bestMove;
	Result* bestRes;
	State* s2 = new State;
	for(int i = 0; i < moves.size(); i++) {

		// Create the state
		makeMove(*s, moves[i], *s2);
		if(level == 0 && moves[i].first == 20 && moves[i].second == 42) dbg = true;
		else if((level == 0) && !(moves[i].first == 20 && moves[i].second == 42)) dbg = false;

		// Get the value
		pair <int, int> move;
		Result* res = minState(s2, !white, level+1);
		if(dbg) printf("\tmade call %d->%d: %d\n", moves[i].first, moves[i].second, res->val);

		// Otherwise, get the value of th
		if(res->val > maxVal) {
			maxVal = res->val;
			bestMove = moves[i];
			bestRes = res;
		}
		else delete res;
	}

	if(dbg) printf("\n>> %d->%d: max val: %d\n", bestMove.first, bestMove.second, maxVal);
	bestRes->moves.push_back(bestMove);
	if(dbg) printf("MAXIMUM STATE %d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n", level);
	delete s2;
	return bestRes;
}

/* ******************************************************************************************** */
Result* minState (State* s, bool white, int level) {

	if(dbg) printf("MINIMUM STATE %d vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n", level);

	// If terminal state, evaluate it
	if(level == MAX_LEVEL) {
		int val = evaluateBoard (*s, white);
		Result* res = new Result();
		res->val = val;
		return res;
	}

	// Get the possible states
	vector <pair <int, int> > moves;
	createMoves(*s, moves, white);

	// Find the state with the maximum value
	int minVal = 10000;
	pair <int, int> bestMove;
	Result* bestRes;
	State* s2 = new State;
	for(int i = 0; i < moves.size(); i++) {

		// Create the state
		makeMove(*s, moves[i], *s2);

		// Get the value
		pair <int, int> move;
		Result* res = maxState(s2, !white, level+1);
		if(dbg) {printf("%d->%d: %d | ", moves[i].first, moves[i].second, res->val); fflush(stdout); }

		// Otherwise, get the value of th
		if(res->val < minVal) {
			minVal = res->val;
			bestMove = moves[i];
			bestRes = res;
		}
		else delete res;
	}

	if(dbg) printf("\n>> %d->%d: min val: %d\n", bestMove.first, bestMove.second, minVal);
	bestRes->moves.push_back(bestMove);
	for(int i = 0; i < bestRes->moves.size(); i++) {
		if(dbg) printMove(bestRes->moves[i]);
	}
	if(dbg) printf("MINIMUM STATE %d ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n", level);
	delete s2;
	return bestRes;
}

/* ******************************************************************************************** */
int main (int argc, char* argv[]) {

	// Initialize the board
	srand(time(NULL));
	State state, state2;
	initBoard(state);
	printBoard(state);
//	makeMove(state, make_pair(5, 44), state2);
//	printBoard(state2);
//	makeMove(state2, make_pair(22, 44), state);
//	printBoard(state);
//	return 1;

	while(true) {

		// Get user input
		char line[256];
		char* p = gets (line);  /* Uh-oh. Don't panic. See below. */
		int prevPos = (p[0] - 'a') + (p[1]-'1') * 8;
		int nextPos = (p[2] - 'a') + (p[3]-'1') * 8;
		int piece = state.board[(p[1]-'1')][p[0]-'a'];
		printf("'%s': %d: %d -> %d\n", p, piece, prevPos, nextPos);

		// Make the user move
		pair <int, int> move = make_pair(piece, nextPos);
		makeMove(state, move, state2);
		printBoard(state2);

		// Let compute make a min-max move
		if(true) {
			pair <int,int> bestMove;
			minMax (&state2, false, bestMove);
			printf("%d -> %d\n", bestMove.first, bestMove.second);
			makeMove(state2, bestMove, state);
			printBoard(state);
		}

		// Let computer make a random move
		else {
			vector <pair <int, int> > moves;
			createMoves(state2, moves, 0);
			int random = rand() % moves.size();
			makeMove(state2, moves[random], state);
			printBoard(state);
		}
		
	}
}
/* ******************************************************************************************** */
