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
#include <map>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

using namespace std;

#define ps(x) cout << #x << ": '" << (x).c_str() << "'\n";

/* ******************************************************************************************** */
/// Action definition
struct Action {
	string name;  ///< The name of the action
	size_t numObjects;  ///< The number of objects that take part in this action
	vector <string> pres;  ///< The preconditions
	vector <string> adds;  ///< The add effects as the action applied
	vector <string> dels;  ///< The delete effects as the action applied
	string args;
	map <string, string> objs;

	/// Prints action name
	string s_ () const {

		// Print the name 
		char buf [256], temp [64];
		sprintf(buf, "%s", name.c_str());

		// Print the arguments
		if(objs.size() > 0) {
			sprintf(buf, "%s(", buf);
			for(int i = 0; i < numObjects; i++) {
				sprintf(temp, "arg%d", i+1);
				map <string, string>::const_iterator it = objs.find(temp);
				if(it != objs.end()) sprintf(buf, "%s%s,", buf, it->second.c_str());
				else sprintf(buf, "%s%s,", buf, temp);
			}
			buf[strlen(buf)-1] = '\0';
			sprintf(buf, "%s)", buf);
		}

		return string(buf);
	}
};

vector <Action*> domainActions;

/* ******************************************************************************************** */
/// Causal relationship between two actions due to the precondition
struct Causal {
	const Action* pre;
	const Action* post;
	string cause;
	Causal (const Action* f, const Action* s, string c) : pre(f), post(s), cause(c) {}
};

/* ******************************************************************************************** */
/// Set of literals that define a state
struct Plan {
	set <const Action*> actions; //< may be uninstantiated	
	vector <Causal> causals;
	set <pair <const Action*, const Action*> > orderings;
	map <string, const Action*> unachieved;
	Plan (const Plan& p) : unachieved(p.unachieved), actions(p.actions), causals(p.causals),
		orderings(p.orderings) {}
	Plan () {}
};

/* ******************************************************************************************** */
/// Set the preconditions, add and delete effects of an action
Action* declareAction(const string& name, size_t numObjects, const string& arguments) {

	// Instantiate the action
	Action* act = new Action();
	act->name = name;
	act->numObjects = numObjects;
	act->args = arguments;

	// Parse the arguments to get preconditions/add-effects/delete-effects
	int preIndex = 0, postIndex = 0;;
	vector <string>* lists [] = {&act->pres, &act->adds, &act->dels};
	for(int i = 0; i < 3; i++) {
		postIndex = arguments.find('!', preIndex);
		istringstream str (arguments.substr(preIndex, postIndex - preIndex));
		for(string temp; getline(str, temp, '|'); ) lists[i]->push_back(temp);
		preIndex = postIndex+1;
	}

	// Create the uninstantiated actions
	for(int i = 0; i < numObjects; i++) {
		char buf [8];
		sprintf(buf, "arg%d", i+1);
		act->objs[string(buf)] = buf;
	}

	printf("Created action: '%s'\n", act->s_().c_str());

	// Create and return the action
	return act;
}

/* ******************************************************************************************** */
/// Shopping domain
void setup (Plan& p) {

	// Create the actions
	domainActions.push_back(declareAction("buy", 2, "At(arg2)|Sells(arg2,arg1)!Have(arg1)!"));
	domainActions.push_back(declareAction("go", 2, "At(arg1)!At(arg2)!At(arg1)"));

	// Create the start and final states
	Action* first = declareAction("start", 0, 
		"!At(Home)|Sells(SuMa,Milk)|Sells(SuMa,Banana)|Sells(HaSt,Drill)!");
	Action* final = declareAction("final", 0, "Have(Milk)|Have(Banana)|Have(Drill)!!");

	// Set the unachieved actions
	for(int i = 0; i < final->pres.size(); i++) 
		p.unachieved.insert(make_pair(final->pres[i] + "|" + final->s_(), final));
	p.orderings.insert(make_pair(first, final));
	p.actions.insert(first);
	p.actions.insert(final);
}

/* ******************************************************************************************** */
/// Returns the objects of a literal
void getObjects (const string& lit, vector <string>& objs) {
	objs.clear();
	string middle = lit.substr(lit.find("("));
	int prev = -1;
	for(int i = 0; i < middle.size(); i++) {
		if((middle[i] == '(') || (middle[i] == ')') || (middle[i] == ',')) {
			if(prev == -1) prev = i;
			else {
				objs.push_back(middle.substr(prev + 1, i - prev - 1));
				prev = i;
			}
		}
	}
}

