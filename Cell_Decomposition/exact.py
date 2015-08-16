# @file bug0.py
# @author Can Erdogan
# @date 2015-08-14
# @brief Simple bug 0 implementation for 2D particle motion planning

import time
import math
import sys
from graphics import *
from operator import itemgetter
from sets import Set

#-------------------------------------------------------------------------------------------------
# Read the data points from the file
def readData ():

	# Read the points
	n = []
	with open("data.txt") as input_file:
		for line in input_file:
			n.append(map(float, line.split()))

	# Shift the points
	for i in xrange(0,len(n)):
		n[i][0] = 60 * n[i][0] + 100
		n[i][1] = 600 - 60 * n[i][1]
		if i < 4: n[i].extend([0,i])
		elif i < 9: n[i].extend([1,i-4])
		else: n[i].extend([2, i-9])

	# Create the polygons
	polys = []
	polys.append(Polygon(Point(n[0][0], n[0][1]), Point(n[1][0], n[1][1]), Point(n[2][0], n[2][1]), 
		Point(n[3][0], n[3][1])))
	polys.append(Polygon(Point(n[4][0], n[4][1]), Point(n[5][0], n[5][1]), Point(n[6][0], n[6][1]),
		Point(n[7][0], n[7][1]), Point(n[8][0], n[8][1])))
	polys.append(Polygon(Point(n[9][0], n[9][1]), Point(n[10][0], n[10][1]), Point(n[11][0], n[11][1]),
		Point(n[12][0], n[12][1]), Point(n[13][0], n[13][1])))

	# Create a list of all the edges in the graph
	edges = []
	for i in xrange(0,4): edges.append([[n[i][0], n[i][1]], [n[(i+1)%4][0], n[(i+1)%4][1]]])
	ind = xrange(4,9)
	for i in xrange(0,5):
		edges.append([[n[ind[i]][0], n[ind[i]][1]], [n[ind[(i+1)%5]][0], n[ind[(i+1)%5]][1]]])
	ind = xrange(9,15)
	for i in xrange(0,5):
		edges.append([[n[ind[i]][0], n[ind[i]][1]], [n[ind[(i+1)%5]][0], n[ind[(i+1)%5]][1]]])

	return (n, edges, polys)

#-------------------------------------------------------------------------------------------------
def intersects (xa, ya, xb, yb, xc, yc, xd, yd):

	dbg = False
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
# Draws a circle
def drawCircle (p, r, col, w, win):
	c = Circle(p, r)
	c.draw(win)
	c.setOutline(col)
	c.setWidth(w)
	return c

#-------------------------------------------------------------------------------------------------
def drawMap (n, polys, edges, win):

	# Draw the polygons
	for i in xrange(0,len(polys)):
		p = polys[i]
		p.draw(win)
		p.setWidth(4)
		if i != 0: p.setFill(color_rgb(150,150,150))

	# Vertices
	for i in xrange(0,len(n)):
		drawCircle(Point(n[i][0], n[i][1]), 4, color_rgb(0,0,0), 2, win)
		#t = Text(Point(n[i][0], n[i][1]), str(i))
		#t.draw(win)
		#t.setSize(36)

	# Edges
	for i in xrange(0,len(edges)):
		e = edges[i]
		m_x = 0.5 * (e[0][0] + e[1][0])
		m_y = 0.5 * (e[0][1] + e[1][1])
		#t = Text(Point(m_x, m_y), str(i))
		#t.draw(win)
		#t.setSize(36)

