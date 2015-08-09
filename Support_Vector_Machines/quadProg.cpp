/**
 * @file quadProg.cpp
 * @author Can Erdogan
 * @date 2015-08-09
 * @brief The use of quadratic programming to minimize the function 0.5x1^2+x2^2-x1*x2-2*x1-6*x2
 * with constraints (x1 + x2) <= 2, (-x1 + 2x2) <= 2, (2x1 + x2) <= 3, 0 <= x1, 0 <= x2.
 * Below, the arguments G and g0 define the cost function as 0.5 * x' * G * x + g0 * x
 * The inequality constraint is defined as x' * CI + ci0 >= 0
 * The equality constraint is defined as x' * CE + ce0 = 0
 */

#include <QuadProg++.hh>

using namespace QuadProgPP;
using namespace std; 

int main () {

	// Create the minimization function f = 0.5 * xT * H * x + g0T * x
	double G_arr [] = {1.0, -1.0, -1.0, 2.0};
	Matrix <double> G (&(G_arr[0]), 2, 2);
	cout << "G: " << G << endl;

	double g0_arr [] = {-2.0, -6.0};
	Vector <double> g0 (&(g0_arr[0]), 2);
	cout << "g0: " << g0 << endl;

	// Create the inequality constraints
	double CI_arr [] = {-1.0, 1.0, -2.0, 1.0, 0.0, -1.0, -2.0, -1.0, 0.0, 1.0};
	Matrix <double> CI (&(CI_arr[0]), 2, 5);
	cout << "CI: " << CI << endl;

	double ci0_arr [] = {2.0, 2.0, 3.0, 0.0, 0.0};
	Vector <double> ci0 (&(ci0_arr[0]), 5);
	cout << "ci0: " << ci0 << endl;

	// Create the equality constraints (nothing)
	double CE_arr [] = {1.0, 0.0};
	Matrix <double> CE (&(CE_arr[0]), 2, 1);
	double ce0_arr [] = {0.5};
	Vector <double> ce0 (&(ce0_arr[0]), 1);

	// Solve the problem
	Vector <double> x;
	solve_quadprog(G, g0, CE, ce0, CI, ci0, x);
	cout << "x: " << x << endl;
}
