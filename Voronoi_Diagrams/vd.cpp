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

	Vector2d p0, p1;	
	HalfEdge* edge;
	bool dummy;

	TreeNode(const Vector2d& p0_, const Vector2d& p1_, bool dummy_ = false) 
		: p0(p0_), p1(p1_), edge(NULL), dummy(dummy_) {}

	/* ------------------------------------------------------------------------------- */
	inline double value () const { 		// see breakPoint2.m, computes break point 

		if(dummy) return p0(0);

    double x1 = p0(0), y1 = p0(1), x2 = p1(0), y2 = p1(1);
    double m2 = (y2 - y1) / (x2 - x1), m = (-1.0 / m2);
    double d = (p0-p1).norm() / 2.0;
    double k = (y2 + y1) / 2.0 - sweepLine;
    
    double a = 1 / (m * m);
    double b = - 2 * (1 + 1 / (m * m)) * k;
    double c = k * k * ( 1 + 1 / (m * m) ) + d * d;
		//printf("\t(a,b,c): (%lf,%lf,%lf)\n", a, b, c);
		//printf("\tsqrt of %lf\n", (b * b - 4 * a * c));
		double sq = b * b - 4 * a * c;
		if(fabs(sq) < 1e-5) sq = 0.0;
    double delta;
		// if(y1 > y2) delta = (-b - sqrt(sq)) / (2 * a);
		if(x1 < x2) delta = (-b - sqrt(sq)) / (2 * a);
		else delta = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
		//printf("\tdelta: %lf\n", delta);
    // assert(delta > 0);
    
    double yn = sweepLine + delta;
    double xn = ((x1 + x2) / 2) - 1 / m * (k - delta);
		return xn;
	}

};

/* ----------------------------------------------------------------------------------- */
bool compare (TreeNode* x, TreeNode* y) { return x->value() < y->value(); }

/* ----------------------------------------------------------------------------------- */
string print (TreeNode* x) { 
	char buf [256];
	sprintf(buf, "{(%0.2lf, %0.2lf), (%0.2lf, %0.2lf)} {%0.2lf}", x->p0(0), x->p0(1), x->p1(0), x->p1(1),
		x->value());
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

			avl.draw();
			getchar();
			printf("-----------------------------------------------------------\n");
			
			// Update the sweep line location
			sweepLine = siteEvent->point(1) - 0.01;
			printf("loc: (%lf, %lf)\n", siteEvent->point(0), siteEvent->point(1));

			// Locate the existing arc information
			pair <bool, AVL<TreeNode*>::Node*> searchRes = 
				avl.search_candidateLoc(new TreeNode(siteEvent->point, Vector2d(), true));

			// The tree is empty. Temporarily add the site information as a dummy node
			if(searchRes.second == NULL) {
				avl.insert(new TreeNode(siteEvent->point, Vector2d(), true));
				printf("Tree empty!\n");
				continue;
			}

			// The tree still doesn't have a break point, but just a dummy site node information
			TreeNode* parentNode = searchRes.second->value;
			if(parentNode->dummy) {
				avl.remove(parentNode);
				avl.insert(new TreeNode(parentNode->p0, siteEvent->point));
				avl.insert(new TreeNode(siteEvent->point, parentNode->p0));
				printf("Tree dummy!\n");
				continue;
			}
			
			// Determine the site by comparing it with the found node value
			Vector2d prevSiteLoc;
			if(parentNode->value() < siteEvent->point(0)) prevSiteLoc = parentNode->p1;
			else prevSiteLoc = parentNode->p0;
			printf("Previous site loc: (%lf, %lf)\n", prevSiteLoc(0), prevSiteLoc(1)); 
			
			// Create the new break points
 			avl.insert(new TreeNode(siteEvent->point, prevSiteLoc));
 			avl.insert(new TreeNode(prevSiteLoc, siteEvent->point));

			// Check for potential circle events
			vector <pair<int, AVL<TreeNode*>::Node*> > leafParents;
			avl.traversal_leaves(leafParents);
			printf("Traversal: {");
			for(int i = 0; i < leafParents.size(); i++) {
				TreeNode* node = leafParents[i].second->value;
				int type = leafParents[i].first;
				if(type == 2) printf("(%lf, %lf), (%lf,%lf), ", node->p1(0), node->p1(1), node->p0(0), node->p0(1));
				if(type == 0) printf("(%lf,%lf), ", node->p1(0), node->p1(1));
				if(type == 1) printf("(%lf,%lf), ", node->p0(0), node->p0(1));
			}
			printf("\b\b}\n");

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
	avl.draw();
}