/* ******************************************************************************************** */
/// After an ordering b < c is given, we update the orderings such that for any a < b, a < c is
/// added and for any c < d, b < d is added.
void orderingImplications (set <pair <const Action*, const Action*> >& orderings, const 
	Action* pre, const Action* post) {

	bool dbg = 0;
	if(dbg) printf("\n\n\n........................................................\n");
	if(dbg) printf("pre: %s, post: %s\n", pre->s_().c_str(), post->s_().c_str());
	set <pair <const Action*, const Action*> > newOnes;
	int i = 0;
	for(set <pair <const Action*, const Action*> >::const_iterator it =  orderings.begin();
			it != orderings.end(); it++) {

		if(dbg) printf("ordering %d: %s <- %s\n", i++, it->first->s_().c_str(), it->second->s_().c_str());

		// We have b < c: for a given a < b, add a < c
		if(it->second == pre) {	
			if(dbg) printf("\tpreCase: %s <- %s\n", it->first->s_().c_str(), post->s_().c_str());
			newOnes.insert(make_pair(it->first, post));
		}
		
		// We have b < c: for a given c < d, add b < d
		if(it->first == post) {
			if(dbg) printf("\tpostCase: %s <- %s\n", pre->s_().c_str(), it->second->s_().c_str());
			newOnes.insert(make_pair(pre, it->second));
		}
	}

	// Add the new ones to the current list
	if(!newOnes.empty())
		for(set <pair <const Action*, const Action*> >::const_iterator it =  orderings.begin();
			it != orderings.end(); it++) 
			orderings.insert(make_pair(it->first, it->second));
}

/* ******************************************************************************************** */
bool resolveThreats (const Plan& p) {

	printf("vvvvvvvvvvvvvvvvvvv %s vvvvvvvvvvvvvv \n", __FUNCTION__);

	// For each action in the plan, see if it threatens any causal links
	set <const Action*>::const_iterator actIt = p.actions.begin(); 
	for(; actIt != p.actions.end(); actIt++) {

		// Check all the causal links
		for(size_t caus_idx = 0; caus_idx < p.causals.size(); caus_idx++) {

			// A threat is not possible if this action is after the "post action" or before the "pre"
			const Causal& causal = p.causals[caus_idx];
			bool afterPost = false, beforePre = false;
			for(set <pair <const Action*, const Action*> >::const_iterator it =  p.orderings.begin();
					it != p.orderings.end(); it++) {
				if((causal.post == it->first) && (*actIt == it->second)) {
					afterPost = true;
					break;
				}
				if((causal.pre == it->second) && (*actIt == it->first)) {
					beforePre = true;
					break;
				}
			}
			if(afterPost || beforePre) continue;

			// Look at the delete effects of the action
			bool threat = false;
			for(size_t del_idx = 0; del_idx < (*actIt)->dels.size(); del_idx++) {
				if(causal.cause.compare((*actIt)->dels[del_idx]) == 0) {
					threat = true;
					break;
				}
			}
			if(!threat) continue;
			
			// See if a promotion (actIt < pre) or demotion (post < actIt) is possible
			bool promotionPossible = true, demotionPossible = true; 
			for(set <pair <const Action*, const Action*> >::const_iterator it =  p.orderings.begin();
					it != p.orderings.end(); it++) {
				if((causal.pre == it->first) && (*actIt == it->second)) 
					promotionPossible = false;
				if((causal.post == it->second) && (*actIt == it->first)) 
					demotionPossible = false;
			}
			
			// Create a promotion if possible
			if(promotionPossible) {
				Plan p2 (p);
				p2.orderings.insert(make_pair(*actIt, causal.pre));
				orderingImplications(p2.orderings, *actIt, causal.pre);
				return resolveThreats(p2);
			}
			else if(demotionPossible) {
				Plan p2 (p);
				p2.orderings.insert(make_pair(causal.post, *actIt));
				orderingImplications(p2.orderings, causal.post, *actIt);
				return resolveThreats(p2);
			}
			else {
				printf("%s: returning false\n", __FUNCTION__);
				return false;
			}
		}
	}
	
	printf("%s: returning true\n", __FUNCTION__);
	return true;
}

