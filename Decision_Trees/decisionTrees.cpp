/**
 * @file decisionTrees.cpp
 * @author Can Erdogan
 * @date July 17, 2015
 * @brief Implementation of decision-trees from example in Figure 18.3 in Russell & Norvig AI 
 * book. 
 */

#include <assert.h>
#include <iostream>
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

vector <string> attrNames;
vector <vector <string> > attrs;
vector < vector <string> > examples;
FILE* graphFile;

/* ********************************************************************************************* */
struct Node {
	int index;
	int label;
	int nCount;
	vector <Node*> children;
	Node (int count, int i = -1, int l = -1) : index(i), label(l) {
		static int bla = 0;
		nCount = bla++;
//		if(count != 0) children = vector <Node*> (count, NULL);
	}
};

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
		attrNames.push_back(attr);
		attrs.push_back(values);
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
double computeGain (const vector <vector <string> >& examples, int attr) {

	bool dbg = false;
	if(attr == 3 || attr == 9) dbg = 0;

	// Count the number of positive and negative examples for each value of the given attribute
	vector <string>& values = attrs[attr];
	vector <int> pos_counts (values.size(), 0), neg_counts (values.size(), 0);
	for(int i = 0; i < examples.size(); i++) {
		const vector <string>& ex = examples[i];
		for(int j = 0; j < values.size(); j++) {
			if(ex[attr].compare(values[j]) == 0) {
				if(ex.back().compare("Yes") == 0) pos_counts[j]++;
				else neg_counts[j]++;
				break;
			}
		}
	}

	// Compute the "Remainder" after the tree is created using attribute as the root
	// Before log2(0) call if pos/neg counts are 0
	double remainder = 0.0;
	for(int i = 0; i < values.size(); i++) {
		int pos = pos_counts[i], neg = neg_counts[i]; 
		if(dbg) printf("\t'%s': %d vs. %d\n", values[i].c_str(), pos, neg);
		double total = pos + neg, ratio = total / 12.0;
		double part1 = 0.0, part2 = 0.0;
		if(pos != 0) part1 = -(pos/total) * log2(pos/total);
		if(neg != 0) part2 = -(neg/total) * log2(neg/total);
		double temp = ratio * (part1 + part2);
		remainder += temp; 
	}

	// Return the gain (note: 1.0 because 6/6 pos/neg examples in total in dataset)
	return (1.0 - remainder);
}

/* ********************************************************************************************* */
Node* createTree (const set <int>& activeAttrs, const vector < vector <string> >& examples, bool defaultLabelPos) {

	static const bool dbg = 0;
	if(dbg) printf("\n\n%d, %d ========================================================\n", activeAttrs.size(), examples.size());
	
	// Check for end cases
	if(activeAttrs.empty() || examples.empty()) return new Node(0, 0, defaultLabelPos);

	// Check if all the examples have the same classification
	int numPos = 0, numNeg = 0, labelPos;
	for(int i = 0; i < examples.size(); i++) {
		const vector <string>& ex = examples[i];
		if(dbg) {
			printf("ex %d: ");
			for(int j = 0; j < ex.size(); j++) printf("'%s' ", ex[j].c_str());
			printf("\n");
		}
		if(ex.back().compare("Yes") == 0) numPos++;
		else numNeg++;
	}
	if(numPos == 0 || numNeg == 0) {
		return new Node (0, 0, numPos == 0 ? false : true);
	}
	if(numNeg > numPos) labelPos = false;

	// Choose the attribute with the most gain
	double maxGain = -1.0;
	int bestAttr = 0;
	for(int i = 0; i < attrs.size(); i++) {
		if(activeAttrs.find(i) == activeAttrs.end()) continue;
		double gain = computeGain(examples, i);
		if(dbg) printf("'%s': %lf\n", attrNames[i].c_str(), gain);
		if(gain > maxGain) {
			maxGain = gain;
			bestAttr = i;
		}
	}

	// Remove this attribute from children's search
	set <int> newAttrs (activeAttrs.begin(), activeAttrs.end());
	newAttrs.erase(bestAttr);

	// Create the tree
	vector <string>& values = attrs[bestAttr];
	Node* root = new Node (values.size(), bestAttr);
	for(int i = 0; i < values.size(); i++) {

		// Get the subset of examples that has this value
		vector < vector <string> > newExamples; 
		for(int j = 0; j < examples.size(); j++) {
			const vector <string>& ex = examples[j];
			if(ex[bestAttr].compare(values[i]) == 0) newExamples.push_back(ex);
		}
			
		// Create the subtree	
		root->children.push_back(createTree(newAttrs, newExamples, labelPos));
	}

	return root;
}

/* ********************************************************************************************* */
void printTree (Node* root, int level) {

	if(root->label != -1) fprintf(graphFile, "%s%d [label=%s];\n", root->label == 1 ? "Yes" : "No", 
		root->nCount, root->label == 1 ? "Yes" : "No");
	vector <string>& attrs_ = attrs[root->index];
	for(int i = 0; i < root->children.size(); i++) {
		if(root->children[i]->label == -1)
			fprintf(graphFile, "%s -- %s [label=%s];\n", attrNames[root->index].c_str(), 
				attrNames[root->children[i]->index].c_str(), attrs_[i].c_str());
		else 
			fprintf(graphFile, "%s -- %s%d [label=%s];\n", attrNames[root->index].c_str(), 
				root->children[i]->label == 1 ? "Yes" : "No", root->children[i]->nCount, attrs_[i].c_str());
		printTree(root->children[i], level+1);
	}
}

/* ********************************************************************************************* */
int main (int argc, char* argv[]) {

	// Read the data
	readData();

	// Recursively create the tree
	set <int> newAttrs;
	for(int i = 0; i < 10; i++) newAttrs.insert(i);
	Node* root = createTree(newAttrs, examples, 0);

	// Draw the tree
	graphFile = fopen("graph.dot", "w+");
	fprintf(graphFile, "graph {\n");
	printTree(root, 0);
	fprintf(graphFile, "}\n");
	fclose(graphFile);
	system("dot -Tpng graph.dot -o graph.png");
}
/* ********************************************************************************************* */
