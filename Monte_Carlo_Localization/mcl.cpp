/**
 * @file mcl.cpp
 * @author Can Erdogan
 * @date 2015-08-04
 * @brief Implementation of Monte-Carlo localization based on the explanation of Dellaert et al.'s
 * ICRA '99 paper.
 */

#include <Eigen/Dense>
#include <assert.h>
#include <iostream>
#include <math.h>
#include <queue>
#include <set>
#include <map>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <random>

#include <GL/glut.h>
#include <GL/gl.h>	
#include <GL/glu.h>	
#include <unistd.h> 

#include <X11/Xlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace Eigen;
using namespace std;

vector <Vector3d> particles;
vector <bool> seen;
int numParticles = 1000;
Vector3d state = Vector3d::Zero();

vector <Vector2d> landmarks;

/* The number of our GLUT window */
int window; 
bool limitMouse = 0;
int mouse_cx, mouse_cy;
int mouse_x, mouse_y;

#define sq(x) ((x) * (x))

/* ********************************************************************************************* */
void InitGL(int Width, int Height) {
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);		// This Will Clear The Background Color To Black
  glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
  glDepthFunc(GL_LESS);				// The Type Of Depth Test To Do
  glEnable(GL_DEPTH_TEST);			// Enables Depth Testing
  glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();				// Reset The Projection Matrix
  gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);	
  glMatrixMode(GL_MODELVIEW);
}

/* ********************************************************************************************* */
double canBeSeen (const Vector3d& state, const Vector2d& landmark) {
	Vector2d dir = (landmark - state.block<2,1>(0,0));
	double dist = dir.norm();
	double angle = acos(dir.normalized().dot(Vector2d(cos(state(2)), sin(state(2)))));
	printf("dist: %lf, angle: %lf\n", dist, angle);
	if((dist < 1.25) && (angle < 0.40)) return true;
	return false;
}


/* ********************************************************************************************* */
/// Returns the likelihood of making a measurement of a landmark at a given state
double measurementLikelihood (const Vector3d& state, const Vector2d& landmark, bool dbg = 0) {

//	bool dbg = 1;

	// Compute the distance from the agent to the landmark
	static const double distMu = 1.2;
	static const double distStdev = 0.25;
	Vector2d dir = landmark - state.block<2,1>(0,0);
	double distErr = dir.norm() - distMu;
	if(dbg) printf("\tdistErr: %lf\n", distErr);

	// Compute the heading from the agent to the landmark
	static const double angleMu = 0.0;
	static const double angleStdev = 0.45;
	double angleErr = acos(dir.normalized().dot(Vector2d(cos(state(2)), sin(state(2)))));
	if(dbg) printf("\tangleErr: %lf\n", angleErr);

	// Compute the probability
	double p = (1.0 / (2 * M_PI * distStdev * angleStdev)) * exp(-0.5 * sq((distErr/distStdev)-(angleErr/angleStdev)));
	if(dbg) printf("\tprob: %lf\n", p);
	return p;
}

/* ********************************************************************************************* */
/// Returns a new state, sampled from the motion model with a given control input
void motionModel (const Vector3d& state, const Vector2d& u, Vector3d& newState, const Vector2d& lastU) {

	// Get the desired angle and the forward velocity
	//cout << "u: " << u.transpose() << endl;
	newState = state;
	static const double K_th = 5, K_x = 0.05;
	double lastNewTh = -atan2(lastU(1), lastU(0));
	double newTh = -atan2(u(1), u(0));
	double diff = (lastNewTh - newTh);
	if(diff > M_PI) newState(2) -= 2*M_PI;
	else if(diff < -M_PI) newState(2) += 2*M_PI;
	double vel_x = K_x * u.norm();
	double vel_th = -K_th * (newState(2) - newTh);
	
	// Add Gaussian noise to the forward velocity
  static default_random_engine generator;
	static normal_distribution <double> forwardDist (0.0, 0.1);
	vel_x += forwardDist(generator);

	// Add Gaussian noise to the forward velocity
	static normal_distribution <double> rotationDist (0.0, 2);
	vel_th += rotationDist(generator);

	// Update the state
	static const double dt = 0.005;
	newState(2) += dt * vel_th;
	newState(0) += cos(newState(2)) * dt * vel_x;
	newState(1) += sin(newState(2)) * dt * vel_x;
	// printf("vel_x: %lf, vel_th: %lf\n", vel_x, vel_th);
}

