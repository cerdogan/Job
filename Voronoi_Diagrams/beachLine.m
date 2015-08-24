clf;
data = load('data.txt');
data
xs = 0:0.01:10;
colors = {'r', 'g', 'b', 'm', 'c', 'y', 'k', 'r', 'g', 'b', 'm', 'c', 'y', 'k', ...
    'r', 'g', 'b', 'm', 'c', 'y', 'k', 'r', 'g', 'b', 'm', 'c', 'y', 'k'};
for j = 1 : size(data,1)
    clf
    y0 = -0.012620; 4.87; data(j,2) - 0.0001;
    plot([xs(1);xs(end)],[y0;y0],'r-'); hold on;
    for i = 1 : j %size(data,1)
        %if(i == 5), continue; end;
        x1 = data(i,1);
        y1 = data(i,2);
        ys = (xs.^2 - 2*xs*x1 + (x1*x1 + y1*y1 - y0*y0)) / (2 * (y1-y0));
        c = colors{i};
        if(i < 8), t = '-';
        elseif(i < 15), t='--';
        end
        plot(x1,y1,['o',c]); hold on;
        plot(xs,ys,[t,c]); hold on;
        
    end
    title(['sweep line: ', num2str(y0)]);
    axis equal
    axis([xs(1), xs(end), 0, 20]); hold on;
    %if(j >2 ), voronoi(data(1:j,1), data(1:j,2)); 5, end
   %pause
    
end

