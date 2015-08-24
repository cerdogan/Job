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
void getchar2() { // getchar(); 
}

/* ******************************************************************************************** */
struct HalfEdge {
	Vector2d p0, p1;
	int cell_idx;
	HalfEdge* twin, *next, *prev;
	HalfEdge () : twin(NULL), next(NULL), prev(NULL), cell_idx(-1) {}
};

map <int, vector <HalfEdge*> > cells;
vector <vector <HalfEdge> > vertexEdges;		///< List of edges incident to a vertex

/* ******************************************************************************************** */
/// Event data structures. Differentiate between Site and Circles.
struct Event { int pi; Vector2d point; virtual ~Event() {} };		
struct SiteEvent : Event { };		// point is used for the site
struct CircleEvent : Event { 		// point is used for the bottom of the circle
	Vector2d center; 
	Vector3i points; 
	double radius;
	bool falseAlarm, falseAlarmCircle;
	CircleEvent () : falseAlarm(false), falseAlarmCircle(false) {}
};	

struct EventComparison {
  bool operator() (const Event* lhs, const Event* rhs) const {
    return (lhs->point(1)<rhs->point(1));
  }
};

priority_queue <Event*, vector <Event*>, EventComparison> eventQueue;

/* ******************************************************************************************** */
struct Vector2dComp{
    bool operator()(const pair<CircleEvent*,Vector2d>& lhs_, const pair<CircleEvent*,Vector2d>&rhs_){
			static const bool dbg = 0;
			Vector2d lhs = lhs_.second, rhs = rhs_.second;
			if(dbg) cout << "inputs: " << lhs.transpose() << ", " << rhs.transpose() << endl;
			if(rhs(0) - lhs(0) > 1e-5) {
				if(dbg) printf("Returning true: 0 index is smaller\n");
				return true;
			}
			else if((fabs(lhs(0) - rhs(0)) < 1e-5) && ((rhs(1) - lhs(1)) > 1e-5)) {
				if(dbg) printf("Returning true: 0 is same, 1 index is smaller\n");
				return true;
			}
			if(dbg) printf("Returning false\n");
			return false;
    }
};
set <pair<CircleEvent*, Vector2d>, Vector2dComp> allCircles;

/* ******************************************************************************************** */
/// AVL tree to keep track of break points between beach line arcs and the site points (sorted
/// by x locations)

struct TreeNode {

	int p0i, p1i;
	Vector2d p0, p1;	
	HalfEdge* edge1;
	HalfEdge* edge2;
	vector <CircleEvent*> circleEvents;		// potential
	bool dummy;

	TreeNode(const Vector2d& p0_, const Vector2d& p1_, bool dummy_ = false) 
		: p0(p0_), p1(p1_), edge1(NULL), edge2(NULL), dummy(dummy_), p0i(-1), p1i(-1) {}

	TreeNode(int p0i_, int p1i_, bool dummy_ = false) 
		: p0i(p0i_), p1i(p1i_), dummy(dummy_) {
		if(p0i > -1) p0 = data[p0i];
		if(p1i > -1) p1 = data[p1i];

		// Create the half edge records
		if((p0i >= 0) && (p1i >= 0)) {
			edge1 = new HalfEdge();
			edge2 = new HalfEdge();
			value(&(edge1->p0));	
			value(&(edge2->p0));	
			edge1->twin = edge2;
			edge2->twin = edge1;
			edge1->cell_idx = p0i;
			edge2->cell_idx = p1i;
			cells[p0i].push_back(edge1);
			cells[p0i].push_back(edge2);
			cells[p1i].push_back(edge1);
			cells[p1i].push_back(edge2);
		}
	}

	/* ------------------------------------------------------------------------------- */
	inline double value (Vector2d* bp = NULL) const { 		// see breakPoint2.m, computes break point 

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
		if(bp != NULL) *bp = Vector2d(xn,yn);
		return xn;
	}

};