/* ********************************************************************************************* */
void updateParticles (const Vector2d& u, const Vector2d& lastU) {

	bool dbg = 0;

	// Motion model
	for(int i = 0; i < particles.size(); i++) {
		Vector3d state = particles[i];
		motionModel(state, u, particles[i], lastU);
	}

	// Compute the update phase sampling weights
	vector <double> accuWeights;
	double totalW = 0.0;
	for(int i = 0; i < particles.size(); i++) {
		double w = 1e-4;
		for(int l = 0; l < landmarks.size(); l++) {
			if(!seen[l]) continue;
			w += measurementLikelihood(particles[i], landmarks[l]);
		}
		totalW += w;
		accuWeights.push_back(totalW);
		if(dbg) printf("weight %d: %lf\n", i, totalW);
	}

	// Sample from the weighted set
	vector <Vector3d> newParts;
	for(int i = 0; i < numParticles; i++) {
		double w = totalW * (((double) rand()) / RAND_MAX);
		for(int j = 0; j < numParticles; j++) {
			if(accuWeights[j] > (w-1e-5)) {
				newParts.push_back(particles[j]);
				if(dbg) printf("adding particle: %d, random weight: %lf\n", j, w);
				break;
			}
		}
	}

	// exit(0);
	particles = newParts;
}

