% Read in the data
clf
data = load('data.txt');
load edges

% Initialize limits
xminG = min(data(:,1)) - 0.5;
xmaxG = max(data(:,1)) + 0.5;
xs = repmat([xminG, xmaxG], size(data,1), 1);

% Simulate
%for y0 = (max(data(:,2) + 0.5)):-0.05:(min(data(:,2) - 0.5))
for y0 = 7.9:-0.05:(min(data(:,2) - 0.5))

    clf
    for i = 1 : size(edges,1), plot([edges(i,1); edges(i,3)], [edges(i,2); edges(i,4)], '-k', 'LineWidth',2); hold on; end; axis([0 10 0 10]);
    plot([xminG;xmaxG],[y0;y0],'r-'); hold on;
    for i = 1 : size(data,1)
        
        % Skip the data point is beach line didn't reach it yet
        x1 = data(i,1);
        y1 = data(i,2);
        if(y1 < y0), continue; end;
        
        % Check if reached a breakpoint for this cell at this height
        bs = edges(edges(:,5) == (i-1),:);
        %bs = bs(bs(:,2) < y0 & bs(:,4) > y0, :);
        for j = 1 : size(bs,1)
            if(i == 1 && j == 4)
                P = inter(bs(j,1:2), bs(j,3:4), data(i,1:2), y0);
                plot([bs(j,1); bs(j,3)], [bs(j,2); bs(j,4)], '-r', 'LineWidth',2); hold on; 
                plot(P(1),P(2),'go','MarkerSize',7,'LineWidth',2); hold on;
                %pause
            end
        end
        
%         if(size(bs,1) > 0)
%             P = inter(bs(1,1:2), bs(1,3:4), data(i,1:2), 0.5)
%             m0 = (bs(1,4) - bs(1,2)) / (bs(1,3) - bs(1,1));
%             b0 = bs(1,4) - m0 * bs(1,3);
%             x0 = (y0 - b0) / m0;
%             %plot(x0,y0,'ro','MarkerSize',4,'LineWidth',2); hold on;
%             plot(P(1),P(2),'ro','MarkerSize',7,'LineWidth',2); hold on;
%             plot([bs(1,1); bs(1,3)], [bs(1,2); bs(1,4)], '-r', 'LineWidth',2); hold on; 
%         %    if(x0 < x1), xs(i,1) = x0;
%         %    elseif(x0 > x1), xs(i,2) = x0; end;
%              
%         end
%         if(size(bs,1) == 2)
%              P = inter(bs(2,1:2), bs(2,3:4), data(i,1:2), 0.5)
%              plot(P(1),P(2),'go','MarkerSize',7,'LineWidth',2); hold on;
%              plot([bs(2,1); bs(2,3)], [bs(2,2); bs(2,4)], '-r', 'LineWidth',2); hold on; 
%             m2 = (bs(2,4) - bs(2,2)) / (bs(2,3) - bs(2,1));
%             b2 = bs(2,4) - m2 * bs(2,3);
%             x2 = (y0 - b2) / m2;
%             %plot(x2,y0,'go','MarkerSize',8,'LineWidth',2); hold on;
%           %  if(x0 < x2), xs(i,1) = x2;
%          %   elseif(x0 > x2), xs(i,2) = x2; end;
%         end
        
        % Draw the polygon
        xs_ = xs(i,1):0.01:xs(i,2);
        ys = (xs_.^2 - 2*xs_*x1 + (x1*x1 + y1*y1 - y0*y0)) / (2 * (y1-y0));
        plot(x1,y1,'o'); hold on;
        plot(xs_,ys,'-'); hold on;
        
    end
    axis equal
    axis([xminG, xmaxG, 0, 10]); hold on;
    %if(j >2 ), voronoi(data(1:j,1), data(1:j,2)); 5, end
    return
   pause;(0.001)
    
end




plot(data(:,1), data(:,2), 'ro'); hold on;

for i = 1 : size(edges,1), plot([edges(i,1); edges(i,3)], [edges(i,2); edges(i,4)], '--k', 'LineWidth',2); hold on; end
plot(data(:,1), data(:,2), 'ro'); hold on;
axis([0 10 0 10])
plot(data(:,1), data(:,2), 'ro'); hold on;
voronoi(data(:,1), data(:,2))