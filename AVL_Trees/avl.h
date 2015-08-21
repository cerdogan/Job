/**
 * @file avl.h
 * @date 2015-08-07
 * @author Can Erdogan
 * @brief Implementation of the AVL trees.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

/* ******************************************************************************************** */
template <class X>
struct AVL {

	bool dbg;
  bool (*comp) (X,X);					///< Comparison function for the tree
  std::string (*toStr) (X);				///< Print function for the tree values
  AVL (bool (*comp_) (X , X), std::string (*print_) (X)) {	///< Constructor
    comp = comp_;
    toStr = print_;
		root = NULL;
		dbg = 0;
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

 	/* ****************************************************************************************** */
	Node* search (const X& x) { 
		if(root == NULL) return NULL;
		return search(x, root); 
	}

 	/* ****************************************************************************************** */
  Node* search (const X& x, Node* curr) {
		if(curr->left != NULL && comp(x, curr->value)) return search(x, curr->left); 
		else if(curr->right != NULL && comp(curr->value, x)) return search(x, curr->right); 
		else if(!comp(curr->value, x) && !comp(x, curr->value)) return curr;
		else return NULL;
	}

 	/* ****************************************************************************************** */
	std::pair<bool,Node*> search_candidateLoc (const X& x) { 
		if(root == NULL) return std::make_pair(false, (Node*)NULL);
		return search_candidateLoc(x, root); 
	}

 	/* ****************************************************************************************** */
	/// Returns the would-be parent node if the given data is added. Returns true if the exact
	/// data is found.
  std::pair<bool,Node*> search_candidateLoc (const X& x, Node* curr) {
		if(curr->left != NULL && comp(x, curr->value)) return search_candidateLoc(x, curr->left); 
		else if(curr->right != NULL && comp(curr->value, x)) return search_candidateLoc(x, curr->right); 
		else if(!comp(curr->value, x) && !comp(x, curr->value)) return std::make_pair(true,curr);
		else return std::make_pair(false,curr);
	}

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
		std::string currS = toStr(curr->value);
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
	void traversal_leaves (std::vector <std::pair<int,Node*> >& leaves) { 
		traversal_leaves(root, leaves); 
	}
	void traversal_leaves (Node* curr, std::vector <std::pair<int,Node*> >& leaves) {
		if(curr->left != NULL) traversal_leaves(curr->left, leaves);
		if(curr->left == NULL && curr->right == NULL)  leaves.push_back(std::make_pair(2,curr));
		else if(curr->left == NULL && curr->right != NULL)  leaves.push_back(std::make_pair(0,curr));
		else if(curr->left != NULL && curr->right == NULL)  leaves.push_back(std::make_pair(1,curr));
		if(curr->right != NULL) traversal_leaves(curr->right, leaves);
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

		if(dbg) 
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

		if(dbg) {
			printf("%s: %s, %s\n", __FUNCTION__, toStr(curr->value).c_str(), toStr(left->value).c_str());
			printf("\tcurr height0: %d, left height0: %d\n", curr->height, left->height);
		}

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
		if(dbg) printf("\tcurr height: %d, left height: %d\n", curr->height, left->height);
	} 
	
 	/* ****************************************************************************************** */
	/// Balances the tree with rotations
	void retrace (Node* curr) {

		return;
		if(dbg) printf("Retrace for '%s'\n", toStr(curr->value).c_str());

		int counter = 0;
		while(curr != NULL) {
			if(counter++ > 10) break;

			// Update the height
			curr->height = max(curr->left ? curr->left->height : 0, 
				curr->right ? curr->right->height : 0) + 1;

			// Look at the left rotations (current balance is +2)
			int currBalance = (curr->right ? curr->right->height : 0) -
				(curr->left ? curr->left->height : 0);
			if(dbg) 
				printf("curr: '%s', balance: %d, height: %d\n", toStr(curr->value).c_str(), currBalance, 
				curr->height);

			if(currBalance > 1) {
	
				// Find out the children's balances
				Node* right = curr->right;
				int rightBalance = (right->right ? right->right->height : 0) -
					(right->left ? right->left->height : 0);
	
				// Case 1: Perform left rotation
				if(rightBalance >= 0) {
					if(dbg) printf("Case 1!\n");
					rotateLeft(curr, right);
				}

				// Case 3: First perform a right rotation, then left rotation
				else {
					if(dbg) printf("Case 3!\n");
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
					if(dbg) printf("Case 2!\n");
					rotateRight(curr, left);
				}

				// Case 4: First perform a left rotation, then right rotation
				else {
					if(dbg) printf("Case 4!\n");
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
		if(dbg) printf("\n%s: %s\n", __FUNCTION__, toStr(x).c_str());
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

 	/* ****************************************************************************************** */
  void remove (const X& x) { 
		if(root == NULL) return;
		else {
			Node* temp = remove(x,root);
			if(temp != NULL) retrace(temp);
		}
	}

 	/* ****************************************************************************************** */
	void connect (Node* child, Node* parent, bool left) {
		if(child != NULL) child->parent = parent;
		if(parent != NULL) {
			if(left) parent->left = child;
			else parent->right = child;
		}
	}

 	/* ****************************************************************************************** */
	Node* remove (const X& x, Node* curr) {

		if(dbg) 
			printf("\n%s: %s, node: %s\n", __FUNCTION__, toStr(x).c_str(), toStr(curr->value).c_str());

		// Check the left side 
		if(curr->left != NULL && comp(x, curr->value)) return remove(x, curr->left); 

		// Check the right side 
		else if(curr->right != NULL && comp(curr->value, x)) return remove(x, curr->right); 

		// Check if this node should be removed
		if(!comp(curr->value, x) && !comp(x, curr->value)) {

			// Case 0: No children
			if(curr->left == NULL && curr->right == NULL) {
				if(dbg) printf("Case 0\n");
				if(curr->parent) {
					if(curr->parent->left == curr) curr->parent->left = NULL;
					else curr->parent->right = NULL;
					return curr->parent;
				}
				else {
					root = NULL;
					return NULL;
				}
			}

			// Case 1a: Only left child
			else if(curr->left != NULL && curr->right == NULL) {
				if(dbg) printf("Case 1a\n");
				if(curr->parent) {
					connect(curr->left, curr->parent, (curr->parent->left == curr));
					return curr->parent;
				}
				else {
					root = curr->left;
					curr->left->parent = NULL;
					return NULL;
				}
			}

			// Case 1b: Only right child
			else if(curr->left == NULL && curr->right != NULL) {
				if(dbg) printf("Case 1b\n");
				if(curr->parent) {
					connect(curr->right, curr->parent, (curr->parent->left == curr));
					return curr->parent;
				}
				else {
					root = curr->right;
					curr->right->parent = NULL;
					return NULL;
				}
				delete curr;
			}

			// Case 2: two children
			else {
				if(dbg) printf("Case 2\n");

				// Find the successor node (left most node of the right subtree)
				Node* succ = curr->right;
				while(true) {
					if(succ->left) succ = succ->left;
					else break;
				}
				if(dbg) printf("\tsucc: %s\n", toStr(succ->value).c_str());

				// Set the successor at the current location
				if(curr->parent) {
					if(succ->parent) {
						if(succ->parent->left == succ) succ->parent->left = NULL;
						else succ->parent->right = NULL;
					}
					connect(succ, curr->parent, (curr->parent->left == curr));
					connect(curr->left, succ, true);
					if(succ != curr->right) connect(curr->right, succ, false);
					return succ;
				}
				else {
					root = succ;
					connect(curr->left, root, true);
					if(succ != curr->right) connect(curr->right, succ, false);
					if(succ->parent) {
						if(succ->parent->left == succ) succ->parent->left = NULL;
						else succ->parent->right = NULL;
					}
					succ->parent = NULL;
					return NULL;
				}

			}
		}

		return NULL;
	}

};