/* ----------------------------------------------------------------------------------- */
bool compare (TreeNode* x, TreeNode* y) { return x->value() < y->value(); }

/* ----------------------------------------------------------------------------------- */
string print (TreeNode* x) { 
	char buf [256];
//	sprintf(buf, "{(%0.2lf, %0.2lf), (%0.2lf, %0.2lf)} {%0.2lf}", x->p0(0), x->p0(1), x->p1(0), x->p1(1),
//		x->value());
	sprintf(buf, "{%d, %d} (#%lu) {%0.9lf}", x->p0i, x->p1i, x->circleEvents.size(), x->value());
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
		e->pi = i;
		eventQueue.push(e);
	}
}

/* ******************************************************************************************** */
Vector2d fitCircle (const Vector2d& A, const Vector2d& B, const Vector2d& C) {

	// Find the perpendicular bisectors
	Vector2d AB = (A + B) / 2.0;
	Vector2d BC = (B + C) / 2.0;
	Vector2d ABdir = (B-A)/(B-A).norm();
	Vector2d ABperp (ABdir(1), -ABdir(0));
	Vector2d AB2 = AB + ABperp * 10;
	Vector2d BCdir = (C-B)/(C-B).norm();
	Vector2d BCperp (BCdir(1), -BCdir(0));
	Vector2d BC2 = BC + BCperp * 10;

	// Find the intersection of the points
	double m1 = (AB2(1) - AB(1)) / (AB2(0) - AB(0));
	double b1 = AB(1) - m1 * AB(0);
	double m2 = (BC2(1) - BC(1)) / (BC2(0) - BC(0));
	double b2 = BC(1) - m2 * BC(0);
	double x = (b2 - b1) / (m1 - m2);
	double y = m1 * x + b1;
	return Vector2d(x,y);
}
 
