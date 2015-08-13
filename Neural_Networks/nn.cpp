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
using namespace Eigen;

#define pv(x) cout << #x << ": " << (x).transpose() << endl;
#define pc(x) cout << #x << ": " << (x) << endl;
#define sq(x) ((x) * (x))

vector <pair<VectorXd,VectorXd> > trainData;
vector <pair<VectorXd,VectorXd> > testData;

struct Node {
	vector <double> weights;		// Wj_i -> weights of incoming (!) edges
	double bias;
};
vector <Node> hiddenLayer, outputLayer;
double trainingThres;
int trainingIters;

/* ******************************************************************************************** */
void readData () {
	ifstream infile("data.txt");
	double a, b, c, d, e;
	while (infile >> a >> b >> c >> d >> e) {
		Vector3d output (0, 0, 0);
		output((int) e) = 1.0;
		if(((double) rand()) / RAND_MAX < 0.2) 
			testData.push_back(make_pair(Eigen::Vector4d(a,b,c,d), output));
		else
			trainData.push_back(make_pair(Eigen::Vector4d(a,b,c,d), output));
	}
	infile.close();
}

/* ******************************************************************************************** */
void setup () {
	// Generate the network: hidden layer
	hiddenLayer = vector <Node> (7, Node());
	for(int i = 0; i < hiddenLayer.size(); i++) {
		for(int j = 0; j < 4; j++) 
			hiddenLayer[i].weights.push_back(((double) rand()) / RAND_MAX);
		hiddenLayer[i].bias = ((double) rand()) / RAND_MAX;
		
	}

	// Generate the network: output layer
	outputLayer = vector <Node> (3, Node());
	for(int i = 0; i < outputLayer.size(); i++) {
		for(int j = 0; j < 7; j++) 
			outputLayer[i].weights.push_back(((double) rand()) / RAND_MAX);
		outputLayer[i].bias = ((double) rand()) / RAND_MAX;
	}
}

