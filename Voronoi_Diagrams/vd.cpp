/** 
 * @file vd.cpp
 * @author Can Erdogan
 * @date 2015-08-19
 * @brief Implementation of the "Fortune's Algorithm" (not the brute force Delaunay method) to 
 * create Voronoi Diagrams. O(n log(n)), instead of O(n2).
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
#include <Eigen/Dense>
#include "../AVL_Trees/avl.h"

using namespace std;
using namespace Eigen;

vector <Vector2d> data;

/* ******************************************************************************************** */
struct HalfEdge {
	Vector2d p0, p1;
	int cell_idx;
	HalfEdge* twin, *next, *prev;
};

vector <vector <HalfEdge> > cells;
vector <vector <HalfEdge> > vertexEdges;		///< List of edges incident to a vertex

/* ******************************************************************************************** */
/// Event data structures. Differentiate between Site and Circles.
struct Event { Vector2d point; };
struct SiteEvent : Event { };
struct CircleEvent : Event { vector <int> points; };

struct EventComparison {
  bool operator() (const Event* lhs, const Event* rhs) const {
    return (lhs->point(1)<rhs->point(1));
  }
};

priority_queue <Event*, vector <Event*>, EventComparison> eventQueue;

/* ******************************************************************************************** */
/// AVL tree to keep track of break points between beach line arcs and the site points (sorted
/// by x locations)

struct TreeNode { virtual double value () const = 0; };
struct BreakNode {
	Vector2d p0, p1;	
	HalfEdge* edge;
	inline double value () const { return 0.5 * (p0(0) + p1(0)); }
};
struct SiteNode {
	Vector2d point;			
	CircleEvent* circleEvent;
	inline double value () const { return point(0); }
};

/* ----------------------------------------------------------------------------------- */
bool compare (const TreeNode& x, const TreeNode& y) { return x.value() < y.value(); }

/* ----------------------------------------------------------------------------------- */
string print (const TreeNode& x) { 
	const BreakNode* b = dynamic_cast <const BreakNode*> (&x);
	char buf [256];
	if(b == NULL) {
		const SiteNode* s = dynamic_cast <const SiteNode*> (&x);
		sprintf(buf, "sn: (%lf, %lf)", s->point(0), s->point(1));
	}
	else 
		sprintf(buf, "sn: {(%lf, %lf), (%lf, %lf)}", b->p0(0), b->p0(1), b->p1(0), b->p1(1));
	return string(buf);
}

AVL <TreeNode> avl (compare, print);

/* ******************************************************************************************** */
void readData () {

	// Read the data
	ifstream infile("data.txt");
	double a, b;
	while (infile >> a >> b) 
		data.push_back(Eigen::Vector2d(a,b));
	infile.close();

	// Initialize the event queue
	for(int i = 0; i < data.size(); i++) {
		Event* e = new SiteEvent;
		e->point = data[i];
		eventQueue.push(e);
	}
}

/* ******************************************************************************************** */
void vd () {

	// Process the site and circle events 
	
}

/* ******************************************************************************************** */
int main () {

	readData();
	vd();
}
