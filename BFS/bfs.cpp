/**
 * @file bfs.cpp
 * @author Can Erdogan
 * @date July 12, 2015
 * @brief Implementation of breadth-first search for the 8-puzzle example in Russell & Norvig AI 
 * book. Start state: 7 2 4; 5 N 6; 8 3 1. Goal state: N 1 2; 3 4 5; 6 7 8.
 * NOTE: Finding the path. With recursive implementation, because we visit earlier states, 
 * the path information is preserved in the stack memory. In iterative, we need to allocate
 * additional memory for this information.
 */

#include <assert.h>
#include <iostream>
#include <math.h>
#include <queue>
#include <set>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

using namespace std;

bool debugVis = false;			///< Visualize the search with graphviz
#define digit(number,n) ((number/((int) pow(10, n)))%10)	 ///< Returns the nth digit of a #

/* ******************************************************************************************** */
struct State {
	int depth;		
	int actionBefore;
	int val;
	State* prev;
	State (int d, int a, int v, State* p) : depth(d), actionBefore(a), val(v), prev(p) {}
};
queue <State*> states; 	/// A state is a 9 digit number with 0 representing the empty cell
vector <int> actions;
int offsets [4] = {-3, -1, 3, 1};

/* ******************************************************************************************** */
inline int applyAction (int val, int index, int actionIdx) {

	static const bool dbg = 0;
	int offset = offsets[actionIdx];
	int new_index = index + offset;
	if(dbg) printf("%d vs. %d, %d, idx: %d\n", index, new_index, offset, actionIdx);
	assert((actionIdx >= 0) && (actionIdx <= 3));
	if((new_index < 0) | (new_index > 8)) {
		if(dbg) printf("\treason 1\n");
		return -1;
	}
	if(((offset == 1) || (offset == -1)) && ((index / 3) != (new_index / 3))) {
		if(dbg) printf("\treason 2\n");
		return -1;
	}

	// Get the digit that is being moved to the empty cell
	int tenthPowerNew = ((int) pow(10, new_index));
	int digitNew = (val / tenthPowerNew) % 10;

	// Create the new val by removing the new digit value 
	int tenthPowerOld = ((int) pow(10, index));
	int new_val = val - digitNew * (tenthPowerNew - tenthPowerOld);
	return new_val;
}

/* ******************************************************************************************** */
/// Generates new states in the search tree from the input and pushes them onto the stack
void generateStates (State& state) {

	// Find the location of the empty cell
	int index = -1;
	int val = state.val;
	int val_ = val;
	for(size_t i = 0; i < 9; i++) {
		if(val_ % 10 == 0) {
			index = i;
			break;
		}
		val_ /= 10;
	}
	
	// Generate the new val based on the location
	for(size_t n = 0; n < 4; n++) {

		// Get the possible neighbor and see if it exists
		int new_index = index + offsets[n];
		// printf("%d vs. %d\n", index, new_index);
		if((new_index < 0) | (new_index > 8)) continue;
		if(((offsets[n] == 1) || (offsets[n] == -1)) && ((index / 3) != (new_index / 3))) continue;

		// Get the digit that is being moved to the empty cell
		int tenthPowerNew = ((int) pow(10, new_index));
		int digitNew = (val / tenthPowerNew) % 10;
	
		// Create the new val by removing the new digit value 
		int tenthPowerOld = ((int) pow(10, index));
		int new_val = val - digitNew * (tenthPowerNew - tenthPowerOld);
		
		// Push to the stack
		// printf("\t%09d\n", new_val);
		states.push(new State(state.depth + 1, n, new_val, &state));
	}
}

/* ******************************************************************************************** */
void printStack() {
	while(!states.empty()) {
		int s = states.front()->val;
		states.pop();
		printf("%09d\n", s);
	}
}

/* ******************************************************************************************** */
void printState (int s) {
	int a = (s / ((int) 1e6));
	int b = ((s / ((int) 1e3))) % 1000;
	int c = s % 1000;
	printf("%03d\n%03d\n%03d\n", a, b, c);
}

/* ******************************************************************************************** */
/// Implements DFS and outputs the path of actions that reaches the goal state if one exists
/// Returns true if the goal is reached; false, otherwise.
bool dfs (State& s0) {

	// Keep track of seen states to avoid loops
	set <int> seen;
	seen.insert(s0.val);
	
	// Populate stack using initial state
	generateStates(s0);
	static const bool dbg = 0;

	// Keep searching until there is children states
	while(!states.empty()) {

		// Get the child state
		State* state = states.front();	
		int val = state->val;
		states.pop();
		
		// Debug information
		if(dbg) {
			printf("=== level: %d ======> \n", state->depth);
			printState(val);
			// getchar();
		}

		// Check if the child is seen
		if(seen.find(val) != seen.end()) {
			if(dbg) printf(">>>>> seen before.\n");
			continue;
		}
		seen.insert(val);

		// Check if the child is the goal
		if(val == 12345678) {
			State* s = state;
			while(s != NULL) {
				if(s->actionBefore != -1) actions.push_back(s->actionBefore);
				s = s->prev;
			}
			printf("# of actions: %d\n", actions.size());
			return true;
		}

		// Generate grandchildren
		generateStates(*state);
		if(dbg) printf(">>>>> not yet.\n");
	} 
	
	assert(false && "NO SOLUTION\n");
	return false;
}

/* ******************************************************************************************** */
void applyActions (int s) {

	printf("\nInitial state:\n");
	printState(s);

	int val = s;
	int step = actions.size() / 10;
	for(int act_idx = actions.size()-1; act_idx >= 0; act_idx--) {

		
		// Find where the empty cell is
		int index = -1;
		int val_ = val;
		for(size_t i = 0; i < 9; i++) {
			if(val_ % 10 == 0) {
				index = i;
				break;
			}
			val_ /= 10;
		}
	
		// Apply the action
		int val2 = applyAction(val, index, actions[act_idx]);
		val = val2;
		assert(val != -1);
		if(act_idx < 10) {
			printf("\nState %d:\n", actions.size() - act_idx);
			printState(val);
		}
	}

	printf("\nFinal state:\n");
	printState(val);
}

/* ******************************************************************************************** */
int main (int argc, char* argv[]) {

	// Parse the input
	if((argc > 1) && (strcmp(argv[1], "-d") == 0)) debugVis = true;

	// Perform DFS
	int s0 = 724506831;
	State state0 (0,-1,s0,NULL);
	dfs(state0);

	// Apply actions to the initial state
	applyActions(s0);
}
