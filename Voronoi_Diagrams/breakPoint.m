% @file circle.m
% @author Can Erdogan
% @date 2015-08-20
% @brief Given two points and a tangent line, computes the circle that
% the two points are on. Uses "Power of a Point Theorem" as explained in
% http://www.cut-the-knot.org/Curriculum/Geometry/GeoGebra/PPL.shtml#solution.

% Read the four points
if ( exist('points') == 0 )
    points = []
    clf
    axis([0 10 0 10]); hold on;
    for i = 1 : 4
        [x,y,m] = ginput(1);
        if(m == 3), break; end;
        points(end+1, :) = [x,y];
        plot(x,y, 'o'); hold on;
    end;
end

C = points(1,:);
D = points(2,:);
A = points(3,:);
B = points(4,:);

plot(C(1), C(2), 'o'); hold on;
plot(D(1), D(2), 'o'); hold on;
plot([A(1); B(1)], [A(2); B(2)], '-'); hold on;

% Find intersection of CD and AB lines
m1 = (B(2) - A(2)) / (B(1) - A(1))
b1 = A(2) - m1 * A(1)
m2 = (D(2) - C(2)) / (D(1) - C(1))
b2 = C(2) - m2 * C(1)
x = (b2 - b1) / (m1 - m2)
y = m1 * x + b1
plot(x,y,'ro'); hold on;
F = [x,y];

% Find J
d = sqrt(norm(C-F) * norm(D-F))
dir = (A - B) / norm(A-B);
J = F + dir * d;
plot(J(1), J(2),'ko'); hold on;

% Find the two bijectors of the triangle (C,D,J)
b = (C+D)/2;
bperp = (C - D) / norm(C-D);
bdir = [-bperp(2), bperp(1)];
b2 = b + bdir * 100;
plot([b(1); b2(1)], [b(2); b2(2)], 'r-'); hold on;
c = (J+D)/2;
cperp = (D - J) / norm(D-J);
cdir = [-cperp(2), cperp(1)];
c2 = c + cdir * 100;
plot([c(1); c2(1)], [c(2); c2(2)], 'r-'); hold on;

% Find their intersection: center of the circle
m1 = (b2(2) - b(2)) / (b2(1) - b(1))
b1 = b(2) - m1 * b(1)
m2 = (c2(2) - c(2)) / (c2(1) - c(1))
b2 = c(2) - m2 * c(1)
x = (b2 - b1) / (m1 - m2)
y = m1 * x + b1
C = [x,y];
plot(C(1), C(2),'ko'); hold on;

% Draw the circle
r = norm(C-D);
for i = 1 : 32
    a = (2 * pi) * (i / 32.0)
    a2 = (2 * pi) * ((i+1) / 32.0)
    p1 = C + r * [cos(a), sin(a)];
    p2 = C + r * [cos(a2), sin(a2)];
    plot([p1(1);p2(1)],[p1(2);p2(2)],'r-'); hold on;
end

axis equal
axis([0 10 0 10]); hold on;
clear points
