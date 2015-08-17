% @file pot.m
% @author Can Erdogan
% @date 2015-08-17
% @brief Application of potential fields to particle motion planning shown in Matlab

% Define the potential field with the goal-driving force and the point-obstacle repelling forces
syms p q
f = -0.02 * exp(-0.2 * (((p)).^2+(q).^2)) + 0.005 * exp(-0.2 * ((10*p-2).^2+(10*q-6).^2)) ...
      + 0.003 * exp(-0.2 * ((20*p-14).^2+(20*q-2).^2)) ...
      + 0.003 * exp(-0.2 * ((40*p-32).^2+(40*q-28).^2))

% Draw the gradients
clf
x = 0:0.1:1;
y = 0:0.1:1;
[X,Y] = meshgrid(x,y);
gs = gradient(f);
vs = subs(gs, [p,q], {X,Y})
vsx = -10*vs(1:size(vs,1)/2,:)
vsy = -10*vs(size(vs,1)/2+1:end,:)
quiver(X,Y,vsx,vsy,0.5); hold on;

% Draw the contour lines of the potential field
x = 0:0.01:1;
y = 0:0.01:1;
[X,Y] = meshgrid(x,y);
Z = subs(f, [p,q], {X,Y});
contour(X,Y,Z,20,'ShowText','on'); hold on;
axis([0 1 0 1])
axis square

% Drive a particle through the field
v = [1; 1]
h = 0
for i = 1 : 300
    det = subs(gs, [p,q], v)
    v = v - 2 * det
    h = plot(v(1), v(2), 'o'); hold on;
    pause(0.01)
end