/* ******************************************************************************************** */
/// Updates the causals, orderings, unachieved and action list in the plan
/// Previous objects and objects belong to the literal that is being targered for instantiation
/// For instance: At(arg1) -> At(home) => ({arg1}, {home}) 
Action* instantiateAction (Plan& p, const Action* act, const map <string, string>& replacements) {

	// Print input data
	printf("Replacements: {\n");
	map <string, string>::const_iterator r_it = replacements.begin();
	for(; r_it != replacements.end(); r_it++) 
		printf("\t'%s' -> '%s'\n", r_it->first.c_str(), r_it->second.c_str());
	printf("}\n");

	// Create the argument list for the new action
	map <string, string> newObjs = act->objs;
	string newArgs = act->args;
	for(r_it = replacements.begin(); r_it != replacements.end(); r_it++) {

		// Update the instances list of the action
		string old_ = r_it->first, new_ = r_it->second;
		if(newObjs.find(old_) != newObjs.end()) newObjs[old_] = new_;

		// Update the argument string for creating the action
		int index = newArgs.find(old_);
		while(index != string::npos) {
			newArgs.replace(index, old_.size(), new_);
			index = newArgs.find(old_, index+1);
		}
	}

	// Create the instantiated action instance
	Action* instAct = declareAction(act->name, act->numObjects, newArgs);
	instAct->objs = newObjs;

	// Update the causals
	for(size_t i = 0; i < p.causals.size(); i++) {
		if(p.causals[i].pre == act) {		
			p.causals[i].pre = instAct;
			for(r_it = replacements.begin(); r_it != replacements.end(); r_it++) 
				if(p.causals[i].cause.find(r_it->first) != string::npos) 
					p.causals[i].cause.replace(p.causals[i].cause.find(r_it->first), r_it->first.size(), r_it->second);
		}
		if(p.causals[i].post == act) {
			p.causals[i].post = instAct;
			for(r_it = replacements.begin(); r_it != replacements.end(); r_it++) 
				if(p.causals[i].cause.find(r_it->first) != string::npos) 
					p.causals[i].cause.replace(p.causals[i].cause.find(r_it->first), r_it->first.size(), r_it->second);
		}
	}

	// Update the orderings
	for(set <pair <const Action*, const Action*> >::const_iterator it = p.orderings.begin();
			it != p.orderings.end(); ) {
		if(it->first == act) {
			set <pair <const Action*, const Action*> >::const_iterator it2 = it;
			p.orderings.insert(make_pair(instAct, it2->second));
			p.orderings.erase(it2);
			++it; 
		}
		else if(it->second== act) {
			set <pair <const Action*, const Action*> >::const_iterator it2 = it;
			p.orderings.insert(make_pair(it2->first, instAct));
			p.orderings.erase(it2);
			++it; 
		}
		else ++it;
	}

	// Update unachieved literals
	map <string, const Action*>::iterator it = p.unachieved.begin();
	int counter = 0;
	while(it != p.unachieved.end()) {
		if(it->second == act) {
			string unachieved = it->first.substr(0,it->first.find("|"));
			printf("unachieved %d: '%s'\n", counter, unachieved.c_str());
			map <string, string>::const_iterator r_it = replacements.begin();
			for(; r_it != replacements.end(); r_it++) 
				if((unachieved.find(r_it->first) != string::npos)) 
					unachieved.replace(unachieved.find(r_it->first), r_it->first.size(), r_it->second);
			std::map<string, const Action*>::iterator toErase = it;
			++it;
			p.unachieved.erase(toErase);
			p.unachieved.insert(make_pair(unachieved+"|"+instAct->s_(), instAct));
			printf("Attempting to insert: '%s'\n", (unachieved+"|"+instAct->s_()).c_str());
		}
		else ++it;
		counter++;
	}

	return instAct;
}

