/**
 * @file avl.cpp
 * @date 2015-08-07
 * @author Can Erdogan
 * @brief Implementation of the AVL trees.
 */

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <string>

using namespace std;

/* ******************************************************************************************** */
template <class X>
struct AVL {

  bool (*comp) (X,X);					///< Comparison function for the tree
  string (*toStr) (X);				///< Print function for the tree values
  AVL (bool (*comp_) (X, X), string (*print_) (X)) {	///< Constructor
    comp = comp_;
    toStr = print_;
		root = NULL;
  }

  struct Node {								///< Node representation
		Node* left, *right;
    Node* parent;
    X value;
		int height;
		Node (const X& v, Node* p) : value(v), parent(p), height(1), left(NULL), right(NULL) {}
  };  

	Node* root;									///< Root of the tree
	FILE* graphFile;

	// Basic utilities
  Node* search (const X& x); 	

 	/* ****************************************************************************************** */
	void draw () { 
		graphFile = fopen("graph.dot", "w+");
		fprintf(graphFile, "digraph {\n");
  	fprintf(graphFile, "graph [ordering=\"out\"]\n");
		if(root != NULL) {
			fprintf(graphFile, "\"%s (%d)\"\n", toStr(root->value).c_str(), root->height);
			draw(root); 
		}
		fprintf(graphFile, "}\n");
		fclose(graphFile);
		system("dot -Tpng graph.dot -o graph.png; eog graph.png &");
	}

 	/* ****************************************************************************************** */
	void draw (Node* curr) {
		string currS = toStr(curr->value);
		if(curr->left != NULL) {
			fprintf(graphFile, "\"%s (%d)\" -> \"%s (%d)\"\n", currS.c_str(), curr->height,
				toStr(curr->left->value).c_str(), curr->left->height);
			draw(curr->left);
		}
		else {
    	fprintf(graphFile, "\"null%s-0\" [shape=point];\n", currS.c_str());
    	fprintf(graphFile, "\"%s (%d)\" -> \"null%s-0\";\n", currS.c_str(), curr->height, 
				currS.c_str());
		}
		if(curr->right != NULL) {
			fprintf(graphFile, "\"%s (%d)\" -> \"%s (%d)\"\n", currS.c_str(), curr->height,
				toStr(curr->right->value).c_str(), curr->right->height);
			draw(curr->right);
		}
		else {
    	fprintf(graphFile, "\"null%s-1\" [shape=point];\n", currS.c_str());
    	fprintf(graphFile, "\"%s (%d)\" -> \"null%s-1\";\n", currS.c_str(), curr->height, 
				currS.c_str());
		}
	}

 	/* ****************************************************************************************** */
	void traversal () { traversal(root); }
	void traversal (Node* curr) {
		if(curr->left != NULL) traversal(curr->left);
		printf("'%s' ", toStr(curr->value).c_str());
		if(curr->right != NULL) traversal(curr->right);

	}

 	/* ****************************************************************************************** */
	void rotateLeft (Node* curr, Node* right) {

		printf("%s: %s, %s\n", __FUNCTION__, toStr(curr->value).c_str(), toStr(right->value).c_str());

		// Make the connection between (current's parent,right)
		right->parent = curr->parent;
		if(curr->parent) {
			if(curr->parent->left == curr) curr->parent->left = right;
			else curr->parent->right = right;
		}
		else root = right;

		// Make the connection between (current,right's left)
		if(right->left) right->left->parent = curr;
		curr->right = right->left;
		

		// Make the connection between (current,right)
		right->left = curr;
		curr->parent = right;

		// Update the heights of current and right, both of which have new children
		curr->height = max(curr->left ? curr->left->height : 0, 
			curr->right ? curr->right->height : 0) + 1;
		right->height = max(right->left ? right->left->height : 0, 
			right->right ? right->right->height : 0) + 1;
	} 

