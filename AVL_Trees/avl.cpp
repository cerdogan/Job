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
#include "avl.h"

using namespace std;

/* ******************************************************************************************** */
bool compare ( double x,  double y) { return x < y; }
string print ( double x) { char buf[256]; sprintf(buf, "%.2f", x); return buf; }

/* ******************************************************************************************** */
int main () {

  AVL <double> avl (compare, print);
	avl.insert(50);
	avl.insert(30);
	avl.insert(10);
	avl.insert(20);
	avl.insert(40);
	avl.draw();
	while(true) {
		double x;
		char c;
		printf("New operation: \n");
		cin >> c;
		cin >> x;
		if(c == 'i') {
			printf("Inserting: %lf\n", x);
			avl.insert(x);
		}
		else if (c == 'r') {
			printf("Removing: %lf\n", x);
			avl.remove(x);
		}
		avl.draw();
	}
	avl.traversal();

}
