/** 
 * @file em.cpp
 * @author Can Erdogan
 * @date 2015-08-10
 * @brief Implementation of expectation-maximization to estimate gaussian mixtures for 2D data.
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

struct Gaussian {
	Vector2d mean;
	Matrix2d cov;
	double mixCoeff; 
	double prob (const Vector2d& p);
};

Gaussian Gs [3];
Gaussian bestGs [3];
vector <Vector2d> data;
vector <Vector3d> weights;		/// Organized as (Ga_0, Gb_0, Gc_0), (Ga_1, Gb_1, Gc_1) ...

/* ******************************************************************************************** */
void readData () {
	ifstream infile("data2.txt");
	double a, b;
	while (infile >> a >> b) 
		data.push_back(Eigen::Vector2d(a,b));
	infile.close();
}

/* ******************************************************************************************** */
/// Returns the Gaussian probability
double Gaussian::prob (const Vector2d& p) {
	static bool const dbg = 0;
	if(dbg) printf("\n%s ---------------------\n", __FUNCTION__);
	double norm_ = 1.0 / 2*M_PI*sqrt(cov.determinant());
	if(dbg) cout << "cov: " << cov << endl;
	if(dbg) cout << "cov inv: " << cov.inverse() << endl;
	if(dbg) cout << "p: " << p.transpose() << ", mean: " << mean.transpose() << endl;
	VectorXd val = (p - mean).transpose() * cov.inverse() * (p - mean);
	if(dbg) printf("exp^(): %lf\n", val(0));
	double exp_ = exp(-0.5 * val(0));
	if(dbg) printf("%lf, %lf\n", norm_, exp_);
	double p_ = norm_ * exp_;
	if(p_ != p_) return 0.0;
	return p_;
}

/* ******************************************************************************************** */
/// Set an assignment score for each data point 
void expectation () {

	static bool const dbg = 0;
	if(dbg) printf("\n%s ---------------------\n", __FUNCTION__);

	for(int i = 0; i < data.size(); i++) {

		// Compute the probabilities for each Gaussian
		Vector2d& p = data[i];
		double probs [3];
		double sum = 0.0;
		for(int j = 0; j < 3; j++) {
			probs[j] = Gs[j].mixCoeff * Gs[j].prob(p);
			if(dbg) printf("probs[%d]: %lf\n", j, probs[j]);
			sum += probs[j];
		}

		if(dbg) cout << "sum: " << sum << endl;
		// Compute the new weights for each gaussian
		for(int j = 0; j < 3; j++) weights[i](j) = probs[j] / sum;
	}
}

/* ******************************************************************************************** */
/// Maximize the "fit score" by changing the means and the covariances of the Gaussians
void maximization () {

	static bool const dbg = 0;
	if(dbg) printf("\n%s ---------------------\n", __FUNCTION__);

	// For each Gaussian...
	for(int j = 0; j < 3; j++) {

		// Compute the sum of the weights and the weighted mean
		double sum = 0.0; 
		Vector2d weightedMean (0, 0);
		for(int i = 0; i < data.size(); i++) {
			sum += weights[i](j);
			weightedMean += weights[i](j) * data[i];
			if(dbg) printf("\t%lf\n", weights[i](j));
		}
		if(dbg) printf("sum: %lf, wM: (%lf, %lf)\n", sum, weightedMean(0), weightedMean(1));

		// Set the new mean
		Gs[j].mean = weightedMean / sum;
		if(dbg) cout << "mean: " << Gs[j].mean.transpose() << endl;

		// Compute the new covariance based on the new mean
		Matrix2d newCov = Matrix2d::Zero();
		for(int i = 0; i < data.size(); i++) 
			newCov += (weights[i](j) * (data[i] - Gs[j].mean) * (data[i] - Gs[j].mean).transpose());
		if(dbg) cout << "newCov: " << newCov << endl;
		Gs[j].cov = newCov / sum;

		// Set the mixing coefficient
		Gs[j].mixCoeff = sum / data.size();
	}
	
	
}

/* ******************************************************************************************** */
/// Evaluate the likelihood of the data generated from the modeled mixture of Gaussians
double modelLikelihood () {
	double logProb = 0.0;
	//printf("weights:{\n");
	for(int i = 0; i < data.size(); i++) {
		double prob = 0.0;
		for(int j = 0; j < 3; j++) 
			prob += Gs[j].mixCoeff * Gs[j].prob(data[i]);
		logProb += log(prob);
//		printf("%lf, %lf, %lf\n", weights[i](0), weights[i](1), weights[i](2));
	}
//	printf("}\n\n");
	return logProb;
}
 
/* ******************************************************************************************** */
/// The algorithm...
double em () {
	double lastLike = 0.0;
	for(int i = 0; i < 20; i++) {
		expectation();
		maximization();
		double like = modelLikelihood();
		if(like != like) return 100;
		if(fabs(like - lastLike) < 1e-4) return like;
		lastLike = like;
		// getchar();
	}
}

/* ******************************************************************************************** */
/// Initializes 3 gaussians with random means and covariances
void init () {

	// Gaussians...
	for(int i = 0; i < 3; i++) {
		Gs[i] = Gaussian();
		Gs[i].mean = 10 * Vector2d(((double) rand()) / RAND_MAX, ((double) rand()) / RAND_MAX);
		Gs[i].cov = 10 * Matrix2d::Identity();
		Gs[i].mixCoeff = 1.0 / 3;
	}

	// Gs[0].mean = Vector2d(2,2);
	// Gs[1].mean = Vector2d(5,8);
	// Gs[2].mean = Vector2d(8,5);

	// Weights..
	weights = vector <Vector3d> (data.size(), Vector3d(1.0/3,1.0/3,1.0/3));
//	for(int i = 0; i < data.size(); i++) 
//		weights.push_back(Vector3d(1.0/3,1.0/3,1.0/3));
}

/* ******************************************************************************************** */
int main () {
	srand(time(NULL));
	readData();
	double minLike = 100.0;
	for(int i = 0; i < 1000; i++) {
		init();
		double val = em();
		// printf("min: %lf\n", val);
		if(val < minLike) {
			minLike = val;
			for(int i = 0; i < 3; i++) bestGs[i] = Gs[i];
		}
	}
	for(int i = 0; i < 3; i++) {
		cout << bestGs[i].mean.transpose() << endl;
		cout << bestGs[i].cov << endl;
	}
	
}
/* ******************************************************************************************** */
