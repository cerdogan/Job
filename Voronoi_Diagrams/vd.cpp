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

double sweepLine;

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
struct Event { Vector2d point; virtual ~Event() {} };
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

struct TreeNode { 
	virtual ~TreeNode () {}
	double value () const { return 0.0; } 
};

struct BreakNode : TreeNode {
	Vector2d p0, p1;	
	HalfEdge* edge;

	BreakNode(const Vector2d& p0_, const Vector2d& p1_) : p0(p0_), p1(p1_), edge(NULL) {}

	/* ------------------------------------------------------------------------------- */
	inline double value () const { 		// see breakPoint2.m, computes break point 

    double x1 = p0(1), y1 = p0(2), x2 = p1(1), y2 = p1(2);
    double m2 = (y2 - y1) / (x2 - x1), m = (-1.0 / m2);
    double d = (p0-p1).norm() / 2.0;
    double k = (y2 + y1) / 2.0 - sweepLine;
    
    double a = 1 / (m * m);
    double b = - 2 * (1 + 1 / (m * m)) * k;
    double c = k * k * ( 1 + 1 / (m * m) ) + d * d;
    double delta = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
    assert(delta > 0);
    
    double yn = sweepLine + delta;
    double xn = ((x1 + x2) / 2) - 1 / m * (k - delta);
		return xn;
	}

};

struct SiteNode : TreeNode {
	Vector2d point;			
	CircleEvent* circleEvent;
	SiteNode (const Vector2d& p) : point(p), circleEvent(NULL) {}
	inline double value () const { return point(0); }
};

/// To perform searches. AVL could have been better designed somehow. 
struct DummyNode : TreeNode {
	double val;
	DummyNode (double x) : val(x) {}
	inline double value () const { return val; }
};

/* ----------------------------------------------------------------------------------- */
bool compare (TreeNode* x, TreeNode* y) { return x->value() < y->value(); }

/* ----------------------------------------------------------------------------------- */
string print (TreeNode* x) { 
	const BreakNode* b = dynamic_cast <const BreakNode*> (x);
	char buf [256];
	if(b == NULL) {
		const SiteNode* s = dynamic_cast <const SiteNode*> (x);
		sprintf(buf, "sn: (%lf, %lf)", s->point(0), s->point(1));
	}
	else 
		sprintf(buf, "sn: {(%lf, %lf), (%lf, %lf)}", b->p0(0), b->p0(1), b->p1(0), b->p1(1));
	return string(buf);
}

AVL <TreeNode*> avl (compare, print);

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
	while(!eventQueue.empty()) {

		// Check if it is a site event
		Event* event = eventQueue.top();
		eventQueue.pop();
		SiteEvent* siteEvent = dynamic_cast <SiteEvent*> (event);
		if(siteEvent != NULL) {

			printf("-----------------------------------------------------------\n");
			
			// Update the sweep line location
			sweepLine = siteEvent->point(1);
			printf("loc: (%lf, %lf)\n", siteEvent->point(0), siteEvent->point(1));

			// Locate the existing arc information
			SiteNode* parentSiteNode;
			pair <bool, AVL<TreeNode*>::Node*> searchRes = 
				avl.search_candidateLoc(new DummyNode(siteEvent->point(0)));
			if(searchRes.second == NULL) {
				parentSiteNode = NULL;
				printf("Tree empty!\n");
		
				// Create a SiteNode for this new event 
				avl.insert(new SiteNode(siteEvent->point));
			}
			else {
				parentSiteNode = dynamic_cast <SiteNode*> ((searchRes.second->value));
				assert(parentSiteNode != NULL);
				printf("Parent location: (%lf, %lf)\n", parentSiteNode->point(0), parentSiteNode->point(1));
			}
		}

		else {
			CircleEvent* circleEvent = dynamic_cast <CircleEvent*> (event);
		}

	}
	
}

/* ******************************************************************************************** */
int main () {

	readData();
	vd();
}