/* ********************************************************************************************* */
void DrawGLScene() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
  glLoadIdentity();				// Reset The View

	// Draw the edges of the view screen
  glColor3f(0.0f, 0.0f, 0.0f);		
  glTranslatef(0.0, 0.0,-6.0f);	
	float width = 7.25, height = 4.85;
  glBegin(GL_POLYGON);		
		glVertex3f(width/2.0, height/2.0, 0.0);
		glVertex3f(-width/2.0, height/2.0, 0.0);
		glVertex3f(-width/2.0, -height/2.0, 0.0);
		glVertex3f(width/2.0, -height/2.0, 0.0);
  glEnd();					

	// Draw the landmarks
	static const float off = 0.03;
	glLineWidth(4);
	for(int i = 0; i < landmarks.size(); i++) {
		Vector2d l_ = landmarks[i];
		if(seen[i]) glColor3f(0.0f, 1.0f, 0.0f);		
		else glColor3f(1.0f, 0.0f, 0.0f);		
		glBegin(GL_POLYGON);		
			glVertex3f(l_(0) + off, l_(1) + off, 0.0);
			glVertex3f(l_(0) - off, l_(1) + off, 0.0);
			glVertex3f(l_(0) - off, l_(1) - off, 0.0);
			glVertex3f(l_(0) + off, l_(1) - off, 0.0);
		glEnd();					
	}
	glLineWidth(3);

	// Draw the robot
  glColor3f(0.0f, 0.0f, 0.0f);		
	Vector2f head = Vector2f(state(0), state(1)) + 0.2 * Vector2f(cos(state(2)), sin(state(2)));
	glBegin(GL_LINES);		
		glVertex3f(state(0), state(1), 0.0);
		glVertex3f(head(0), head(1), 0.0);
	glEnd();
  glBegin(GL_POLYGON);			
	float Kc = 0.05;
	for(int i = 0; i < 32; i++) 
		glVertex3f(state(0) + Kc * cos(i * 2 * M_PI / 32), state(1) + Kc * sin(i * 2 * M_PI / 32), 0.0f);	
  glEnd();					
	
	// Draw the particles
  glColor3f(0.0f, 0.0f, 1.0f);		
	for(int i = 0; i < particles.size(); i++) {
		Vector3d state = particles[i];
		Vector2f head = Vector2f(state(0), state(1)) + 0.2 * Vector2f(cos(state(2)), sin(state(2)));
		glBegin(GL_LINES);		
			glVertex3f(state(0), state(1), 0.0);
			glVertex3f(head(0), head(1), 0.0);
		glEnd();
		glBegin(GL_POLYGON);			
		float Kc = 0.05;
		for(int i = 0; i < 32; i++) 
			glVertex3f(state(0) + Kc * cos(i * 2 * M_PI / 32), state(1) + Kc * sin(i * 2 * M_PI / 32), 0.0f);	
		glEnd();					
	}

	// Draw the mouse area
  glColor3f(0.0f, 0.0f, 0.0f);		
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glTranslatef(2.9f,-1.7f,0.0f);	
	glLineWidth(6);
  glBegin(GL_POLYGON);			
	float K = 0.7, K2 = 0.02;
	for(int i = 0; i < 32; i++) 
		glVertex3f(K * cos(i * 2 * M_PI / 32), K * sin(i * 2 * M_PI / 32), 0.0f);		// Top
  glEnd();					
  glBegin(GL_POLYGON);		
	for(int i = 0; i < 32; i++) 
		glVertex3f(K2 * cos(i * 2 * M_PI / 32), K2 * sin(i * 2 * M_PI / 32), 0.0f);		// Top
  glEnd();					
  glBegin(GL_POLYGON);		
		glVertex3f(0.7, 0.7, 0.0);
		glVertex3f(-0.7, 0.7, 0.0);
		glVertex3f(-0.7, -0.7, 0.0);
		glVertex3f(0.7, -0.7, 0.0);
  glEnd();					

	// Limit the mouse location to within the circle
	bool input = false;
	if(limitMouse) {
		
		input = true;
		float draw_x = -width * (mouse_cx - mouse_x) / (800);
		float draw_y = height * (mouse_cy - mouse_y) / (540);
		glBegin(GL_LINES);		
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(draw_x, draw_y, 0.0);
		glEnd();
		float dist = sqrt(sq(mouse_cx - mouse_x) + sq(mouse_cy - mouse_y));
		if(dist > 75.0) {
			Vector2d dir = (Vector2d(mouse_x, mouse_y) - Vector2d(mouse_cx, mouse_cy)).normalized();
			Vector2d new_mouse = Vector2d(mouse_cx, mouse_cy) + dir * 75;
			glutWarpPointer(new_mouse(0), new_mouse(1));
		}
	}
//	else glutWarpPointer(mouse_cx, mouse_cy);

	// Update world 
	if(input) {

		printf("\n\nvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");

		// Update agent motion
		static Vector2d lastU = Vector2d::Zero();
		Vector2d u = (Vector2d(mouse_x, mouse_y) - Vector2d(mouse_cx, mouse_cy));
		Vector3d newState;
		motionModel(state, u, newState, lastU);
		state = newState;
		cout << "state: " << state.transpose() << endl;

		// See if particles are seen by the agent
		for(int i = 0; i < landmarks.size(); i++) {
			seen[i] = canBeSeen(state, landmarks[i]);
		}

		// Update the particles
		updateParticles(u, lastU);

		lastU = u;
	}

  // swap buffers to display, since we're double buffered.
  glutSwapBuffers();
}

/* ********************************************************************************************* */
bool keyInput = 0;
void passive(int x, int y) {
	// printf("mouse: (%d, %d)\n", x, y);
	if(!keyInput) {
		mouse_x = x, mouse_y = y;
	}
}

