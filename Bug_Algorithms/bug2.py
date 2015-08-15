# @file bug2.py
# @author Can Erdogan
# @date 2015-08-14
# @brief Simple bug 2 implementation for 2D particle motion planning

import time
import math
import sys
from graphics import *

#-------------------------------------------------------------------------------------------------
def intersects (xa, ya, xb, yb, xc, yc, xd, yd):

	print "{(%f,%f),(%f,%f)} vs. {(%f,%f),(%f,%f)}" % (xa, ya, xb, yb, xc, yc, xd, yd)

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

	print "\t>> Point: (%f,%f)" % (x, y)
		
	# Check if intersection is within the first line segment
	d1 = math.sqrt((xb - xa) * (xb - xa) + (yb - ya) * (yb - ya))
	v1x = (xb - xa) / d1
	v1y = (yb - ya) / d1
	proj1 = (x - xa) * v1x + (y - ya) * v1y 
	if (proj1 < 0) or (proj1 > d1): 
		print "\t>> (%f, %f, %f, %f)" % (d1, v1x, v1y, proj1)
		print "\t>> Point not on 1st segment: %f" % proj1
		return [0,0,0]

	# Check if intersection is within the second line segment
	d2 = math.sqrt((xd - xc) * (xd - xc) + (yd - yc) * (yd - yc))
	v2x = (xd - xc) / d2
	v2y = (yd - yc) / d2
	proj2 = (x - xc) * v2x + (y - yc) * v2y 
	if proj2 < 0 or proj2 > d2: 
		print "\t>> (%f, %f, %f, %f)" % (d2, v2x, v2y, proj2)
		print "\t>> Point not on 2nd segment: %f" % proj2
		return [0,0,0]

	# If no parallels and intersection is in segments, return point
	print "\t>> Intersection +"
	return [x,y,1]

#-------------------------------------------------------------------------------------------------
# Checks if the bug can move toward the goal. If it can, updates the bug position and returns 
# 1. Otherwise, return 0. If bug at goal, returns 2.
def headTowardGoal (objs, b, gPos):

	dbg = 0

	# Check if the bug is at the goal
	bPos = b.getCenter()
	print "bug pos: %f, %f" % (bPos.getX(), bPos.getY())
	dist = math.sqrt((bPos.getX() - gPos.getX()) * (bPos.getX() - gPos.getX()) +
		(bPos.getY() - gPos.getY()) * (bPos.getY() - gPos.getY()))
	if dbg: print "dist: %f" % dist
	if dist < 1:
		if dbg: print "Reached goal!"
		return [2,0,0]

	# Attempt to move toward the goal
	k = 3.0
	dir_x = (gPos.getX() - bPos.getX()) / dist 
	dir_y = (gPos.getY() - bPos.getY()) / dist
	new_x = bPos.getX() + k * dir_x
	new_y = bPos.getY() + k * dir_y
	if dbg: print "new bug pos: %f, %f" % (new_x, new_y)

	# Check if the new location is in collision with one of the objects
	intersectEdges = []
	for obj_i in xrange(0,2):
		obj = objs[obj_i]
		for i in xrange(0,len(obj)):

			# Check for intersection
			asdf = -0.01
			bPosX_ = bPos.getX() - asdf * dir_x
			bPosY_ = bPos.getY() - asdf * dir_y
			point = intersects(bPosX_, bPosY_, new_x, new_y, obj[i].getX(), obj[i].getY(), 
				obj[(i+1)%len(obj)].getX(), obj[(i+1)%len(obj)].getY())

			# If intersection, snap the object to the object and return true for hit
			if point[2] == 1:
				dx = point[0] - bPos.getX()
				dy = point[1] - bPos.getY()
				b.move(dx, dy)
				return [1,obj_i,i] 

	# Update the bug position
	k = 1
	b.move(k * dir_x, k * dir_y)
	return [0,0,0]

#-------------------------------------------------------------------------------------------------
# Attempts to move the bug along the given side of the object. If the end of the edge is reached,
# returns 0. If the starting position is reached, returns 1. Updates the closest dist if one
# is reached as well.
def followEdge (edges, edgeId, b, gPos, startPos, closestDist, iters):

	k = 1

	# Get the edge information
	p0 = edges[edgeId % len(edges)]
	p1 = edges[(edgeId + 1) % len(edges)]
	eDist = math.sqrt((p0.getX() - p1.getX()) * (p0.getX() - p1.getX()) +
		(p0.getY() - p1.getY()) * (p0.getY() - p1.getY()))
	v_x = (p1.getX() - p0.getX()) / eDist
	v_y = (p1.getY() - p0.getY()) / eDist

	# Check if the bug reached the end vertex (if so, snap it)
	bPos = b.getCenter()
	dist = math.sqrt((bPos.getX() - p1.getX()) * (bPos.getX() - p1.getX()) +
		(bPos.getY() - p1.getY()) * (bPos.getY() - p1.getY()))
	print "end vertex dist: %f" % dist
	if dist < k:
		b.move(p1.getX() - bPos.getX(), p1.getY() - bPos.getY())
		return [0, 0]

	# Check if the bug reached the starting position along the edge
	dist = math.sqrt((bPos.getX() - startPos.getX()) * (bPos.getX() - startPos.getX()) +
		(bPos.getY() - startPos.getY()) * (bPos.getY() - startPos.getY()))
	if iters > 5 and dist < k:
		return [1, 0]

	# Update the bug location by moving it along the object
	b.move(k * v_x, k * v_y)
	bPos = b.getCenter()
	
	# Update the closest point
	dist = math.sqrt((bPos.getX() - gPos.getX()) * (bPos.getX() - gPos.getX()) +
		(bPos.getY() - gPos.getY()) * (bPos.getY() - gPos.getY()))
	if dist < closestDist:
		return [2, dist]
	
	return [3, 0]

