# @file bug0.py
# @author Can Erdogan
# @date 2015-08-14
# @brief Simple bug 0 implementation for 2D particle motion planning

import time
import math
import sys
from graphics import *
from operator import itemgetter

#-------------------------------------------------------------------------------------------------
# Read the data points from the file
def readData ():
	numbers = []
	with open("data.txt") as input_file:
		for line in input_file:
			numbers.append(map(float, line.split()))
	return numbers

#-------------------------------------------------------------------------------------------------
def intersects (xa, ya, xb, yb, xc, yc, xd, yd):

	dbg = 0
	if dbg: print "{(%f,%f),(%f,%f)} vs. {(%f,%f),(%f,%f)}" % (xa, ya, xb, yb, xc, yc, xd, yd)

	# Check if the two lines are parallel
	if (math.fabs(xb - xa) < 1e-4) and (math.fabs(xd - xc) < 1e-4): return [0,0,0]
	elif (math.fabs(yb - ya) < 1e-4) and (math.fabs(yd - yc) < 1e-4): return [0,0,0]
	elif (math.fabs(xb - xa) > 1e-4) and (math.fabs(xd - xc) > 1e-4): 
		m1 = (yb - ya) / (xb - xa)
		m2 = (yd - yc) / (xd - xc)
		if (math.fabs(m1 - m2) < 1e-4): return [0,0,0]

	# Find the intersection of the two lines
	if math.fabs(xb - xa) < 1e-4:				 # First line is vertical
		m2 = (yd - yc) / (xd - xc)
		b2 = yc - m2 * xc
		x = xa
		y = m2 * xa + b2
	elif math.fabs(xd - xc) < 1e-4:			 # Second line is vertical
		m1 = (yb - ya) / (xb - xa)
		b1 = ya - m1 * xa
		x = xc
		y = m1 * xc + b1
	else:																 # Normal case
		m1 = (yb - ya) / (xb - xa)
		b1 = ya - m1 * xa
		m2 = (yd - yc) / (xd - xc)
		b2 = yc - m2 * xc
		x = (b2 - b1) / (m1 - m2)
		y = m1 * x + b1
		
	if dbg: print "\t>> Point: (%f,%f)" % (x, y)
		
	# Check if intersection is within the first line segment
	d1 = math.sqrt((xb - xa) * (xb - xa) + (yb - ya) * (yb - ya))
	v1x = (xb - xa) / d1
	v1y = (yb - ya) / d1
	proj1 = (x - xa) * v1x + (y - ya) * v1y 
	if (proj1 < -1e-3) or (proj1 > d1 + 1e-3): 
		if dbg: print "\t>> (%f, %f, %f, %f)" % (d1, v1x, v1y, proj1)
		if dbg: print "\t>> Point not on 1st segment: %f" % proj1
		return [0,0,0]

	# Check if intersection is within the second line segment
	d2 = math.sqrt((xd - xc) * (xd - xc) + (yd - yc) * (yd - yc))
	v2x = (xd - xc) / d2
	v2y = (yd - yc) / d2
	proj2 = (x - xc) * v2x + (y - yc) * v2y 
	if proj2 < -1e-3 or proj2 > d2 + 1e-3: 
		if dbg: print "\t>> (%f, %f, %f, %f)" % (d2, v2x, v2y, proj2)
		if dbg: print "\t>> Point not on 2nd segment: %f" % proj2
		return [0,0,0]

	# If no parallels and intersection is in segments, return point
	if dbg: print "\t>> Intersection +"
	return [x,y,1]

#-------------------------------------------------------------------------------------------------
# Draws a line
debugObjs = []
def drawLine (p0, p1, c, w, win):
	l = Line(Point(p0[0],p0[1]), Point(p1[0],p1[1]))
	l.draw(win)
	l.setOutline(c)
	l.setWidth(w)
	return l

#-------------------------------------------------------------------------------------------------
def drawMap (vs, win):

	# Vertices
	for i in xrange(0,len(vs)):
		vs[i][0] = 60 * vs[i][0] + 100
		vs[i][1] = 600 - 60 * vs[i][1]
		c = Circle(Point(vs[i][0], vs[i][1]), 4)
		c.draw(win)
		c.setWidth(2)
		c.setFill(color_rgb(0,0,0))

	# Perimeter
	edges = []
	black = color_rgb(0,0,0)
	for i in xrange(0,4):
		drawLine([vs[i][0],vs[i][1]],[vs[(i+1)%4][0],vs[(i+1)%4][1]],black,4,win)
		edges.append([[vs[i][0], vs[i][1]], [vs[(i+1)%4][0], vs[(i+1)%4][1]]])

	# First object
	ind = xrange(4,9)
	for i in xrange(0,5):
		drawLine([vs[ind[i]][0],vs[ind[i]][1]],[vs[ind[(i+1)%5]][0],vs[ind[(i+1)%5]][1]], black,4,win)
		edges.append([[vs[ind[i]][0], vs[ind[i]][1]], [vs[ind[(i+1)%5]][0], vs[ind[(i+1)%5]][1]]])

	# Second object
	ind = xrange(9,15)
	for i in xrange(0,5):
		drawLine([vs[ind[i]][0],vs[ind[i]][1]],[vs[ind[(i+1)%5]][0],vs[ind[(i+1)%5]][1]], black,4,win)
		edges.append([[vs[ind[i]][0], vs[ind[i]][1]], [vs[ind[(i+1)%5]][0], vs[ind[(i+1)%5]][1]]])

	return edges

#-------------------------------------------------------------------------------------------------
# For a given vertex, returns the edges up and above the vertex if they exist.
def upAndDown (v, edges, win):

	g = Circle(Point(v[0],v[1]), 6)
	g.draw(win)
	g.setFill(color_rgb(0,200,0))
	g.setWidth(4)
	debugObjs.append(g)

	# For each edge, check if there is an intersection with {v, [vx,700]} or {v, [vx,-100]}
	ints = []
	ints.append([v[1], -1])
	for i in xrange(0,len(edges)):

		p0 = edges[i][0]
		p1 = edges[i][1]
		if (math.fabs(p0[0] - v[0]) < 1e-4) and (math.fabs(p0[1] - v[1]) < 1e-4): continue
		if (math.fabs(p1[0] - v[0]) < 1e-4) and (math.fabs(p1[1] - v[1]) < 1e-4): continue
		resUp = intersects(p0[0], p0[1], p1[0], p1[1], v[0], -100, v[0], 700)
		if resUp[2] == 1: 
			ints.append([resUp[1],i])
			debugObjs.append(drawLine(p0,p1,color_rgb(200,0,0),4,win))
	
	print ints

#-------------------------------------------------------------------------------------------------
def main():

	win = GraphWin("My Circle", 800, 600, autoflush=False)

	# Read the data file, draw the map and sort vertices based on 'x' values
	verts = readData()
	edges = drawMap(verts, win)
	vertsSorted = sorted(verts, key=itemgetter(0))

	for v in vertsSorted:
		upAndDown(v, edges, win)
		win.getMouse()
		for obj in debugObjs:
			obj.undraw()
		win.getMouse()

	win.getMouse()
	win.close()
main()
