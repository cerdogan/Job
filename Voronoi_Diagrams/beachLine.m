clf;
data = load('data.txt');

xs = 0:0.01:20;
colors = {'r', 'g', 'b', 'm', 'c', 'y', 'k'};
for j = 1 : size(data,1)
    clf
    y0 = data(j,2) - 0.01
    plot([xs(1);xs(end)],[y0;y0],'r-'); hold on;
    for i = 1 : j %size(data,1)
        x1 = data(i,1);
        y1 = data(i,2);
        ys = (xs.^2 - 2*xs*x1 + (x1*x1 + y1*y1 - y0*y0)) / (2 * (y1-y0))
        c = colors{i};
        plot(x1,y1,['o',c]); hold on;
        plot(xs,ys,['-',c]); hold on;
    end
    axis([xs(1), xs(end), 0, 32]); hold on;
    pause
end

