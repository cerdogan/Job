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
	C1, K1, B1, Q, W, B2, K2, C2
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
		{"P", "P", "P", "P", "P", "P", "P", "P", "C", "K", "B", "Q", "W", "B", "K", "C"};
	
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
void knightMoves (const State& s, vector <pair <int,int> >& moves, bool white, int px, int py,
		int index) {

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

/* ******************************************************************************************** */
void pawnMoves (const State& s, vector <pair <int,int> >& moves, bool white, int px, int py, 
		int index) {

	// Check if it can move forward if white
	int forw_offset = white ? 1 : -1, new_vert = px + forw_offset;
	if(((new_vert >= 0) && (new_vert < 8)) && (s.board[new_vert][py] == 0)) {

		moves.push_back(make_pair(index, new_vert*8+py));

		// Check if it can make a double forward now that we know its forward is empty
		if((px == 1 && white) || (px == 6 && !white)) {
			new_vert += forw_offset;
			if(s.board[new_vert][py] == 0) 
				moves.push_back(make_pair(index, new_vert*8+py));
		}
	}

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

/* ******************************************************************************************** */
void otherMoves (const State& s, vector <pair <int,int> >& moves, bool white, int px, int py, 
		int index, int i) {

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

/* ******************************************************************************************** */
bool getPieceLocation (const State& s, int index, bool white, int& px, int& py) {
	int positionBad;
	map <int, int>::const_iterator it = s.positions.find(index);
	if(it == s.positions.end()) return false;
	else positionBad = it->second;
	px = positionBad / 8, py = positionBad % 8;
	return true;
}

/* ******************************************************************************************** */
void createMoves (const State& s, vector <pair <int,int> >& moves, bool white) {
	
	// For each piece, generate the possibilities
	// for(int i = 1; i < 17; i++) {
	for(int i = 1; i < 17; i++) {

		// Get the piece position if it exists
		int index = i + (white ? 0 : 16), px, py;
		if(!getPieceLocation(s, index, white, px, py)) continue;

				// Create the position based on the piece
		if(i < 9) pawnMoves(s, moves, white, px, py, index);
		else if((i == 10) || (i == 15)) knightMoves(s, moves, white, px, py, index);
		else if(((i == 9) || (i == 16)) || ((i == 11) || (i == 14)) || (i == 12) || (i == 13))	
			otherMoves(s, moves, white, px, py, index, i);
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
		{"P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8", "C1", "K1", "B1", "Q", "W", "B2", "K2", "C2"};
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
Result* maxState (State* s, bool white, int level, int alpha, int beta);
Result* minState (State* s, bool white, int level, int alpha, int beta);
int numStates = 0;

void minMax (State* s, bool white, pair <int,int>& bestMove) {
	Result* res = maxState(s, white, 0, -100000, 100000);
	bestMove = res->moves[res->moves.size()-1];
	for(int i = 0; i < res->moves.size(); i++) {
		printMove(res->moves[i]);
	}
}

bool dbg = 0;
/* ******************************************************************************************** */
Result* maxState (State* s, bool white, int level, int alpha, int beta) {

	numStates++;
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
		Result* res = minState(s2, !white, level+1, alpha, beta);
		if(dbg) printf("\tmade call %d->%d: %d\n", moves[i].first, moves[i].second, res->val);

		// Otherwise, get the value of th
		if(res->val > maxVal) {
			maxVal = res->val;
			bestMove = moves[i];
			bestRes = res;

			// Cut short if necessary
			if(maxVal >= beta) return bestRes;

			// Update alpha
			alpha = max(alpha, maxVal);
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
Result* minState (State* s, bool white, int level, int alpha, int beta) {

	if(dbg) printf("MINIMUM STATE %d vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n", level);

	numStates++;
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
		Result* res = maxState(s2, !white, level+1, alpha, beta);
		if(dbg) {printf("%d->%d: %d | ", moves[i].first, moves[i].second, res->val); fflush(stdout); }

		// Otherwise, get the value of th
		if(res->val < minVal) {
			minVal = res->val;
			bestMove = moves[i];
			bestRes = res;

			// Cut short if necessary
			if(minVal <= alpha) return bestRes;

			// Update alpha
			beta = min(beta, minVal);
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
pair <int, int> processInput (State& s) {

	// Get user input
	char line[256];
	char* p = gets (line);  /* Uh-oh. Don't panic. See below. */
	bool shortP = strlen(p) < 3;

	// Determine the next position
	int nextPos;
	if(shortP) nextPos = (p[0] - 'a') + (p[1]-'1') * 8;
	else nextPos = (p[1] - 'a') + (p[2]-'1') * 8;
	printf("nextPos: %d\n", nextPos);

	// Get the piece
	int i = -1;
	if(!shortP) {
		switch(p[0]) {
			case 'P': i = 1; break;
			case 'C': i = 9; break;
			case 'K': i = 10; break;
			case 'B': i = 11; break;
			case 'Q': i = 12; break;
			case 'W': i = 13; break;
		}
	}
	else i = 1;
	assert(i != -1 && "Unknown piece");

	printf("i: %d\n", i);
	
	// For multiple options, choose one of the pieces
	vector <pair <int,int> > moves;
	int rookIdx [] = {9,16}, knightIdx [] = {10, 15}, bishopIdx [] = {11,14};
	int px, py;
	if(i < 9) {		// For pawns
		for(int i = 1; i < 9; i++) {
			moves.clear();
			if(!getPieceLocation(s, i, true, px, py)) continue;
			pawnMoves (s, moves, true, px, py, i);
			for(int j = 0; j < moves.size(); j++) {
				if(moves[j].second == nextPos) 
					return make_pair(i, nextPos);
			}
		}
	}
	else if((i == 9) || (i == 16)) {		// For rooks
		for(int i = 0; i < 2; i++) {
			moves.clear();
			if(!getPieceLocation(s, rookIdx[i], true, px, py)) continue;
			otherMoves (s, moves, true, px, py, rookIdx[i], rookIdx[i]);
			for(int j = 0; j < moves.size(); j++) 
				if(moves[j].second == nextPos) 
					return make_pair(rookIdx[i], nextPos);
		}
	}
	else if((i == 11) || (i == 14)) {		// For bishops
		for(int i = 0; i < 2; i++) {
			moves.clear();
			if(!getPieceLocation(s, bishopIdx[i], true, px, py)) continue;
			otherMoves (s, moves, true, px, py, bishopIdx[i], bishopIdx[i]);
			for(int j = 0; j < moves.size(); j++) 
				if(moves[j].second == nextPos) 
					return make_pair(bishopIdx[i], nextPos);
		}
	}
	else if((i == 10) || (i == 15)) {		// For knights
		for(int i = 0; i < 2; i++) {
			moves.clear();
			if(!getPieceLocation(s, knightIdx[i], true, px, py)) continue;
			knightMoves (s, moves, true, px, py, knightIdx[i]);
			for(int j = 0; j < moves.size(); j++) {
				// printf("i: %d, j: %d/%d, move to: (%d, %d) [%d]\n", i, j, moves.size(), moves[j].second/8, moves[j].second%8, moves[j].second);
				if(moves[j].second == nextPos) 
					return make_pair(knightIdx[i], nextPos);
			}
		}
	}
	else if(i == 12) {
		if(!getPieceLocation(s, 12, true, px, py)) {
			printf("Impossible move. Try again.\n");
			return processInput(s);
		}
		otherMoves (s, moves, true, px, py, 12, 12);
		for(int j = 0; j < moves.size(); j++) 
			if(moves[j].second == nextPos) 
				return make_pair(12, nextPos);
	}
	else if(i == 13) {
		assert(getPieceLocation(s, 13, true, px, py));
		otherMoves (s, moves, true, px, py, 13, 13);
		for(int j = 0; j < moves.size(); j++) 
			if(moves[j].second == nextPos) 
				return make_pair(13, nextPos);
	}


	printf("Impossible move. Try again.\n");
	return processInput(s);
}

/* ******************************************************************************************** */
int main (int argc, char* argv[]) {

	// Initialize the board
	srand(time(NULL));
	State state, state2;
	initBoard(state);
	printBoard(state);

	while(true) {

		// Get user input
		pair <int, int> move = processInput(state);

		// Make the user move
		makeMove(state, move, state2);
		printBoard(state2);

		// Let compute make a min-max move
		if(true) {
			pair <int,int> bestMove;
			numStates = 0;
			minMax (&state2, false, bestMove);
			// printf("%d -> %d\n", bestMove.first, bestMove.second);
			printf("#states examined: %d\n", numStates);
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