#-------------------------------------------------------------------------------------------------
# Determines which kind of vertex: can go up/down, can go up, can go down, can't go anywhere
def vertexType (v_i, pvs, win):

	dbg = 0

	# Get the points
	p0 = pvs[(v_i - 1) % len(pvs)]
	p1 = pvs[v_i % len(pvs)]
	p2 = pvs[(v_i + 1) % len(pvs)]
	if len(pvs) == 4: (p0, p2) = (p2, p0)

	# Get the angle between the edges
	d01 = math.sqrt((p0.getX() - p1.getX()) * (p0.getX() - p1.getX()) + 
		(p0.getY() - p1.getY()) * (p0.getY() - p1.getY()))
	v01_x = (p0.getX() - p1.getX()) / d01
	v01_y = (p0.getY() - p1.getY()) / d01
	d21 = math.sqrt((p2.getX() - p1.getX()) * (p2.getX() - p1.getX()) + 
		(p2.getY() - p1.getY()) * (p2.getY() - p1.getY()))
	v21_x = (p2.getX() - p1.getX()) / d21
	v21_y = (p2.getY() - p1.getY()) / d21
	dot = v01_x * v21_x + v01_y * v21_y
	maxAngle = math.acos(dot)
	projPerp = -v01_y * v21_x + v01_x * v21_y
	if projPerp > 0: maxAngle = 2 * math.pi - maxAngle
	if dbg == 1: print "maxAngle: %f" % (maxAngle / math.pi * 180.0)
	
	# Get the angle between the first edge and the upper vertical
	up = False
	dot = v01_x * 0.0 + v01_y * -1.0
	angle = math.acos(dot)
	projPerp = -v01_y * 0.0 + v01_x * -1.0
	if projPerp > 0: angle = 2 * math.pi - angle
	if dbg == 1: print "angle: %f" % (angle / math.pi * 180.0)
	if angle > maxAngle: up = True
	if dbg == 1: print "up: %d" % up

	# Get the angle between the first edge and the lower vertical
	down = False
	dot = v01_x * 0.0 + v01_y * 1.0
	angle = math.acos(dot)
	projPerp = -v01_y * 0.0 + v01_x * 1.0
	if projPerp > 0: angle = 2 * math.pi - angle
	if dbg == 1: print "angle: %f" % (angle / math.pi * 180.0)
	if angle > maxAngle: down = True
	if dbg == 1: print "down: %d" % down

	return 2 * up + down