/* ******************************************************************************************** */
void printState () {

	printf("\nvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
	printf("Hidden layer: \n");
	for(int i = 0; i < hiddenLayer.size(); i++) {
		printf("weights: {");
		for(int j = 0; j < 4; j++) 
			printf("%lf, ", hiddenLayer[i].weights[j]);
		printf("\b\b}, bias: %lf\n", hiddenLayer[i].bias);
	}

	printf("\nOutput layer: \n");
	for(int i = 0; i < outputLayer.size(); i++) {
		printf("weights: {");
		for(int j = 0; j < 7; j++) 
			printf("%lf, ", outputLayer[i].weights[j]);
		printf("\b\b}, bias: %lf\n", outputLayer[i].bias);
	}
	printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
}

/* ******************************************************************************************** */
VectorXd ho, outs, ins;
VectorXd forward () {
	
	static bool const dbg = 0;
	if(dbg) printf("\n\n%s -----------------------------------------\n", __FUNCTION__);
	if(dbg) pv(ins);

	// Propagate across hidden layer
	ho = VectorXd (hiddenLayer.size());
	for(int i = 0; i < hiddenLayer.size(); i++) {
		double in = 0.0;
		for(int j = 0; j < ins.rows(); j++) in += hiddenLayer[i].weights[j] * ins(j);
		in += hiddenLayer[i].bias;
		double tanh = (exp(in) - exp(-in)) / (exp(in) + exp(-in));
		ho(i) = tanh;
	}
	
	if(dbg) pv(ho);
		
	// Propagate across output layer (only gather inputs to the activation function)
	VectorXd temps (outputLayer.size());
	for(int i = 0; i < outputLayer.size(); i++) {
		double in = 0.0, sumE = 0.0;
		for(int j = 0; j < ho.rows(); j++) 
			in += (outputLayer[i].weights[j] * ho[j]);
		temps(i) = in + outputLayer[i].bias;
	}

	if(dbg) pv(temps);

	// Compute the softmax (activation function) values
	outs = VectorXd (outputLayer.size());
	double sum = 0.0;
	for(int i = 0; i < outputLayer.size(); i++) {
		outs(i) = exp(temps(i));
		sum += outs(i);
	}
	outs /= sum;
	return outs;
} 
/* ******************************************************************************************** */
/// Delta = error * softmax derivative
void updateWeights (const VectorXd& exp) {
	
	static bool const dbg = 0;
	if(dbg) printf("\n\n%s -----------------------------------------\n", __FUNCTION__);

	// Compute output gradients
	VectorXd oGrads (outputLayer.size()); 
	for(int i = 0; i < outputLayer.size(); i++) {
		double deriv = (1 - outs(i)) * outs(i);
		if(dbg) printf("\terr %d: %lf\n", i, (exp(i) - outs(i)));
		oGrads(i) = deriv * (exp(i) - outs(i));
	}
	if(dbg) pv(oGrads);

	// Compute hidden gradients
	VectorXd hGrads (hiddenLayer.size()); 
	for(int i = 0; i < hiddenLayer.size(); i++) {
		double deriv = (1 - ho(i)) * (1 + ho(i));
		double sum = 0.0;
		for(int j = 0; j < outputLayer.size(); j++) {
			sum += (oGrads(j) * outputLayer[j].weights[i]);
			if(dbg) printf("\t\t%lf * %lf\n", oGrads(j), outputLayer[j].weights[i]);
		}
		if(dbg) printf("\tsum %d: %lf\n", i, sum);
		hGrads(i) = deriv * sum;
	}
	if(dbg) pv(hGrads);
	
	// Update hidden weights
	static const double learnRate = 0.01;
	for(int i = 0; i < ins.rows(); i++) {
		for(int j = 0; j < hiddenLayer.size(); j++) {
			double delta = learnRate * hGrads(j) * ins(i);
			hiddenLayer[j].weights[i] += delta;
		}
	}

	// Update hidden biases
	for(int j = 0; j < hiddenLayer.size(); j++) 
		hiddenLayer[j].bias += learnRate * hGrads(j);

	// Update output weights
	for(int i = 0; i < hiddenLayer.size(); i++) {
		for(int j = 0; j < outputLayer.size(); j++) {
			double delta = learnRate * oGrads(j) * ho(i);
			outputLayer[j].weights[i] += delta;
		}
	}

	// Update output biases
	for(int j = 0; j < outputLayer.size(); j++) 
		outputLayer[j].bias += learnRate * oGrads(j);
}

/* ******************************************************************************************** */
void test () {
	// Compute the mean error
	size_t dataSize = testData.size();
	int correct = 0;
	for(int d = 0; d < dataSize; d++) {

		// Perform forward computation with the net
		pair <VectorXd, VectorXd>& data = testData[d];
		ins = data.first;
		outs = forward();

		// Get the most likely answer
		double maxVal = 0.0;
		int maximizer = 0;
		for(int i = 0; i < outs.rows(); i++) {
			if(outs(i) > maxVal) {
				maxVal = outs(i);
				maximizer = i;
			}
		}

		// Get the most likely expectation (bad formatting problem)
		double maxValExp = 0.0;
		int maximizerExp = 0;
		VectorXd& exp = data.second;
		for(int i = 0; i < exp.rows(); i++) {
			if(exp(i) > maxValExp) {
				maxValExp = exp(i);
				maximizerExp = i;
			}
		}

		if(maximizer == maximizerExp) correct++;
	}
	printf("success rate: %lf (%d/%d)\n", 100.0 * ((double) correct) / dataSize, correct, dataSize);
}

/* ******************************************************************************************** */
void train () {

	for(int i = 0; i < trainingIters; i++) {

		// Set up the random indexing for the training data
		vector <int> indices;
		size_t dataSize = trainData.size();
		for(int d = 0; d < dataSize; d++) indices.push_back(d);
		for(int d = 0; d < dataSize; d++) {
			int r = rand() % indices.size();
			int temp = indices[r];
			indices[r] = indices[d];
			indices[d] = temp;
		}
		
		// Perform back-propagation for each example
		for(int d = 0; d < dataSize; d++) {

			// Perform forward computation with the net
			pair <VectorXd, VectorXd>& data = trainData[indices[d]];
			ins = data.first;
			outs = forward();

			// Update the weights
			updateWeights(data.second);
		}
		
		// printState();

		// Compute the mean error
		double errorSQ = 0.0;
		for(int d = 0; d < dataSize; d++) {

			// Perform forward computation with the net
			pair <VectorXd, VectorXd>& data = trainData[indices[d]];
			ins = data.first;
			outs = forward();

			// Compute the error
			for(int i = 0; i < outs.rows(); i++) errorSQ += sq(outs(i) - data.second(i));
		}
		
		errorSQ /= dataSize;
		// printf("iter %d: error: %lf\n", i, errorSQ);
		if(errorSQ < trainingThres) break;
		// getchar();
	}
}

/* ******************************************************************************************** */
int main (int argc, char* argv[]) {
	assert(argc > 2 && "Need a training threshold (0.01-0.05?) and training #iters: (500 - 15000?)");
	trainingThres = atof(argv[1]);
	trainingIters = atof(argv[2]);
	srand(time(NULL));
	setup();
	// printState();
	//test();
	//return 1;
	readData();
	train();
	test();
}
/* ******************************************************************************************** */