/* ******************************************************************************************** */
void vd () {

	// Process the site and circle events 
	while(!eventQueue.empty()) {

		// avl.draw();
		getchar2();

		// Check if it is a site event
		Event* event = eventQueue.top();
		eventQueue.pop();
		SiteEvent* siteEvent = dynamic_cast <SiteEvent*> (event);
		if(siteEvent != NULL) {

			printf("\n--- site --------------------------------------------------\n");
			
			// Update the sweep line location
			sweepLine = siteEvent->point(1) - 0.0001;
			printf("sweepLine: %lf\n", sweepLine);
			printf("new site: (%lf, %lf)\n", siteEvent->point(0), siteEvent->point(1));
			//avl.draw();
			getchar2();

			// Locate the existing arc information
			pair <bool, AVL<TreeNode*>::Node*> searchRes = 
				avl.search_candidateLoc(new TreeNode(siteEvent->pi, -1, true));

			// The tree is empty. Temporarily add the site information as a dummy node
			if(searchRes.second == NULL) {
				avl.insert(new TreeNode(siteEvent->pi, -1, true));
				printf("Tree empty!\n");
				continue;
			}

			// The tree still doesn't have a break point, but just a dummy site node information
			TreeNode* parentNode = searchRes.second->value;
			if(parentNode->dummy) {
				avl.remove(parentNode);
				avl.insert(new TreeNode(parentNode->p0i, siteEvent->pi));
				avl.insert(new TreeNode(siteEvent->pi, parentNode->p0i));
				printf("Tree dummy!\n");
				continue;
			}
			
			// Determine the site by comparing it with the found node value
			int prevSiteIdx = 0;
			if(parentNode->value() < siteEvent->point(0)) prevSiteIdx = parentNode->p1i;
			else prevSiteIdx = parentNode->p0i;
			printf("Previous site idx: (%d)\n", prevSiteIdx);
			
			// Create the new break points
			TreeNode* newNode1 = new TreeNode(siteEvent->pi, prevSiteIdx);
			TreeNode* newNode2 = new TreeNode(prevSiteIdx, siteEvent->pi);
 			avl.insert(newNode1);
 			avl.insert(newNode2);

			// Check for "false alarms" for circle events
			set <pair<CircleEvent*, Vector2d>, Vector2dComp>::iterator it =  allCircles.begin();
			printf("# parent circles: %d\n", parentNode->circleEvents.size());
//			for(size_t c_i = 0; c_i < parentNode->circleEvents.size(); c_i++) {
			for(; it != allCircles.end(); it++) {
//				CircleEvent* ce = parentNode->circleEvents[c_i];
				CircleEvent* ce = it->first;
				printf("\tTriplet (%d,%d,%d)\n", ce->points(0), ce->points(1), ce->points(2)); 
				if((ce->center - siteEvent->point).norm() < ce->radius) {
					printf("\tRemoving triplet: (%d,%d,%d)\n", ce->points(0),ce->points(1),ce->points(2));
					ce->falseAlarm = true;
				}
			}
			
			// Get the leaf information to check for circles
			vector <pair<int, AVL<TreeNode*>::Node*> > leafParents;
			avl.traversal_leaves(leafParents);
			printf("Traversal: {");
			vector <pair<int, TreeNode*> > sites;
			for(int i = 0; i < leafParents.size(); i++) {
				TreeNode* node = leafParents[i].second->value;
				int type = leafParents[i].first;
				if(type == 2) {
					printf("(%d,%d), ", node->p0i, node->p1i);
					sites.push_back(make_pair(node->p0i, node));
					sites.push_back(make_pair(node->p1i, node));
				}
				if(type == 0) {
					printf("%d, ", node->p0i);
					sites.push_back(make_pair(node->p0i, node));
				}
				if(type == 1) {
					printf("%d, ", node->p1i);
					sites.push_back(make_pair(node->p1i, node));
				}
			}
			printf("\b\b}\n");

			// Check for circles in triplets
			for(int s_i = 0; s_i < sites.size()-2; s_i++) {

				// Skip newly generated centers
				int i0 = sites[s_i].first, i1 = sites[s_i+1].first, i2 = sites[s_i+2].first;
				if(i0 == i2) continue;

				// If the bottom point of the fit circle can be tangent to the sweep line,
				// add it to the queue
				Vector2d center = fitCircle(data[i0], data[i1], data[i2]);
				double radius = (data[i0]-center).norm();
				double temp_y = center(1) - radius;
				printf("idx: %d, center: (%lf, %lf), temp_y: %lf\n", s_i, center(0), center(1), temp_y);
				printf("radius: %lf, sweepLine: %lf\n", radius, sweepLine);
				if(temp_y < sweepLine) { 

					if (allCircles.find(make_pair((CircleEvent*) NULL, center)) != allCircles.end()) {
						printf("\tTriplet (%d,%d,%d), (%lf, %lf) already exists.\n", i0, i1, i2, center(0), center(1));
						printf("all circles #: %lu\n", allCircles.size());
						continue;
					}

					// Create the circle event
					CircleEvent* ce = new CircleEvent();
					ce->point = Vector2d(center(0), temp_y);
					ce->points = Vector3i(i0,i1,i2);
					ce->center = center;
					ce->radius = radius;
					eventQueue.push(ce);
					allCircles.insert(make_pair(ce, ce->center));
					printf("\tAdding triplet: (%d,%d,%d), (%lf, %lf)\n", i0, i1, i2, center(0), center(1));

					// Register the circle event with the involved arcs
					sites[s_i].second->circleEvents.push_back(ce);
					sites[s_i+1].second->circleEvents.push_back(ce);
					sites[s_i+2].second->circleEvents.push_back(ce);
				}
				else printf("\tCircle already passed, not adding!\n");
			}

		}

		else {

			printf("\n--- circle ------------------------------------------------\n");

			// Update the sweepline
			CircleEvent* ce = dynamic_cast <CircleEvent*> (event);
			printf("circle event: point: (%lf, %lf), center:, (%lf, %lf), points: %d, %d, %d\n", ce->point(0), ce->point(1), ce->center(0), ce->center(1), 
				ce->points(0), ce->points(1), ce->points(2));
			sweepLine = ce->point(1) + 0.00001;
			printf("sweepLine: %lf\n", sweepLine);
			// avl.draw();
			getchar2();

			// Check if false alarm
			if(ce->falseAlarm || ce->falseAlarmCircle) {
				printf("\tFalse alarm!\n");
				continue;
			}

			// Get the arc that is disappearing due to the circle
			pair <bool, AVL<TreeNode*>::Node*> searchRes = 
				avl.search_candidateLoc(new TreeNode(ce->point, Vector2d(), true));
			assert(searchRes.second != NULL && "Could not find the above arc");
			TreeNode* node1 = searchRes.second->value;
			AVL<TreeNode*>::Node* searchNode = searchRes.second;
			printf("node1: (%d,%d)\n", node1->p0i, node1->p1i);

			// Fix node1 if next one is better
			AVL<TreeNode*>::Node* temp = avl.next(searchNode);
			AVL<TreeNode*>::Node* temp2 = avl.prev(searchNode);
			if(temp != NULL) printf("temp: '%s'\n", print(temp->value).c_str());
			if(temp != NULL) printf("temp2: '%s'\n", print(temp2->value).c_str());
			double diff1 = (node1->value() - ce->point(0));
			double diff2 = (temp == NULL) ? 1000.0 : (temp->value->value() - ce->point(0));
			double diff3 = (temp2 == NULL) ? 1000.0 : (temp2->value->value() - ce->point(0));
			printf("\t%lf vs %lf\n", diff1, diff2);
			if(fabs(diff2) < fabs(diff1)) {
				node1 = temp->value;
				searchNode = temp;
				printf("\t\tupdating node1 with temp\n");
			}
			printf("\t%lf vs %lf\n", diff1, diff3);
			if(fabs(diff3) < fabs(diff1)) {
				node1 = temp2->value;
				searchNode = temp2;
				printf("\t\tupdating node1 with temp2\n");
			}
			printf("node1: (%d,%d)\n", node1->p0i, node1->p1i);

			// Skip if the node can't be found
			double diff = (node1->value() - ce->point(0));
			if(fabs(diff) > 0.05) {
				printf("Skipping a circle event (node1) because it is behind the beach line\n");
				continue;
			}

			// Determine the other node
			AVL<TreeNode*>::Node* opt1 = avl.next(searchNode);
			if(opt1 != NULL) printf("opt1: '%s'\n", print(opt1->value).c_str());
			AVL<TreeNode*>::Node* opt2 = avl.prev(searchNode);
			if(opt2 != NULL) printf("opt2: '%s'\n", print(opt2->value).c_str());
			TreeNode* node2;
			if(opt1 == NULL) node2 = opt2->value;
			else if(opt2 == NULL) node2 = opt1->value;
			else {
				double diff1 = (node1->value() - opt1->value->value());	
				double diff2 = (node1->value() - opt2->value->value());	
				printf("diff1: %lf, diff2: %lf\n", diff1, diff2);
				if(fabs(diff1) < fabs(diff2)) node2 = opt1->value;
				else node2 = opt2->value;
			}

			// Skip if the node can't be found
			diff = (node2->value() - ce->point(0));
			if(fabs(diff) > 0.05) {
				printf("Skipping a circle event (node2) because it is behind the beach line\n");
				continue;
			}

			printf("node1: (%d,%d)\n", node1->p0i, node1->p1i);
			printf("node2: (%d,%d)\n", node2->p0i, node2->p1i);

			// Remove any potential circles that were going to use one of the break points for 
			// convergence that just got merged into a voronoi vertex.
			set <pair<CircleEvent*, Vector2d>, Vector2dComp>::iterator it =  allCircles.begin();
			int si0 = ce->points(0), si1 = ce->points(1), si2 = ce->points(2); 
			for(; it != allCircles.end(); it++) {
				CircleEvent* ce = it->first;
				int i0 = ce->points(0), i1 = ce->points(1), i2 = ce->points(2); 
				bool remove = false;
				if((i0 == si0 && i1 == si1) || (i1 == si0 && i2 == si1) || 
					(i0 == si1 && i1 == si0) || (i1 == si1 && i2 == si0)) remove = true; 
				if((i0 == si1 && i1 == si2) || (i1 == si1 && i2 == si2) || 
					(i0 == si2 && i1 == si1) || (i1 == si2 && i2 == si1)) remove = true; 
				
				if(remove) {
					printf("\tRemoving triplet: (%d,%d,%d)\n", i0, i1, i2);
					ce->falseAlarmCircle = true;
				}
			}
	
			// Remove the potential circle events from these nodes
			for(int ce_i = 0; ce_i < node1->circleEvents.size(); ce_i++) {
				CircleEvent* ce = node1->circleEvents[ce_i];
				if(ce->points[0] == node1->p0i && ce->points[1] == node1->p1i)
					ce->falseAlarmCircle = true;
				if(ce->points[1] == node1->p0i && ce->points[2] == node1->p1i)
					ce->falseAlarmCircle = true;
			}
			for(int ce_i = 0; ce_i < node2->circleEvents.size(); ce_i++) {
				CircleEvent* ce = node2->circleEvents[ce_i];
				if(ce->points[0] == node2->p0i && ce->points[1] == node2->p1i)
					ce->falseAlarmCircle = true;
				if(ce->points[1] == node2->p0i && ce->points[2] == node2->p1i)
					ce->falseAlarmCircle = true;
			}

			// Remove the arc from the tree
			printf("Before removes\n");
			getchar2();
			avl.remove(node1);
			// avl.draw();
			printf("Drawn after remove 1\n");
			getchar2();
			avl.remove(node2);
			// avl.draw();
			printf("Drawn after remove 2\n");
			getchar2();

			// Add the new break point 
			TreeNode* newNode;
			if(node1->p0i == node2->p1i)
				newNode = new TreeNode(node2->p0i, node1->p1i);
			else if(node1->p1i == node2->p0i)
				newNode = new TreeNode(node1->p0i, node2->p1i);
			else assert(false && "Unknown new break point creation");
			
 			avl.insert(newNode);
			printf("Inserted new node: '%s'\n", print(newNode).c_str());

			// Set the second points of the completed voronoi edges
			node1->edge1->p1 = ce->center;
			node1->edge2->p0 = ce->center;
			node2->edge1->p1 = ce->center;
			node2->edge2->p0 = ce->center;

			// Find angles around the cell center to place them ccw
			HalfEdge* e1 = node1->edge1, *e2 = node2->edge2;
			int site_idx = (node1->p0i == node2->p1i) ? node1->p0i : node1->p1i;
			Vector2d site = data[site_idx];
			Vector2d v1 = (0.5 * (e1->p0 + e1->p1) - site).normalized();
			Vector2d v2 = (0.5 * (e2->p0 + e2->p1) - site).normalized();
			double angle1 = atan2(v1(1), v1(0)) + (v1(1) < 0 ? 2*M_PI : 0);
			double angle2 = atan2(v2(1), v2(0)) + (v2(1) < 0 ? 2*M_PI : 0);
			if((angle1 < angle2) && fabs(angle1-angle2) > M_PI) angle1 += 2*M_PI;
			else if((angle2 < angle1) && fabs(angle2-angle1) > M_PI) angle2 += 2*M_PI;
			if(angle1 > angle2) {
				e1->prev = e2;
				e2->next = e1;
			}
			else {
				e2->prev = e1;
				e1->next = e2;
			}

			// Get the leaf information to check for circles again
			vector <pair<int, AVL<TreeNode*>::Node*> > leafParents;
			avl.traversal_leaves(leafParents);
			printf("Traversal: {");
			vector <pair<int, TreeNode*> > sites;
			for(int i = 0; i < leafParents.size(); i++) {
				TreeNode* node = leafParents[i].second->value;
				int type = leafParents[i].first;
				if(type == 2) {
					printf("(%d,%d), ", node->p0i, node->p1i);
					sites.push_back(make_pair(node->p0i, node));
					sites.push_back(make_pair(node->p1i, node));
				}
				if(type == 0) {
					printf("%d, ", node->p0i);
					sites.push_back(make_pair(node->p0i, node));
				}
				if(type == 1) {
					printf("%d, ", node->p1i);
					sites.push_back(make_pair(node->p1i, node));
				}
			}
			printf("\b\b}\n");

			// Check for circles in triplets
			for(int s_i = 0; s_i < sites.size()-2; s_i++) {

				// Skip newly generated centers
				int i0 = sites[s_i].first, i1 = sites[s_i+1].first, i2 = sites[s_i+2].first;
				if(i0 == i2) continue;

				// If the bottom point of the fit circle can be tangent to the sweep line,
				// add it to the queue
				Vector2d center = fitCircle(data[i0], data[i1], data[i2]);
				double temp_y = center(1) - (data[i0]-center).norm();
				printf("idx: %d, center: (%lf, %lf), temp_y: %lf\n", s_i, center(0), center(1), temp_y);
				if(temp_y < sweepLine) { 

					// Check if it existed before
					set <pair<CircleEvent*, Vector2d>, Vector2dComp>::iterator it =
						allCircles.find(make_pair((CircleEvent*) NULL, center));
					if(it != allCircles.end() && it->first->falseAlarmCircle){

						printf("\tTurning on an old false alarm for triplet: %d, %d, %d.\n", it->first->points(0), it->first->points(1), it->first->points(2));
						it->first->falseAlarmCircle = false;
						getchar2();
						continue;
					}
					else if(it == allCircles.end()) {

						// Create the circle event
						CircleEvent* ce = new CircleEvent();
						ce->point = Vector2d(center(0), temp_y);
						ce->points = Vector3i(i0,i1,i2);
						ce->center = center;
						eventQueue.push(ce);
						allCircles.insert(make_pair(ce, ce->center));
						printf("\tAdding triplet: (%d, %d, %d)\n", i0, i1, i2);

						// Register the circle event with the involved arcs
						sites[s_i].second->circleEvents.push_back(ce);
						sites[s_i+1].second->circleEvents.push_back(ce);
						sites[s_i+2].second->circleEvents.push_back(ce);
					}
				}

			}

			getchar2();
		}

		
	}
	
}

