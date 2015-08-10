/** 
 * @file svm.cpp
 * @author Can Erdogan
 * @date 2015-08-09
 * @brief Implementation of the linear support vector machines to simple 2D data.
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
#include <QuadProg++.hh>

using namespace std;

vector <pair<Eigen::Vector2d,double> > data;

/* ******************************************************************************************** */
void readData () {
	ifstream infile("data.txt");
	double a, b, c;
	while (infile >> a >> b >> c) 
		data.push_back(make_pair(Eigen::Vector2d(a,b), c));
	infile.close();
	for(int i = 0; i < data.size(); i++)
		cout << data[i].first.transpose() << " " << data[i].second << endl;
}

/* ******************************************************************************************** */
/// Find the Lagrangian coefficients by solving for the quadratic program
void lagrangian () {

	// Create the cost function
	int N = data.size();
	double H_ [N * N], f_ [N];
	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) 
			H_[i * N + j] = data[i].second * data[j].second * data[i].first.dot(data[j].first);
		f_[i] = -1.0;
	}
	QuadProgPP::Matrix <double> H (&(H_[0]), N, N);
	QuadProgPP::Vector <double> f (&(f_[0]), N);
	cout << "H: " << H << endl;
	cout << "f: " << f << endl;

	// Create the inequality constraints (alpha_i >= 0)
	double I_ [N * N], i0_ [N];
	for(int i = 0; i < N; i++) {
		I_[i * (N+1)] = 1.0;
		i0_[i] = 0.0;
	}
	QuadProgPP::Matrix <double> I (&(I_[0]), N, N);
	QuadProgPP::Vector <double> i0 (&(i0_[0]), N);

	// Create the equality constraint ((sum_i alpha_i * y_i) = 0)
	double E_ [N], e0_ [1];
	for(int i = 0; i < N; i++) E_[i] = data[i].second;
	e0_[0] = 0.0;
	QuadProgPP::Matrix <double> E (&(I_[0]), N, 1);
	QuadProgPP::Vector <double> e0 (&(e0_[0]), 1);

	// Solve the problem
	QuadProgPP::Vector <double> x;
	solve_quadprog(H, f, E, e0, I, i0, x);
	cout << "x: " << x << endl;
}

/* ******************************************************************************************** */
int main () {
	readData();
	lagrangian();
}
/* ******************************************************************************************** */