#-------------------------------------------------------------------------------------------------
# For a given vertex, returns the edges up and above the vertex if they exist.
def upAndDown (v_i, pvs, edges, win):

	dbg = False 

	# Get the point and find out its type
	p = pvs[v_i % len(pvs)]
	t_ = vertexType(v_i, pvs, win)

	# Determine the higher and lower neighboring edge 
	pn1 = pvs[(v_i + 1) % len(pvs)] 
	pn2 = pvs[(v_i - 1) % len(pvs)] 
	if pn2.getY() > pn1.getY(): (highp, lowp) = (pn2, pn1)
	else: (highp, lowp) = (pn1, pn2)
	(highe_i, lowe_i) = (0, 0)
	for i in xrange(0,len(edges)):	 # terrible way to organize data...
		p0 = edges[i][0]
		p1 = edges[i][1]
		if ((((math.fabs(p0[0] - p.getX()) < 1e-4) and (math.fabs(p0[1] - p.getY()) < 1e-4)) and \
		((math.fabs(p1[0] - highp.getX()) < 1e-4) and (math.fabs(p1[1] - highp.getY()) < 1e-4))) or \
		(((math.fabs(p1[0] - p.getX()) < 1e-4) and (math.fabs(p1[1] - p.getY()) < 1e-4)) and \
		((math.fabs(p0[0] - highp.getX()) < 1e-4) and (math.fabs(p0[1] - highp.getY()) < 1e-4)))):
			lowe_i = i
		if ((((math.fabs(p0[0] - p.getX()) < 1e-4) and (math.fabs(p0[1] - p.getY()) < 1e-4)) and \
		((math.fabs(p1[0] - lowp.getX()) < 1e-4) and (math.fabs(p1[1] - lowp.getY()) < 1e-4))) or \
		(((math.fabs(p1[0] - p.getX()) < 1e-4) and (math.fabs(p1[1] - p.getY()) < 1e-4)) and \
		((math.fabs(p0[0] - lowp.getX()) < 1e-4) and (math.fabs(p0[1] - lowp.getY()) < 1e-4)))):
			highe_i = i
	res = []
	res.append([min(highe_i, lowe_i),max(highe_i, lowe_i), [p.getX(), p.getY(), p.getY()]])

	# Debugging information
	if dbg: debugObjs.append(drawCircle(p, 6, color_rgb(0,200,0), 4, win))

	# If a up-vertical can be made, get the first edge it would touch
	up = []
	if (t_ > 1): 

		up_cuts = []
		for i in xrange(0,len(edges)):

			p0 = edges[i][0]
			p1 = edges[i][1]
			if (math.fabs(p0[0] - p.getX()) < 1e-4) and (math.fabs(p0[1] - p.getY()) < 1e-4): continue
			if (math.fabs(p1[0] - p.getX()) < 1e-4) and (math.fabs(p1[1] - p.getY()) < 1e-4): continue
			resUp = intersects(p0[0], p0[1], p1[0], p1[1], p.getX(), p.getY(), p.getX(), -100)
			if resUp[2] == 1: 
				up_cuts.append([resUp[0],resUp[1],i])
		
		# Sort the intersections and get the lowest 'y' value one
		if len(up_cuts) > 0:
			up_cuts = sorted(up_cuts, key=itemgetter(1), reverse=True)
			up = up_cuts[0]
			#res.append(up)
			res.append([min(up[2], highe_i), max(up[2], highe_i), [p.getX(), min(p.getY(), up[1]), max(p.getY(), up[1])]])
			res.append([min(up[2], lowe_i), max(up[2], lowe_i), [p.getX(), min(p.getY(), up[1]), max(p.getY(), up[1])]])
			if dbg:
				debugObjs.append(drawCircle(Point(up[0], up[1]), 6, color_rgb(0,200,0), 4, win))
				debugObjs.append(drawLine(edges[up[2]][0],edges[up[2]][1],color_rgb(200,0,0),4,win))
				debugObjs.append(drawLine([up[0],up[1]],[p.getX(),p.getY()],color_rgb(200,0,200),3,win))

	# If a down-vertical can be made, get the first edge it would touch
	down = []
	if (t_ == 1) or (t_ == 3):
		down_cuts = []
		for i in xrange(0,len(edges)):

			p0 = edges[i][0]
			p1 = edges[i][1]
			if (math.fabs(p0[0] - p.getX()) < 1e-4) and (math.fabs(p0[1] - p.getY()) < 1e-4): continue
			if (math.fabs(p1[0] - p.getX()) < 1e-4) and (math.fabs(p1[1] - p.getY()) < 1e-4): continue
			resUp = intersects(p0[0], p0[1], p1[0], p1[1], p.getX(), p.getY(), p.getX(), 700)
			if resUp[2] == 1: 
				down_cuts.append([resUp[0],resUp[1],i])
		
		# Sort the intersections and get the lowest 'y' value one
		if len(down_cuts) > 0:
			down_cuts = sorted(down_cuts, key=itemgetter(1))
			down = down_cuts[0]
			# res.append(down)
			res.append([min(down[2], lowe_i), max(down[2], lowe_i), [p.getX(), min(p.getY(), down[1]), max(p.getY(), down[1])]])
			res.append([min(down[2], highe_i), max(down[2], highe_i), [p.getX(), min(p.getY(), down[1]), max(p.getY(), down[1])]])
			if dbg:
				debugObjs.append(drawCircle(Point(down[0], down[1]), 6, color_rgb(0,200,0), 4, win))
				debugObjs.append(drawLine(edges[down[2]][0],edges[down[2]][1],color_rgb(200,0,0),4,win))
				debugObjs.append(drawLine([down[0], down[1]],[p.getX(),p.getY()],color_rgb(200,0,200),3,win))

	# Add the neighboring edges
	if dbg:
		for i in xrange(0,len(edges)):

			p0 = edges[i][0]
			p1 = edges[i][1]
			if ((math.fabs(p0[0] - p.getX()) < 1e-4) and (math.fabs(p0[1] - p.getY()) < 1e-4)):
				#res.append([p1[0], p1[1], i])
				debugObjs.append(drawLine(p0, p1, color_rgb(200,0,0),4,win))
			if ((math.fabs(p1[0] - p.getX()) < 1e-4) and (math.fabs(p1[1] - p.getY()) < 1e-4)):
				#res.append([p0[0], p0[1], i])
				debugObjs.append(drawLine(p0, p1, color_rgb(200,0,0),4,win))

	if t_ == 3: res.append([min(up[2], down[2]), max(up[2],down[2]), [p.getX(), min(up[1], down[1]), max(up[1], down[1])], -1]) # -1 to avoid adding key point there
	# print res

	if dbg:
		win.getMouse()
		for o in debugObjs: o.undraw()

	return res