#-------------------------------------------------------------------------------------------------
# Returns the next point on the object surface that crosses the line between the starting position 
# and the goal position
def nextLinePoint (edges, edgeId, sPos, gPos):

	for i in xrange(0,len(edges)):
		p0 = edges[(i + edgeId) % len(edges)]
		p1 = edges[(i + edgeId + 1) % len(edges)]
		res = intersects(p0.getX(), p0.getY(), p1.getX(), p1.getY(), sPos.getX(), sPos.getY(), 
			gPos.getX(), gPos.getY())
		if res[2] == 1:
			return Point(res[0], res[1])

#-------------------------------------------------------------------------------------------------
def main():
	win = GraphWin("My Circle", 400, 600, autoflush=False)

	# Draw the first obstacle
	r1 = Rectangle(Point(70, 360), Point(330, 410))
	r1.draw(win)
	r1.setWidth(2)
	grey = color_rgb(100,100,100)
	r1.setFill(grey)
	objs = []
	objs.append([Point(70,360), Point(330,360), Point(330,410), Point(70,410)])

	# Draw the second obstacle in two parts
	r2a = Rectangle(Point(130, 250), Point(230, 300))
	r2a.draw(win)
	r2a.setWidth(2)
	r2a.setFill(grey)
	r2b = Rectangle(Point(230, 300), Point(280, 150))
	r2b.draw(win)
	r2b.setWidth(2)
	r2b.setFill(grey)
	objs.append([Point(130,250), Point(230,250), Point(230,150), Point(280,150), Point(280,300),Point(130,300)])

	# Draw the goal location
	g = Circle(Point(210,210), 4)
	g.draw(win)
	g.setFill(color_rgb(0,200,0))
	gPos = g.getCenter()

	# Draw the initial bug location
	while True:

		m = win.getMouse() # pause for click in window
		#m = Point(110, 450)
		sPos = m
		b = Circle(m, 4)
		b.draw(win)
		b.setFill(color_rgb(200,0,0))

		k = 3.0
		k2 = 1.0
		iters = 0

		navigating = 0
		currEdgeId = 0
		navEdges = []
		nextPoint = Point(0,0)
		collisionResult = []
		goBest = 0
		reachedBest = 0
		navIters = 0
		nextCircle = Circle(Point(2000,2000), 4)
		nextCircle.setFill(color_rgb(0,0,200))
		nextCircle.draw(win)
		greedyIters = 0
		while True:

			# Debugging information
			print "\n-------------------------------------------------"
			print "Iters: %d" % iters
			iters = iters + 1
		
			# Check if the bug can move to the goal directly (if reached, stop; if good, try again)
			if not(navigating):

				# Attempt the motion
				collisionResult = headTowardGoal(objs, b, gPos)
				greedyIters = greedyIters + 1
				print "headTowardGoal result: %d" % collisionResult[0]
				if collisionResult[0] == 2: break

				# For a collision, initialize the navigation
				if (greedyIters > 1) and (collisionResult[0] == 1): 
					navigating = 1
					navEdges = objs[collisionResult[1]]
					currEdgeId = collisionResult[2]
					navIters = 0
	
					# Get the next line intersection point
					nextPoint = nextLinePoint (navEdges, currEdgeId + 1, sPos, gPos)
					nextCircle.move(nextPoint.getX() - nextCircle.getCenter().getX(), 
						nextPoint.getY() - nextCircle.getCenter().getY())
			
			# If there was a collision, circumnavigate the object 
			if navigating == 1:

				# Move along the current edge of the object
				followRes = followEdge(navEdges, currEdgeId, b, gPos, nextPoint, -1, navIters)
				navIters = navIters + 1
				print "followEdge result: %d" % followRes[0]
					
				# If the end of the edge is reached, move on to the next edge
				if followRes[0] == 0: currEdgeId = currEdgeId + 1

				# If the next position is reached, stop navigating and go to normal mode
				if followRes[0] == 1:
					navigating = 0
					nextCircle.move(2000,2000)
					navIters = 0

			win.update()
			time.sleep(0.00166)

		b.undraw()

	win.close()
main()
