/**
 * @file decisionTrees.cpp
 * @author Can Erdogan
 * @date July 17, 2015
 * @brief Implementation of decision-trees from example in Figure 18.3 in Russell & Norvig AI 
 * book. 
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

map <string, vector <string> > attrs;
vector < vector <string> > examples;

/* ********************************************************************************************* */
void readData () {

	// Open the file
	ifstream file ("data.txt");
	assert(file.is_open()); 

	// Read the attributes; parse each line
	char line [256];
	string attr;
	vector <string> values;
	file.getline(line, 256);
	file.getline(line, 256);
	for(int i = 0; i < 10; i++) {

		// Get the next line
		file.getline(line, 256);
		
		// Tokenize the line
		int counter = 0;
		char *p = strtok(line, "| {},[]");
		while (p) {

			// Differentiate between the attribute and its values
			if(counter == 0) attr = string(p);
			else values.push_back(p);	

			// Tokenize
			p = strtok(NULL, "| {},[]");
			counter++;
		}	

		// Save the attribute and its values
		attrs[attr] = values;
		values.clear();
	}

	// Get the examples
	for(int i = 0; i < 4; i++) file.getline(line, 256);
	vector <string> words;
	for(int i = 0; i < 12; i++) {
		file.getline(line, 256);
		char *p = strtok(line, "| {},[]");
		while (p) {
			words.push_back(p);	
			p = strtok(NULL, "| {},[]");
		}	
		examples.push_back(words);
		words.clear();
	}
}

/* ********************************************************************************************* */
void createTree () {


}

/* ********************************************************************************************* */
int main (int argc, char* argv[]) {

	// Read the data
	readData();

	// Recursively create the tree
	createTree();
}
/* ********************************************************************************************* */
