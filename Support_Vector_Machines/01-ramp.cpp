/**
 * @file 01-ramp.cpp
 * @author Can Erdogan
 * @date Apr 21, 2012; Sep 09, 2013
 * @brief Demonstrates the construction of a recursive ramp structure. See the doc for results.
 */

#include "ramp.h"
#include "basic.h"

#include <gtsam/base/Vector.h>
#include <gtsam/nonlinear/NonlinearFactorGraph.h>
#include <gtsam/nonlinear/LevenbergMarquardtOptimizer.h>
typedef gtsam::NonlinearFactorGraph Graph;  

#define ADD(x) graph->add(x)

using namespace std;
using namespace gtsam;

/* ******************************************************************************************** */
/// Creates a factor graph that represents a set of ramps where the left tip of each ramp either 
/// lies on the middle of the previous ramp or on the given blocks top-right corner. 
/// The variables of the factor graph are the locations of the end-points of each stick. For each 
/// stick, we have a distance factor to keep the end-points at the right distance. Also for each 
/// stick's right end-point, we have a prior factor which indicates the 'y' value should be 0 (it 
/// should be on the ground).  
/// The interesting stuff is on the left end-points (except first). The alignment factor basically 
/// says the left end-point of a 2nd stick should be between the end-points of the first stick.  
/// Lastly, the story is that the last stick should be out of the water, so there is a constraint 
/// trying to pull it to the edge of the water.
Graph* createGraph(Point2 block, float waterLength, std::vector <float> plateLengths, 
		size_t numSticks) {

	// Create the graph and the noise models
	Graph* graph = new Graph;
	SharedNoiseModel model = noiseModel::Diagonal::Sigmas(Vector_(1, 1e-3));
	SharedNoiseModel model2 = noiseModel::Diagonal::Sigmas(Vector_(2, 1e-3, 1e-3));
	assert(numSticks <= plateLengths.size());

	// Keep track of the number of the sticks and the variables to create the keys
	size_t stickID = 1;
	size_t numVariables = 0;
	Key A1(numVariables++), A2(numVariables++);

	// Add the initial factors for the first level
	ADD(PriorFactor < LieVector >(A1,LieVector(Eigen::Vector2d(block.x(), block.y())), model2));
	ADD(Factors::Distance(A1, A2, plateLengths[0], model));
	ADD(Factors::Prior1D(A2, false, 0.0, model));

	// Continue adding levels as necessary
	Key prev1 = A1, prev2 = A2;
	while(stickID != numSticks) {

		Key new1 = Key(numVariables++), new2 = Key(numVariables++);
		ADD(Factors::Alignment(prev1, prev2, new1, model2));
		ADD(Factors::Distance(new1, new2, plateLengths[stickID++], model));
		ADD(Factors::Prior1D(new2, false, 0.0, model));
		ADD(Factors::Ordering(prev1, new1, true, .15, true));
		ADD(Factors::Bounding2D(new1, false, 0.03, true));
		prev1 = new1;
		prev2 = new2;
	}

	// Add the bounding constraint for the water for the last stick
	Key lastKey(numVariables - 1);
	ADD(Factors::Prior1D(lastKey, true, block.x() + waterLength, model));
	return graph;
}

/* ******************************************************************************************** */
/// Creates a random vector within the given limits
static Vector randomVector(const Vector& minLimits, const Vector& maxLimits) {

	// Get the number of dimensions and create the return vector
	size_t numDims = dim(minLimits);
	Vector vector = zero(numDims);

	// Create the random vector
	for(size_t i = 0; i < numDims; i++) {
		double range = maxLimits(i) - minLimits(i);
		vector(i) = (((double) rand()) / RAND_MAX) * range + minLimits(i);
	}
	return vector;
}