/* ********************************************************************************************* */
void test () {

	FILE* testFile = fopen("test", "w+");

	printf("Test started.\n");

	// Initialize all the particles at the origin
	vector <Vector3d> particles;
	int numParticles = 50;
	for(int i = 0; i < numParticles; i++) particles.push_back(Vector3d::Zero());
	
	// Simulate forward in time with going right
	for(int time_step = 0; time_step < 1300; time_step++) {

		if(time_step < 300) {
			for(int i = 0; i < numParticles; i++) {
				Vector3d particle = particles[i], newParticle;
				motionModel(particle, Vector2d(75, 0), particles[i], Vector2d(75, 0));
			}	
		}
		else if(time_step < 800) {
			for(int i = 0; i < numParticles; i++) {
				Vector3d particle = particles[i], newParticle;
				motionModel(particle, Vector2d(0, -75), particles[i], Vector2d(0, -75));
			}
		}
		else {
			for(int i = 0; i < numParticles; i++) {
				Vector3d particle = particles[i], newParticle;
				motionModel(particle, Vector2d(-75, 0), particles[i], Vector2d(-75, 0));
			}
		}

		if(time_step % 100 == 0) {
			for(int i = 0; i < numParticles; i++) 
				fprintf(testFile, "%lf %lf %lf\n", particles[i](0), particles[i](1), particles[i](2)); 
		}
	}

	fclose(testFile);
	printf("Test finished.\n");
}

/* ********************************************************************************************* */
void keyPressed(unsigned char key, int x, int y) {
	usleep(100);
	if (key == ' ')  limitMouse = !limitMouse;
	else if(key == 'a') {
		mouse_x = 640, mouse_y = mouse_cy;
		glutWarpPointer(mouse_x, mouse_y);
		keyInput = !keyInput;
	}
	else if(key == 'd') {
		mouse_x = 790, mouse_y = mouse_cy;
		glutWarpPointer(mouse_x, mouse_y);
		keyInput = !keyInput;
	}
	else if(key == 'w') {
		mouse_x = mouse_cx, mouse_y = mouse_cy - 75;
		glutWarpPointer(mouse_x, mouse_y);
		keyInput = !keyInput;
	}
	else if(key == 't') {
		test();
	}
}

/* ********************************************************************************************* */
int main(int argc, char **argv) {  

	// Initialize landmarks
//	landmarks.push_back(Vector2d(1.9, 0.2));
//	 landmarks.push_back(Vector2d(-2.5, 0.2));
	srand(time(NULL));
	float width = 7.25, height = 4.85;
	for(int i = 0; i < 20; i++) {
		double r1 = (((double) rand()) / RAND_MAX) - 0.5;
		double r2 = (((double) rand()) / RAND_MAX) - 0.5;
		landmarks.push_back(Vector2d(width * r1, height * r2));
	}
//	landmarks.push_back(Vector2d(1.5, 1.2));
//	landmarks.push_back(Vector2d(0.9, 0.6));
	for(int i = 0; i < landmarks.size(); i++) seen.push_back(0);

	// Initialize particles
	for(int i = 0; i < numParticles; i++) particles.push_back(Vector3d::Zero());

	// GL stuff
  glutInit(&argc, argv);  
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);  
  glutInitWindowSize(800, 540);  
  glutInitWindowPosition(0, 0);  
  window = glutCreateWindow("Monte Carlo Localization: No visual feedback except 1/0");
  glutDisplayFunc(&DrawGLScene);  
  glutIdleFunc(&DrawGLScene);
  glutKeyboardFunc(&keyPressed);
	glutPassiveMotionFunc(&passive);
  InitGL(800, 540);

	// Set the middle mouse location
	int screen_pos_x = glutGet((GLenum)GLUT_WINDOW_X);
	int screen_pos_y = glutGet((GLenum)GLUT_WINDOW_Y); 
	mouse_cx = screen_pos_x + 715;
	mouse_cy = screen_pos_y + 455;
	printf("screen_pos: (%d, %d)\n", screen_pos_x, screen_pos_y);

  glutMainLoop();  

  return 1;
}
/* ********************************************************************************************* */
