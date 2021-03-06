clf;
data = load('data.txt');
data
xs = 0:0.1:10;
colors = {'r', 'g', 'b', 'm', 'c', 'y', 'k'};
for y0 = 8.5:-0.01:0
    clf
 %   for i = 1 : size(edges,1), plot([edges(i,1); edges(i,3)], [edges(i,2); edges(i,4)], '-r'); hold on; end
    
    plot([xs(1);xs(end)],[y0;y0],'r-'); hold on;
    for i = 1 : size(data,1)
        
        x1 = data(i,1);
        y1 = data(i,2);
        if(y1 < y0), continue; end;
        ys = (xs.^2 - 2*xs*x1 + (x1*x1 + y1*y1 - y0*y0)) / (2 * (y1-y0));
        c = colors{i};
        plot(x1,y1,['o',c]); hold on;
        plot(xs,ys,['-',c]); hold on;
        
    end
    axis equal
    axis([xs(1), xs(end), 0, 10]); hold on;
    %if(j >2 ), voronoi(data(1:j,1), data(1:j,2)); 5, end
   pause
    
end

