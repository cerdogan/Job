% @file circle3points.m
% @author Can Erdogan
% @date 2015-08-21
% @brief Finds the circle that connects three points.

% Read the four points
points = []
clf
axis([0 10 0 10]); hold on;
for i = 1 : 3
    [x,y,m] = ginput(1);
    if(m == 3), break; end;
    points(end+1, :) = [x,y];
    plot(x,y, 'o'); hold on;
end;
A = points(1,:);
B = points(2,:);
C = points(3,:);

% Find the perpendicular bisectors
AB = (A + B) / 2.0;
BC = (B + C) / 2.0;
ABdir = (B-A)/norm(B-A);
ABperp = [ABdir(2), -ABdir(1)];
AB2 = AB + ABperp * 10;
BCdir = (C-B)/norm(C-B);
BCperp = [BCdir(2), -BCdir(1)];
BC2 = BC + BCperp * 10;
plot([AB(1);AB2(1)], [AB(2);AB2(2)], 'g-'); hold on;
plot([BC(1);BC2(1)], [BC(2);BC2(2)], 'b-');

% Find the intersection of the points
m1 = (AB2(2) - AB(2)) / (AB2(1) - AB(1))
b1 = AB(2) - m1 * AB(1)
m2 = (BC2(2) - BC(2)) / (BC2(1) - BC(1))
b2 = BC(2) - m2 * BC(1)
x = (b2 - b1) / (m1 - m2)
y = m1 * x + b1
C = [x,y];
plot(x,y,'ro'); hold on;

% Draw the circle
r = norm(C-A);
for i = 1 : 32
    a = (2 * pi) * (i / 32.0);
    a2 = (2 * pi) * ((i+1) / 32.0);
    p1 = C + r * [cos(a), sin(a)];
    p2 = C + r * [cos(a2), sin(a2)];
    plot([p1(1);p2(1)],[p1(2);p2(2)],'r-'); hold on;
end
axis equal