/* ******************************************************************************************** */
void termination () {

	
	sweepLine -= 3.0;
	vector <AVL<TreeNode*>::Node*> nodes;
	avl.traversal(nodes);
	for(int i = 0; i < nodes.size(); i++) {
		printf("awef\n");
			TreeNode* node = nodes[i]->value;
			Vector2d temp;
			node->value(&temp);
			cout << "\t" << temp.transpose() << endl;
			node->edge1->p1 = temp;
			node->edge2->p0 = temp;
	}
}

/* ******************************************************************************************** */
int main () {

	readData();
	vd();
	//avl.draw();
	termination();
	
	FILE* file = fopen("edges", "w");
	map <int, vector <HalfEdge*> >::iterator it = cells.begin();
	int cell_c = 0;
	for(; it != cells.end(); it++, cell_c++) {
		// printf("%d: %d\n", it->first, it->second.size());
		for(int i = 0; i < it->second.size(); i++) {
			Vector2d p0 = it->second[i]->p0;
			Vector2d p1 = it->second[i]->p1;
			if(p1(1) < p0(1)) {
				Vector2d temp = p0;
				p0 = p1;
				p1 = temp;
			}
			if(p1.norm() > 1e-5 && p0.norm() > 1e-5)
				fprintf(file, "%lf %lf %lf %lf %d\n", p0(0), p0(1), p1(0), p1(1), cell_c);
		}
	}
	fclose(file);
}
