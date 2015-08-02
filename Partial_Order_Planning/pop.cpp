/**
 * @file pop.cpp
 * @author Can Erdogan
 * @date 2015-07-22
 * @brief Implements partial order planning.
 */

#include <assert.h>
#include <iostream>
#include <math.h>
#include <queue>
#include <set>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

using namespace std;

/* ******************************************************************************************** */
/// Action definition
struct Action {
	string name;  ///< The name of the action
	size_t numObjects;  ///< The number of objects that take part in this action
	vector <string> pres;  ///< The preconditions
	vector <string> adds;  ///< The add effects as the action applied
	vector <string> dels;  ///< The delete effects as the action applied
	vector <size_t> instantiatedObjects_;  ///< The objects in the instantiated version
};

vector <Action*> actions;

/// Set of literals that define a state
struct State : set <string> {
};


/* ******************************************************************************************** */
/// Set the preconditions, add and delete effects of an action
Action* declareAction(const string& name, size_t numObjects, const string& arguments) {

	// Instantiate the action
	Action* act = new Action();
	act->name = name;
	act->numObjects = numObjects;

	// Parse the arguments to get preconditions/add-effects/delete-effects
	int preIndex = 0, postIndex = 0;;
	vector <string>* lists [] = {&act->pres, &act->adds, &act->dels};
	for(int i = 0; i < 3; i++) {
		postIndex = arguments.find('!', preIndex);
		istringstream str (arguments.substr(preIndex, postIndex - preIndex));
		for(string temp; getline(str, temp, '|'); ) lists[i]->push_back(temp);
		preIndex = postIndex+1;
	}

	// Create and return the action
	return act;
}

/* ******************************************************************************************** */
/// Shopping domain
void createDomain () {

	// Create the actions
	actions.push_back(declareAction("buy", 2, "At(arg2)|Sells(arg2,arg1)!!Have(arg1)"));
	actions.push_back(declareAction("go", 2, "At(arg1)!At(arg2)!At(arg1)"));

	// Create the start and final states
	actions.push_back(declareAction("start", 2, 
		"!At(Home)|Sells(SM,Milk)|Sells(SM,Banana)|Sells(HW,Drill)!"));
	actions.push_back(declareAction("final", 2, "Have(Milk)|Have(Banana)|Have(Drill)!!"));
}

/* ******************************************************************************************** */
void search () {

}

/* ******************************************************************************************** */
int main (int argc, char* argv[]) {

	// Create the domain information
	createDomain();

	// Perform the search
}
/* ******************************************************************************************** */