/* ******************************************************************************************** */
/// Optimizes for the given graph with random restarts where the initial values are sampled
/// from the given bounding box limit.
static Values solveGraph(Graph& graph, double maxValue, size_t numIterations = 1000) {

	// Try different instantiations
	size_t attempt;
	size_t numKeys = graph.keys().size();
	for(attempt = 0; attempt < numIterations; attempt++) {

		if(attempt % 100 == 0) printf("Attempt %lu...\n", attempt);

		// Set random values for initialization
		Values values;
		Vector minLimits = zero(numKeys * 2), maxLimits = maxValue * ones(numKeys * 2);
		Vector random = randomVector(minLimits, maxLimits);
		for(size_t i = 0; i < numKeys; i++)
			values.insert(i, LieVector(maxValue * Eigen::Vector2d::Random()));
		// cout << "\n\n\n\n\n\n\n" << "random: " << random.transpose() << "\n\n\n\n\n\n\n";

		// Optimize
		LevenbergMarquardtParams params;
		params.absoluteErrorTol = 1e-15;
		params.relativeErrorTol = 1e-15;
		Values result;
		double error;
			LevenbergMarquardtOptimizer optimizer(graph, values, params);
			result = optimizer.optimize();
			error = graph.error(result);
		// Stop if a zero error is attained.
		if(!result.empty() && error < 1e-2) {
			// result.print("\n\n\nRESULT::\n");
			printf("attempt: %lu, error: %lf\n", attempt, error);
			return result;
		}
	}

	return Values();
}

/* ******************************************************************************************** */
void visualize (const Values& values) {

	// Write the file
	FILE* file = fopen(".results", "w");
	char firstLine[512], secondLine[512];
	sprintf(firstLine, "");
	sprintf(secondLine, "");
	for (size_t i = 0; i < values.size(); i++) {
		double x = values.at <LieVector>(i)(0), y = values.at <LieVector>(i)(1);
		if(i % 2 == 0)
			sprintf(firstLine, "%s%.3lf\t%.3lf\t", firstLine, x, y);
		else sprintf(secondLine, "%s%.3lf\t%.3lf\t", secondLine, x, y);
	}
	fprintf(file, "%s\n", firstLine);
	fprintf(file, "%s\n", secondLine);
	fclose(file);

	// Create the gnuplot file
	FILE* plotter = fopen(".plotter", "w");
	fprintf(plotter, "set output 'plates.png'\nset terminal png\nset grid front\nset pointsize 1.5\n"
			"set size ratio -1\nplot \\\n");
	std::string colors[] = { "red", "blue", "green", "cyan", "orange" };
	for (size_t i = 0; i < values.size() / 2; i++) {
		fprintf(plotter,
				"\"< head -1 .results\" u %lu:%lu with points pt 6 lw 2 lc rgb \"%s\" notitle, \\\n",
				2 * i + 1, 2 * i + 2, colors[i].c_str());
		fprintf(plotter, "\".results\" u %lu:%lu with lines lc rgb \"%s\" lw 2 lt 1 title \"line %lu\"",
				2 * i + 1, 2 * i + 2, colors[i].c_str(), i);
		fprintf(plotter, "%s", (i < (values.size() / 2 - 1)) ? ", \\\n" : "\n");
	}
	fprintf(plotter, "\nset term pop\nset output\n");
	fclose(plotter);

	// Save the image file and display it
	int res;
	res = system("gnuplot < .plotter");
	res = system("killall shotwell");
	res = system("shotwell plates.png &");
}

/* ******************************************************************************************** */
/// The main thread
int main () {

	// Prepare the block size, the plate lengths and the water size
	float temp[] = { 1.2192, 1.2192, 1.2192, 1.2192, 1.2192, 1.2192};
	vector <float> plateLengths(temp, temp + sizeof(temp) / sizeof(float));
	Point2 block(0.0, .585);
	float waterLength = 2.5;

	// Create the graph
	Graph* graph = createGraph(block, waterLength, plateLengths, 3);

	// Optimize the result with random initializations
	srand(time(NULL));
	Values result = solveGraph(*graph, block.x() + waterLength);
	result.print("Result: ");

	// Visualize the result if one is achieved
	if(!result.empty()) {
		printf("Found result!\n");
		visualize(result);
	}
}