/* ******************************************************************************************** */
bool tryAction (const Plan& p, const Action* act, pair<string,const Action*> prev, Plan& p2,
		bool newAction = false) {

	printf("vvvvvvvvvvvvvvvvvvv %s vvvvvvvvvvvvvv \n", __FUNCTION__);
	printf("\t\tcalled with: %s: %s\n", act->name.c_str(), act->args.c_str());

	// Get the previous action information
	string subgoal = (prev.first).substr(0, prev.first.find("|"));
	ps(subgoal);
	const Action* prevAct = prev.second;
	string subgoalName = subgoal.substr(0, subgoal.find("("));
	vector <string> subgoalObjs;
	getObjects(subgoal, subgoalObjs);
	
	// Look at the add effects of this action
	vector <string> objs;
	for(int i = 0; i < act->adds.size(); i++) {

		printf("\tadd effect %d: ", i); ps(act->adds[i]);

		// Check if the literal is the same type 
		if(act->adds[i].substr(0, act->adds[i].find("(")).compare(subgoalName) == 0) {

			// If the subgoal literal doesn't have objects, you are done; select the action
			if(subgoalObjs.empty()) {

				// Add a causal link between the previous action and this one
				p2 = Plan (p);
				if(newAction) {
					p2.actions.insert(act);
				}
				p2.causals.push_back(Causal(act, prevAct, subgoal));
				p2.orderings.insert(make_pair(act, prevAct));
				orderingImplications(p2.orderings, act, prevAct);
				printf("No objects: Adding action: %s: %s\n", act->name.c_str(), act->args.c_str());

				// See if the threats can be resolved and a plan can be constructed after this choice
				if(resolveThreats(p2)) return true;
				else continue;
			}

			// Check if there are any conflicts with instantiated objects
			bool conflicts = false;
			getObjects(act->adds[i], objs);
			assert(subgoalObjs.size() == objs.size() && "Number of literal objects don't match");
			for(int j = 0; j < subgoalObjs.size(); j++) {

				// Check if either of the objects are uninstantiated
				bool free1 = (subgoalObjs[j].size() > 3) && (subgoalObjs[j].substr(0,3).compare("arg") == 0);
				bool free2 = (objs[j].size() > 3) && (objs[j].substr(0,3).compare("arg") == 0);
		
				// Check for conflict
				if(!free1 && !free2 && (subgoalObjs[j].compare(objs[j]) != 0)) {
					conflicts = true;
					break;
				}
			}
		
			// If there are no conflicts, instantiate free variables of chosen action to the subgoal's needs
			map <string, string> newObjs;
			if(!conflicts) {

				// Generate the list of replacements for free variables
				map <string, string> replacements;
				for(int i = 0; i < subgoalObjs.size(); i++) {
					if(objs[i].substr(0,3).compare("arg") == 0)
						replacements.insert(make_pair(objs[i], subgoalObjs[i]));
					else if(subgoalObjs[i].substr(0,3).compare("arg") == 0)
						replacements.insert(make_pair(subgoalObjs[i], objs[i]));
				}

				// Replace the chosen action with the instantiated action version
				p2 = Plan (p);
				printf("curr >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
				Action* instAct = instantiateAction(p2, act, replacements);
				p2.actions.insert(instAct);
				
				// Instantiate the previous action
				printf("prev >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
				Action* prevInstAct = instantiateAction(p2, prevAct, replacements);
				p2.actions.insert(prevInstAct);
				p2.actions.erase(prevAct);
				printf("prevInstAct: '%s'\n", prevInstAct->s_().c_str());

				// Also instantiate the other unachieved goals of the predecessor action 
				// map <string, const Action*>::iterator it = p2.unachieved.begin();
				// while(it != p2.unachieved.end()) {
				// 	if(it->second == prevAct) {
				// 		string unachieved = it->first.substr(0,it->first.find("|"));
				// 		if(unachieved.compare(subgoal) == 0) {
				// 			++it;
				// 			continue;
				// 		}
				// 		printf("unachieved: '%s', action: %s\n", unachieved.c_str(), prevAct->s_().c_str());
				// 		for(int j = 0; j < subgoalObjs.size(); j++) {			// replace the free argument if one exists
				// 			bool free2 = (subgoalObjs[j].size() > 3) && (subgoalObjs[j].substr(0,3).compare("arg") == 0);
				// 			printf("\tsubgoalObjs[%d]: %s, free2: %d, objs[%d]: %s\n", j, subgoalObjs[j].c_str(), free2, j, objs[j].c_str());
				// 			if(free2 && unachieved.find(subgoalObjs[j]) != string::npos) {
				// 				printf("here: "); ps(unachieved); fflush(stdout);
				// 				unachieved.replace(unachieved.find(subgoalObjs[j]), objs[j].size(), objs[j]);
				// 			}
				// 		}
				// 		std::map<string, const Action*>::iterator toErase = it;
				// 		++it;
				// 		p2.unachieved.erase(toErase);
				// 		p2.unachieved.insert(make_pair(unachieved+"|"+prevAct->s_(), prevAct));
				// 	}
				// 	else ++it;
				// }

				// Add it to the list and update unachieved preconditions
				if(newAction) {
					for(int i = 0; i < instAct->pres.size(); i++) 
						p2.unachieved.insert(make_pair(instAct->pres[i] + "|" + instAct->s_(), instAct));
				}

				// Add causality and ordering
				p2.causals.push_back(Causal(instAct, prevInstAct, subgoal));
				p2.orderings.insert(make_pair(instAct, prevInstAct));
				orderingImplications(p2.orderings, instAct, prevInstAct);

				// If already exists in the plan in an uninstantiated form, update it
				if(!newAction) {
					p2.actions.erase(act);
				}
				
				// See if the threats can be resolved and a plan can be constructed after this choice
				printf("With objects: Adding action: %s\n", instAct->s_().c_str());
				if(resolveThreats(p2)) return true;
				else continue;
			}
		}
	}

	printf("%s: returning false\n", __FUNCTION__);
	return false;
}

/* ******************************************************************************************** */
void printGraph (const Plan& p) {

	FILE* graphFile = fopen("graph.dot", "w+");
	fprintf(graphFile, "digraph {\n");
	for(set <pair <const Action*, const Action*> >::const_iterator it = p.orderings.begin();
			it != p.orderings.end(); it++) {
		const Action* act1 = it->first, *act2 = it->second;
		fprintf(graphFile, "\"%s\" -> \"%s\"\n", act1->s_().c_str(), act2->s_().c_str());
		// printf("\"%s\" -> \"%s\"\n", act1->s_().c_str(), act2->s_().c_str());
	}
	fprintf(graphFile, "}\n");
	fclose(graphFile);
	system("dot -Tpng graph.dot -o graph.png");

}

/* ******************************************************************************************** */
bool search (const Plan& p_) {

	printf("vvvvvvvvvvvvvvvvvvv %s vvvvvvvvvvvvvv \n", __FUNCTION__);

	Plan p (p_), p2;
	while(true) {

		// Print state as a graph
		printGraph(p);
		map <string, const Action*>::iterator it = p.unachieved.begin();
		printf("\n\n\n\n\n\n\nUnachieved size: %d\n", p.unachieved.size());
		for(int i = 0; it != p.unachieved.end(); it++, i++) {
			printf("%d: %s due %s\n", i, it->first.c_str(), it->second->s_().c_str());
		}

		// If all the actions are satisfied, return
		if(p.unachieved.empty()) return true;
		getchar();
		
		// Choose a subgoal of any action in the plan that is not yet satisfied
		size_t numUnachieved = p.unachieved.size();
		map <string, const Action*>::const_reverse_iterator randIt = p.unachieved.rbegin();
		// advance(randIt, rand() % numUnachieved);
		printf("\n\nChosen subgoal: %s\n", randIt->first.c_str());
		printf("\tprevAct: : %s: %s\n", randIt->second->name.c_str(), randIt->second->args.c_str());

		// Check if any of the actions already in the plan can satisfy this subgoal
		bool addressedGoal = false;
		for(set <const Action*>::const_iterator it = p.actions.begin(); it != p.actions.end(); it++) {

			// Look at the add conditions of the action
			const Action* act = *it;
			printf("\nTrying old plan action.\n");
			if(tryAction(p, act, *randIt, p2, false)) {
				addressedGoal = true;
				p2.unachieved.erase(randIt->first);
				p = p2;
				break;
			}
		}
		if(addressedGoal) continue;

		// If no actions can be used from the current plan, choose a new action from the domain
		for(int i = 0; i < domainActions.size(); i++) {
			printf("\nTrying new domain action.\n");
			if(tryAction(p, domainActions[i], *randIt, p2, true)) {
				p2.unachieved.erase(randIt->first);
				addressedGoal = true;
				p = p2;
				break;
			}
		}
		
		if(!addressedGoal) return false;
	}

	return false;
}

/* ******************************************************************************************** */
int main (int argc, char* argv[]) {

	// Create the domain information
	Plan p;
	setup(p);

	// Perform the search
	// srand(time(NULL));
	bool result = search(p);
	printf("Result: %d\n", result);
}
/* ******************************************************************************************** */
