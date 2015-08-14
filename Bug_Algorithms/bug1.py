# @file bug0.py
# @author Can Erdogan
# @date 2015-08-14
# @brief Simple bug 0 implementation for 2D particle motion planning

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
	#objs.append([Point(330,410), Point(70,410)])

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

	while True:

		# Draw the initial bug location
		#m = win.getMouse() # pause for click in window
		m = Point(110, 450)
		b = Circle(m, 4)
		b.draw(win)
		b.setFill(color_rgb(200,0,0))

		k = 3.0
		k2 = 1.0
		iters = 0

		minDist = 10000000
		circumnavigate = 0
		circumStart = Point(0,0)
		circumBest = Point(0,0)
		goBest = 0
		reachedBest = 0
		while True:

			print "\n-------------------------------------------------"
			print "Iters: %d" % iters
			iters = iters + 1

			# Check if the bug is at the goal
			bPos = b.getCenter()
			print "bug pos: %f, %f" % (bPos.getX(), bPos.getY())
			dist = math.sqrt((bPos.getX() - gPos.getX()) * (bPos.getX() - gPos.getX()) +
				(bPos.getY() - gPos.getY()) * (bPos.getY() - gPos.getY()))
			# print "dist: %f" % dist
			if dist < 1:
				print "Reached goal!"
				break
			
			# Check if circumnavigation should stop and should go to the closest location
			distCircumStart = math.sqrt((bPos.getX() - circumStart.getX()) * (bPos.getX() - circumStart.getX()) +
				(bPos.getY() - circumStart.getY()) * (bPos.getY() - circumStart.getY()))
			if (circumnavigate == 1) and (distCircumStart < 1e-1):
				circumnavigate = 0
				goBest = 1
				
			# Check if reached closest location in circumnavigation
			distCircumBest = math.sqrt((bPos.getX() - circumBest.getX()) * (bPos.getX() - circumBest.getX()) +
				(bPos.getY() - circumBest.getY()) * (bPos.getY() - circumBest.getY()))
			if (goBest == 1) and (distCircumBest < 1e-1):
				reachedBest = 1
				goBest = 0

			print "c-nav: %d, go-best: %d, c-best: (%f,%f), c-start: (%f,%f), dist-best: %f, dist-start: %f" % (circumnavigate, goBest, circumBest.getX(), circumBest.getY(), circumStart.getX(), circumStart.getY(), distCircumBest, distCircumStart)

			# Attempt to move toward the goal
			dir_x = (gPos.getX() - bPos.getX()) / dist 
			dir_y = (gPos.getY() - bPos.getY()) / dist
			new_x = bPos.getX() + k * dir_x
			new_y = bPos.getY() + k * dir_y
			print "new bug pos: %f, %f" % (new_x, new_y)

			# Check if the new location is in collision with one of the objects
			intersectEdges = []
			for obj_i in xrange(0,1):
				obj = objs[obj_i]
				for i in xrange(0,len(obj)):
		
					#i = 2

					# Check for intersection
					asdf = 0.01
					bPosX_ = bPos.getX() - asdf * dir_x
					bPosY_ = bPos.getY() - asdf * dir_y
					point = intersects(bPosX_, bPosY_, new_x, new_y, obj[i].getX(), obj[i].getY(), 
						obj[(i+1)%len(obj)].getX(), obj[(i+1)%len(obj)].getY())
					if point[2] == 1:

						#print "point: %f, %f" % (point[0], point[1])

						# Move bug to the intersection point
						temp = 0
						dx = point[0] - bPos.getX() - temp * dir_x
						dy = point[1] - bPos.getY() - temp * dir_y
						b.move(dx, dy)
						bPos = b.getCenter()

						intersectEdges.append([obj_i, i])
						print "\tHit edge %d on obj with %d edges" % (i, len(obj))
			
						# Start circumnavigating if not already
						if not(circumnavigate) and not(goBest) and not(reachedBest):
							circumnavigate = 1
							circumStart = bPos
							circStart = Circle(circumStart, 4)
							circStart.draw(win)
							circStart.setFill(color_rgb(0,0,200))
						

			# print "# inter. edges: %d" % len(intersectEdges)

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
				#print "Follow: (%f, %f), (%f, %f)\n" % (p0.getX(), p0.getY(), p1.getX(), p1.getY())
				dist = math.sqrt((p0.getX() - p1.getX()) * (p0.getX() - p1.getX()) +
					(p0.getY() - p1.getY()) * (p0.getY() - p1.getY()))
				dir_x = (p1.getX() - p0.getX()) / dist 
				dir_y = (p1.getY() - p0.getY()) / dist
				b.move(k2 * dir_x, k2 * dir_y)
				lastEdge = intersectEdges[0]
				
				# Update the minimum distance to the goal
				bPos = b.getCenter()
				dist = math.sqrt((bPos.getX() - gPos.getX()) * (bPos.getX() - gPos.getX()) +
					(bPos.getY() - gPos.getY()) * (bPos.getY() - gPos.getY()))
				if dist < minDist:
					minDist = dist
					circumBest = bPos
			
			# Move towards the goal
			if not(circumnavigate) and not(goBest) and len(intersectEdges) == 0:
				print "moving towards goal"
				b.move(k2 * dir_x, k2 * dir_y)
				reachedBest = 0
		
				# Reset minimum dists
				for i in xrange(0,2):
					minDist = 1000000

			#print "%f, %f" % (dir_x, dir_y)
			sys.stdout.flush()
			win.update()
			time.sleep(0.0166)

		b.undraw()

	win.close()
main()
