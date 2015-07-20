/**
 * @file csp.cpp
 * @author Can Erdogan
 * @date July 19, 2015
 * @brief Implementation of discrete constraint satisfaction for US map 4-coloring using 
 * heuristics such as min-conflict, least-constraint value, etc. 
 */

#include <assert.h>
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

/* ********************************************************************************************* */
struct Node {
	string name;
	vector <Node*> neighs;
	Node (string n) : name(n) {}
};

map <string, Node*> nodes;
map <Node*, int> colors;
set <Node*> unassigned;

/* ********************************************************************************************* */
void readFile () {

	// Open the file
	ifstream file ("data.txt");
	assert(file.is_open()); 

	// Read the states and their neighbors
	char line [256];
	for(int i = 0; i < 51; i++) {

		// Get the next line
		file.getline(line, 256);
		
		// Tokenize the line
		int counter = 0;
		char *p = strtok(line, "| {},[]");
		Node* node = NULL, *neigh = NULL;
		while (p) {

			// Create a node for the state if it doesn't already exist
			string sp (p);
			map <string, Node*>::iterator it = nodes.find(sp);
			if(it != nodes.end()) {
				if(counter == 0) {
					node = it->second;
					unassigned.insert(node);
				}
				else neigh = it->second;
			}
			else {
				Node* temp = new Node(sp);
				nodes[sp] = temp;
				if(counter == 0) {
					node = temp;
					unassigned.insert(node);
				}
				else neigh = temp;
			}

			// Record the central node
			if(counter != 0) node->neighs.push_back(neigh);

			// Tokenize
			p = strtok(NULL, "| {},[]");
			counter++;
		}	
	}
}

/* ********************************************************************************************* */
int counter = 0;
bool recursive_backtracking_search () {

//	printf("counter: %d\n", counter);

	// Check if complete
	if(colors.size() == 51) {
		// printf("Completed the colors; returning true!\n");
		return true;
	}
	
	// Select an unassigned variable
	//int random = (unassigned.size() < 2) ? 0 : (rand() % (unassigned.size() - 1));
	//set <Node*>::iterator itf = unassigned.begin();
	//advance(itf, random);
	//Node* var = *itf;
	//unassigned.erase(itf);
	set <Node*>::reverse_iterator itf = unassigned.rbegin();
	Node* var = *itf;
	unassigned.erase(*itf);
	// printf("var: %s\n", var->name.c_str());
	// getchar();

	// Attempt each color
	map <Node*, int>::iterator it;
	for(int color = 0; color < 4; color++) {

		counter++;

		// Check if color is consistent with immediate neighbors
		vector <Node*>& neighs = var->neighs;
		bool collision = false;
		for(int neigh_idx = 0; neigh_idx < neighs.size(); neigh_idx++) {
			it = colors.find(neighs[neigh_idx]);
			if((it != colors.end()) && (it->second == color)) {
				collision = true;
				break;
			}
		}
		if(collision) continue;
		
		// Make the assignment 
		colors[var] = color;

		// Attempt to solve the rest
		if(recursive_backtracking_search()) {
			// printf("Child returned true; returning true!\n");
			return true;
		}

		// If failed, remove the assignment and try another one
		colors.erase(var);
	}

	// printf("No assignments, returning false :(\n");
	unassigned.insert(var);
	return false;
}

/* ********************************************************************************************* */
int main (int argc, char* argv[]) {

	// Create the csp graph
	readFile();

	// Perform recursive backtracking search
	srand(time(NULL));
	bool res = recursive_backtracking_search ();
	printf("res: %d\n", res);

	// Draw the tree
	FILE* graphFile = fopen("graph.dot", "w+");
	fprintf(graphFile, "graph {\n");
	const char * const colorNames [] = {"red", "blue", "green", "yellow"};
	for(map<string,Node*>::iterator it = nodes.begin(); it != nodes.end(); it++) {
		fprintf(graphFile, "%s [style=filled fillcolor=%s]\n", it->second->name.c_str(), 
			colorNames[colors[it->second]]);
		for(int j = 0; j < it->second->neighs.size(); j++) {
			if(it->second->name.compare(it->second->neighs[j]->name) > 0) continue;
			fprintf(graphFile, "%s -- %s;\n", it->second->name.c_str(), 
				it->second->neighs[j]->name.c_str());
		}
		fprintf(graphFile, "\n");
	}
	fprintf(graphFile, "overlap=false; \nsplines=true;\n}\n");
	fclose(graphFile);
	system("dot -Tpng graph.dot -o graph.png");
	printf("%d\n", counter);
}
/* ********************************************************************************************* */
