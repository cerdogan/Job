clf
data = load('data.txt');
plot(data(:,1), data(:,2), 'ro'); hold on;
load edges
for i = 1 : size(edges,1), plot([edges(i,1); edges(i,3)], [edges(i,2); edges(i,4)], '--k', 'LineWidth',2); hold on; end
plot(data(:,1), data(:,2), 'ro'); hold on;
axis([0 10 0 10])
plot(data(:,1), data(:,2), 'ro'); hold on;
voronoi(data(:,1), data(:,2))