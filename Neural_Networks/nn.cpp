/** 
 * @file nn.cpp
 * @author Can Erdogan
 * @date 2015-08-10
 * @brief Implementation of the backpropagation algorithm for a neural network with 4-input,
 * 7-hidden and 3-output layers and designed for the Iris flower set. Example taken from:
 * visualstudiomagazine.com/articles/2013/09/01/neural-network-training-using-back-propagation.aspx
 * For input-to-hidden:  f(z) = tanh(z) = (e^z - e^(-z)) / (e^z + e^(-z)). 
 * For hidden-to-output: f(zj) = softmax(z) = e^zj / sum_i e^zi.
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

using namespace std;

vector <pair<Eigen::Vector4d,double> > data;

struct Node {
	vector <double> weights;		// Wj_i -> weights of incoming (!) edges
};
vector <Node> hiddenLayer, outputLayer;

/* ******************************************************************************************** */
void readData () {
	ifstream infile("data.txt");
	double a, b, c, d, e;
	while (infile >> a >> b >> c >> d >> e) 
		data.push_back(make_pair(Eigen::Vector4d(a,b,c,d), e));
	infile.close();
}

/* ******************************************************************************************** */
void setup () {
	// Generate the network: hidden layer
	hiddenLayer = vector <Node> (7, Node());
	for(int i = 0; i < hiddenLayer.size(); i++) 
		hiddenLayer[i].weights = vector <double> (4, 0.5);

	// Generate the network: output layer
	outputLayer = vector <Node> (3, Node());
	for(int i = 0; i < outputLayer.size(); i++) 
		outputLayer[i].weights = vector <double> (7, 0.5);
}

/* ******************************************************************************************** */
Vector3d forward (const Vector4d& in) {
	
	// Propagate across hidden layer
	vector <double> hiddenOuts;
	for(int i = 0; i < hiddenLayer.size(); i++) {
		double in = 0.0;
		for(int j = 0; j < 4; j++) in += hiddenLayer[i].weights[j] * in(j);
		double tanh = (exp(in) - exp(-in)) / (exp(in) + exp(-in));
		hiddenOuts.push_back(tanh);
	}
		
	// Propagate across output layer
	Vector3d out;
	for(int i = 0; i < outputLayer.size(); i++) {
		double in = 0.0, sumE = 0.0;
		for(int j = 0; j < hiddenOuts.size(); j++) {
			in += (outputLayer[i].weights[j] * hiddenOutputs[j]);
		}
		double softmax = 
}

/* ******************************************************************************************** */
int main () {
	readData();
}
/* ******************************************************************************************** */
