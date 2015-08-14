# @file bug0.py
# @author Can Erdogan
# @date 2015-08-14
# @brief Failure case for bug 0 implementation for 2D particle motion planning 

import time
import math
import sys
from graphics import *

#-------------------------------------------------------------------------------------------------
def intersects (xa, ya, xb, yb, xc, yc, xd, yd):

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
		
	# Check if intersection is within the first line segment
	d1 = math.sqrt((xb - xa) * (xb - xa) + (yb - ya) * (yb - ya))
	v1x = (xb - xa) / d1
	v1y = (yb - ya) / d1
	proj1 = (x - xa) * v1x + (y - ya) * v1y 
	if proj1 < 0 or proj1 > d1: return [0,0,0]

	# Check if intersection is within the second line segment
	d2 = math.sqrt((xd - xc) * (xd - xc) + (yd - yc) * (yd - yc))
	v2x = (xd - xc) / d2
	v2y = (yd - yc) / d2
	proj2 = (x - xc) * v2x + (y - yc) * v2y 
	if proj2 < 0 or proj2 > d2: return [0,0,0]

	# If no parallels and intersection is in segments, return point
	return [x,y,1]

#-------------------------------------------------------------------------------------------------
def main():
	win = GraphWin("My Circle", 400, 600, autoflush=False)

	# Draw the obstacle
	grey = color_rgb(100,100,100)
	r2a = Rectangle(Point(80, 250), Point(110, 150))
	r2a.draw(win)
	r2a.setWidth(2)
	r2a.setFill(grey)
	r2b = Rectangle(Point(80, 120), Point(280, 150))
	r2b.draw(win)
	r2b.setWidth(2)
	r2b.setFill(grey)
	r2c = Rectangle(Point(250, 350), Point(280, 150))
	r2c.draw(win)
	r2c.setWidth(2)
	r2c.setFill(grey)
	r2d = Rectangle(Point(280, 380), Point(110, 350))
	r2d.draw(win)
	r2d.setWidth(2)
	r2d.setFill(grey)
	r2e = Rectangle(Point(200, 250), Point(80, 280))
	r2e.draw(win)
	r2e.setWidth(2)
	r2e.setFill(grey)

	objs = []
	objs.append([ Point(200,280), Point(80,280), Point(80,120), Point(280,120), Point(280,380), 
		Point(110,380), Point(110,350), Point(250,350), Point(250,150), Point(110,150), 
		Point(110,250), Point(200,250)])
	for p in objs[0]:
		pc = Circle(p, 3)
		pc.setFill(color_rgb(0,0,0))
		pc.draw(win)
	

	# Draw the goal location
	g = Circle(Point(180,210), 4)
	g.draw(win)
	g.setFill(color_rgb(0,200,0))
	gPos = g.getCenter()

	while True:

		# Draw the initial bug location
		m = win.getMouse() # pause for click in window
		b = Circle(m, 4)
		b.draw(win)
		b.setFill(color_rgb(200,0,0))

		k = 1
		iters = 0
		while True:

			print "\n-------------------------------------------------"
			print "Iters: %d" % iters
			iters = iters + 1

			# Check if the bug is at the goal
			bPos = b.getCenter()
			print "bug pos: %f, %f" % (bPos.getX(), bPos.getY())
			dist = math.sqrt((bPos.getX() - gPos.getX()) * (bPos.getX() - gPos.getX()) +
				(bPos.getY() - gPos.getY()) * (bPos.getY() - gPos.getY()))
			print "dist: %f" % dist
			if dist < 1:
				print "Reached goal!"
				break
			
			
			# Attempt to move toward the goal
			dir_x = (gPos.getX() - bPos.getX()) / dist 
			dir_y = (gPos.getY() - bPos.getY()) / dist
			new_x = bPos.getX() + k * dir_x
			new_y = bPos.getY() + k * dir_y
			print "new bug pos: %f, %f" % (new_x, new_y)

			# Check if the new location is in collision with one of the objects
			intersectEdges = []
			for obj_i in xrange(0,len(objs)):
				obj = objs[obj_i]
				for i in xrange(0,len(obj)):
		
					# Check for intersection
					point = intersects(bPos.getX(), bPos.getY(), new_x, new_y, obj[i].getX(), obj[i].getY(), 
						obj[(i+1)%len(obj)].getX(), obj[(i+1)%len(obj)].getY())
					if point[2] == 1:

						print "point: %f, %f" % (point[0], point[1])

						# Move bug to the intersection point
						temp = 1
						dx = point[0] - bPos.getX() - temp * dir_x
						dy = point[1] - bPos.getY() - temp * dir_y
						print "d: %f, %f" % (dx, dy)
		
						b.move(dx, dy)
						bPos = b.getCenter()
						print "bug pos: %f, %f" % (bPos.getX(), bPos.getY())

						intersectEdges.append([obj_i, i])
						print "\tHit edge %d on obj with %d edges" % (i, len(obj))
			
			# If intersecting with two edges, follow the new one
			if len(intersectEdges) == 2:
				print "two intersecting edges"
				for i in xrange(0,2):
					edge = intersectEdges[i] 
					print "(%d,%d) vs. (%d,%d)" % (edge[0], edge[1], lastEdge[0], lastEdge[1])
					if (edge[0] != lastEdge[0] or edge[1] != lastEdge[1]):
						print "removing an edge"
						intersectEdges = []
						intersectEdges.append(edge)
						break

			# Follow the edge
			if len(intersectEdges) == 1:
				print "following single edge"
				temp = intersectEdges[0]
				obj = objs[temp[0]]
				p0 = obj[temp[1]]
				p1 = obj[(temp[1]+1)%len(obj)]
				print "Follow: (%f, %f), (%f, %f)\n" % (p0.getX(), p0.getY(), p1.getX(), p1.getY())
				dist = math.sqrt((p0.getX() - p1.getX()) * (p0.getX() - p1.getX()) +
					(p0.getY() - p1.getY()) * (p0.getY() - p1.getY()))
				dir_x = (p1.getX() - p0.getX()) / dist 
				dir_y = (p1.getY() - p0.getY()) / dist
				b.move(k * dir_x, k * dir_y)
				lastEdge = intersectEdges[0]
			
			# Move towards the goal
			if len(intersectEdges) == 0:
				print "moving towards goal"
				b.move(k * dir_x, k * dir_y)

			#print "%f, %f" % (dir_x, dir_y)
			sys.stdout.flush()
			win.update()
			time.sleep(0.00166)

		b.undraw()

	win.close()
main()