#-------------------------------------------------------------------------------------------------
def main():

	win = GraphWin("My Circle", 800, 600, autoflush=False)

	# Read the data file, draw the map and sort vertices based on 'x' values
	(verts, edges, polys) = readData()
	drawMap(verts, polys, edges, win)
	vertsSorted = sorted(verts, key=itemgetter(0), reverse=False)

	# Get the edges that would be added for each vertex and create the cells
	cuts = []
	cutxs = []
	cells = []
	for i in xrange(0, len(edges)): 
		cuts.append([])
		cutxs.append([])
	for v_i in xrange(0,len(vertsSorted)):

		# Get the new edge pairings
		v = vertsSorted[v_i]
		pvs = polys[v[2]].getPoints()
		cs = upAndDown(v[3], pvs, edges, win)

		# Check for polygons
		for c in cs:
			print c
			if c[0] in cuts[c[1]]:

				# Add the new cell
				(ind10,ind01) = (cuts[c[1]].index(c[0]),cuts[c[0]].index(c[1]))
				cells.append([c[0],c[1],cutxs[c[1]][ind10],c[2]])

				# Draw the polygon
				p1 = Point(cutxs[c[1]][ind10][0], cutxs[c[1]][ind10][1]) 
				p2 = Point(cutxs[c[1]][ind10][0], cutxs[c[1]][ind10][2]) 
				p4 = Point(c[2][0], c[2][1])
				p3 = Point(c[2][0], c[2][2])
				P = Polygon (p1, p2, p3, p4)
				P.draw(win)
				P.setWidth(4)
				P.setOutline(color_rgb(0,100,0))
				pm = Point((p1.getX() + p2.getX() + p3.getX() + p4.getX()) / 4.0, (p1.getY() + p2.getY() + p3.getY() + p4.getY()) / 4.0)
				drawCircle(pm, 4, color_rgb(0,150,0), 4, win)
				drawCircle(Point(p1.getX(), (p1.getY() + p2.getY()) / 2.0), 4, color_rgb(0,150,0), 4, win)
				drawCircle(Point(p3.getX(), (p3.getY() + p4.getY()) / 2.0), 4, color_rgb(0,150,0), 4, win)
				t = Text(pm, str(len(cells)-1))
				t.draw(win)
				t.setSize(36)

				# Remove the paired cuts
				cuts[c[1]].pop(ind10)
				cutxs[c[1]].pop(ind10)
				cuts[c[0]].pop(ind01)
				cutxs[c[0]].pop(ind01)

			# Add new cuts to the mapping
			else:
				cuts[c[0]].append(c[1])
				cuts[c[1]].append(c[0])
				cutxs[c[0]].append(c[2])
				cutxs[c[1]].append(c[2])

	# Create a graph from the cell edge midpoints and centers
	for c_i in xrange(0,len(cells)):
		for c2_i in xrange(c_i+1,len(cells)):

			#print "\n--------- %d vs %d --------------------------------" % (c_i, c2_i)

			# Get cell shapes, midpoints
			(vs1a, vs1b) = (cells[c_i][2], cells[c_i][3])
			(vs2a, vs2b) = (cells[c2_i][2], cells[c2_i][3])
			if (vs1a[0] > vs1b[0]): (vs1a, vs1b) = (vs1b, vs1a)
			if (vs2a[0] > vs2b[0]): (vs2a, vs2b) = (vs2b, vs2a)

			#print vs1a, " | ", vs1b
			#print vs2a, " | ", vs2b

			# If the two cells share the same vertical line, see if their line segments intersect
			if ((math.fabs(vs1b[0] - vs2a[0]) < 1e-4) and \
				(not(vs1b[1] > vs2a[2]) and not(vs1b[2] < vs2a[1]))):

					print "neighs: (%d, %d)" % (c_i, c2_i)
					ma = [(vs1a[0] + vs1b[0]) / 2, (vs1a[1] + vs1a[2] + vs1b[1] + vs1b[2]) / 4]
					mb = [(vs2a[0] + vs2b[0]) / 2, (vs2a[1] + vs2a[2] + vs2b[1] + vs2b[2]) / 4]
					ra = [vs1b[0], (vs1b[1] + vs1b[2]) / 2]
					lb = [vs2a[0], (vs2a[1] + vs2a[2]) / 2]
					if len(vs1b) < 4:
						drawLine(ra, mb, color_rgb(0,100,0),4,win)
					if len(vs2a) < 4:
						drawLine(ma, lb, color_rgb(0,100,0),4,win)
					
		
	#for v in vertsSorted:
	#	upAndDown(v, edges, win)
	#	win.getMouse()
	#	for obj in debugObjs:
	#		obj.undraw()
	#	win.getMouse()

	win.getMouse()
	win.close()
main()

#		# Check for triangles completed with one new edge
#		for c in cs:
#			(e1, e2) = (c[0], c[1])
#			for v in cuts[e1]:
#				if v in cuts[e2]: 
#					cells.append([e1,e2,v])
#					break;
#
#		# Check for triangles completed with two new edge
#		if len(cs) > 1:
#			for i in xrange(0, len(cs)):
#				for j in xrange(i+1, len(cs)):
#					(c1,c2) = (cs[i],cs[j])
#					print "(%d,%d) and (%d,%d)" % (c1[0], c1[1], c2[0], c2[1])
#					if c1[0] == c2[0] and c1[1] in cuts[c2[1]]: cells.append([c1[0], c1[1], c2[1]])
#					elif c1[0] == c2[1] and c1[1] in cuts[c2[0]]: cells.append([c1[0], c1[1], c2[0]])
#					elif c1[1] == c2[0] and c1[0] in cuts[c2[1]]: cells.append([c1[1], c1[0], c2[1]])
#					elif c1[1] == c2[1] and c1[0] in cuts[c2[0]]: cells.append([c1[1], c1[0], c2[0]])
#
#