 	/* ****************************************************************************************** */
	void rotateRight (Node* curr, Node* left) {

		printf("%s: %s, %s\n", __FUNCTION__, toStr(curr->value).c_str(), toStr(left->value).c_str());
		printf("\tcurr height0: %d, left height0: %d\n", curr->height, left->height);

		// Make the connection between (current's parent,left)
		left->parent = curr->parent;
		if(curr->parent) {
			if(curr->parent->right == curr) curr->parent->right = left;
			else curr->parent->left = left;
		}
		else root = left;

		// Make the connection between (current,left's right)
		if(left->right) left->right->parent = curr;
		curr->left = left->right;

		// Make the connection between (current,left)
		left->right = curr;
		curr->parent = left;

		// Update the heights of current and left, both of which have new children
		curr->height = max(curr->right ? curr->right->height : 0, 
			curr->left ? curr->left->height : 0) + 1;
		left->height = max(left->right ? left->right->height : 0, 
			left->left ? left->left->height : 0) + 1;
		printf("\tcurr height: %d, left height: %d\n", curr->height, left->height);
	} 
	
 	/* ****************************************************************************************** */
	/// Balances the tree with rotations
	void retrace (Node* curr) {

		printf("Retrace for '%s'\n", toStr(curr->value).c_str());

		int counter = 0;
		while(curr != NULL) {
			if(counter++ > 10) break;

			// Look at the left rotations (current balance is +2)
			int currBalance = (curr->right ? curr->right->height : 0) -
				(curr->left ? curr->left->height : 0);
			printf("curr: '%s', balance: %d, height: %d\n", toStr(curr->value).c_str(), currBalance, 
				curr->height);

			if(currBalance > 1) {
	
				// Find out the children's balances
				Node* right = curr->right;
				int rightBalance = (right->right ? right->right->height : 0) -
					(right->left ? right->left->height : 0);
	
				// Case 1: Perform left rotation
				if(rightBalance >= 0) {
					printf("Case 1!\n");
					rotateLeft(curr, right);
				}

				// Case 3: First perform a right rotation, then left rotation
				else {
					printf("Case 3!\n");
					rotateRight(curr->right, curr->right->left);
					rotateLeft(curr, curr->right);
				}
				
			}
			
			else if(currBalance < -1) {

				Node *left = curr->left;
				int leftBalance = (left->left ? left->left->height : 0) -
					(left->left ? left->left->height : 0);

				// Case 2: Perform left rotation
				if(leftBalance >= 0) {
					printf("Case 2!\n");
					rotateRight(curr, left);
				}

				// Case 4: First perform a left rotation, then right rotation
				else {
					printf("Case 4!\n");
					rotateLeft(curr->left, curr->left->right);
					rotateRight(curr, curr->left);
				}
			}

			// Update the parents height after all the rotations
			if(curr->parent) 
				curr->parent->height = max(curr->parent->left ? curr->parent->left->height : 0, 
					curr->parent->right ? curr->parent->right->height : 0) + 1;

			// Retrace the parent next
			curr = curr->parent;
		}
	}

 	/* ****************************************************************************************** */
  void insert (const X& x) { 
		printf("\n%s: %s\n", __FUNCTION__, toStr(x).c_str());
		if(root == NULL) root = new Node(x, NULL);
		else {
			Node* newNode = insert(x,root);
			retrace(newNode);
		}
	}

 	/* ****************************************************************************************** */
	Node* insert (const X& x, Node* curr) {

		// Check for the left insertion
		Node* newNode;
		if(comp(x, curr->value)) {
			if(curr->left != NULL) newNode = insert(x, curr->left);
			else {
				curr->left = new Node(x, curr);
				newNode = curr->left;
			}
		}
		
		// Check for the right insertion
		else if(comp(curr->value, x)) {
			if(curr->right != NULL) newNode = insert(x, curr->right);
			else {
				curr->right = new Node(x, curr);
				newNode = curr->right;
			}
		}

		else return curr;

		curr->height = max(curr->left ? curr->left->height : 0, 
			curr->right ? curr->right->height : 0) + 1;
		return newNode;
	} 
};

/* ******************************************************************************************** */
bool compare (double x, double y) { return x < y; }
string print (double x) { char buf[256]; sprintf(buf, "%.2f", x); return buf; }

/* ******************************************************************************************** */
int main () {

  AVL <double> avl (compare, print);
	avl.insert(50);
	avl.draw();
	avl.insert(30);
	avl.draw();
	avl.insert(10);
	avl.draw();
	avl.insert(20);
	avl.draw();
	avl.insert(40);
	avl.draw();
	while(true) {
		double x;
		printf("Insert value: ");
		scanf("%lf", &x);
		printf("Inserting: %lf\n", x);
		avl.insert(x);
		avl.draw();
	}
	avl.traversal();

}
