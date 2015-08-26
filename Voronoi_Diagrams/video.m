% Read in the data
clf
data = load('data.txt');
load edges
% 
colors = {'r', 'g', 'b', 'm', 'c', 'y', 'k', 'r'};

if 1 == 0
for m = 1 : size(data,1)
    bs = edges(edges(:,5) == (m-1),:); 
    clf
    for i = 1 : size(edges,1), plot([edges(i,1); edges(i,3)], [edges(i,2); edges(i,4)], '-', 'LineWidth',1); hold on; end; axis([0 10 0 10]);
    for j = 1 : size(bs,1)
        plot([bs(j,1); bs(j,3)], [bs(j,2); bs(j,4)], ['-o',colors{j}], 'LineWidth',2); hold on; 
    end
    axis([0 10 0 10])
    axis square
pause
end
end
    
% Initialize limits
xminG = min(data(:,1)) - 0.5;
xmaxG = max(data(:,1)) + 0.5;
xs = [data(:,1),data(:,1)];

% Simulate
%for y0 = (max(data(:,2) + 0.5)):-0.05:(min(data(:,2) - 0.5))


edgeDone = zeros(size(edges,1), 2);
for i = 1 : size(edges,1)
    if(min(edges(i,1:2)) < 0 || max(edges(i,1:2)) > 10), edgeDone(i,1) = 1; end;
    if(min(edges(i,3:4)) < 0 || max(edges(i,3:4)) > 10), edgeDone(i,2) = 1; end;
end

iter = 0;
for y0 = 9.5:-0.01:-1.2

    iter = iter+1;
    
    % Clean the scene and draw the edges and the sweep line temporarily
    clf
    for i = 1 : size(edges,1)
        if(edgeDone(i,1) == 0), continue; end;
        if(edgeDone(i,2) == 0), continue; end;
        plot([edges(i,1); edges(i,3)], [edges(i,2); edges(i,4)], '-r', 'LineWidth',1); hold on; 
    end; 
    axis([0 10 0 10]);
    plot([xminG;xmaxG],[y0;y0],'r-'); hold on;
    plot(data(:,1),data(:,2),'o'); hold on;
    
    % Draw the beachline for every cell
    for i = 1 : size(data,1)
        
      %  if(i ~= 5), continue; end;
        
        % Skip the data point is beach line didn't reach it yet
        x1 = data(i,1);
        y1 = data(i,2);
        if(y1 < y0), continue; end;
        
        % Sort the polygon in clockwise
        bs = edges(edges(:,5) == (i-1),:);
        ps = [bs(:,1:2); bs(:,3:4)];
        ps = ps - repmat([x1,y1],size(ps,1),1);
        ps(:,3) = cart2pol(ps(:,1),ps(:,2));
        ps = sortrows(ps,3);
        ps = 1.02 * ps + repmat([x1,y1,0],size(ps,1),1);
        
        % Draw the polygon
        xs_ = xminG:0.01:xmaxG;
        ys = ((xs_.^2 - 2*xs_*x1 + (x1*x1 + y1*y1 - y0*y0)) / (2 * (y1-y0)))';
        [in,on] = inpolygon(xs_,ys,ps(:,1),ps(:,2));
        temp = [in]; %,on];
        temp = temp';
        xs_ = xs_';
        xs = xs_(temp,:);
        ys = ys(temp,:);
        plot(x1,y1,'o'); hold on;
        if(size(xs)>0)
            plot(xs,ys,'-','LineWidth',2); hold on;
          %  plot(xs(1),ys(1),'o','LineWidth',2); hold on;
          
          pb = [xs(1),ys(1)]; pe = [xs(end),ys(end)];
          % Draw the edge the breakpoint is on
          for j = 1 : size(edges,1)
              if(edges(j,5) ~= (i-1)), continue; end;
              p1 = edges(j,1:2);
              p2 = edges(j,3:4);
              if(p1(2) > p2(2)), pt = p1; p1 = p2; p2 = pt; end;
              
              dist = norm(p2-p1);
              dir12 = (p2 - p1) / dist;
              dir21 = (p1 - p2) / dist;
              perp = [-dir12(2),dir12(1)];
              dbg = 0;
              if(norm(pb-[7.848,7.884])<1e-1), dbg = 1; end; % && norm(edges(j,1:4)-[3.4920    8.3341    4.5801   12.6429]) < 1e-1) , dbg = 1; end;
              if(dbg == 12)
                  
                  fprintf('----------------------------------------------------------------\n');
                  [p1,p2]
                [dot(perp, (pb-p1)), dot(perp, (pe-p1))]
                %keyboard
                for i = 1 : size(bs,1), plot([bs(i,1); bs(i,3)], [bs(i,2); bs(i,4)], ':r'); hold on; end
              end
              k = 0.2;
              if(((abs(dot(perp, (pb-p1))) < k)) && (dot(dir12, (pb-p1)) < dist) && (dot(dir12, (pb-p1)) >= 0))
                if(edgeDone(j,1) == 1)
                    if(dbg==1), [pb, 0, 1], end;
                    plot([p1(1);pb(1)], [p1(2);pb(2)], '-g','LineWidth',2); hold on;
                end
              end
              if(((abs(dot(perp, (pe-p1))) < k)) && (dot(dir12, (pe-p1)) < dist) && (dot(dir12, (pe-p1)) >= 0))
                if(edgeDone(j,1) == 1)
                    if(dbg==1), [pe, 1, 1], end;
                    plot([p1(1);pe(1)], [p1(2);pe(2)], '-g','LineWidth',2); hold on;
                end
              end
              if(((abs(dot(perp, (pb-p2))) < k)) && (dot(dir21, (pb-p2)) < dist) && (dot(dir21, (pb-p2)) >= 0))
                if(edgeDone(j,2) == 1)
                    if(dbg==1), [pb, 0, 2], end;
                    plot([p2(1);pb(1)], [p2(2);pb(2)], '-g','LineWidth',2); hold on;
                end
              end
              if(((abs(dot(perp, (pe-p2))) < k)) && (dot(dir21, (pe-p2)) < dist) && (dot(dir21, (pe-p2)) >= 0))
                if(edgeDone(j,2) == 1)

                    plot([p2(1);pe(1)], [p2(2);pe(2)], '-g','LineWidth',2); hold on;
                                        if(dbg==1), [pe, 1, 2], end;
                end
              end
            %  plot(pb(1),pb(2),'rx','LineWidth',2); hold on;
              %  plot(pe(1),pe(2),'gx','LineWidth',2); hold on;
              %plot([p1(1);p2(1)],[p1(2);p2(2),'go','LineWidth',2); hold on;
            %  plot(p1(1),p1(2),'ro','LineWidth',2); hold on;
            % plot(p2(1),p2(2),'go','LineWidth',2); hold on;

              % Check if completed the edge information
              
              if(norm(p1-pb) < k), edgeDone(j,1) = 1;
              elseif(norm(p1-pe) < k), edgeDone(j,1) = 1; 
              elseif(norm(p2-pb) < k), edgeDone(j,2) = 1; 
              elseif(norm(p2-pe) < k), edgeDone(j,2) = 1; 
%               elseif(norm(p1-pb) < 1e-1), edgeDone(j,1) = 1;
%               elseif(norm(p1-pe) < 1e-1), edgeDone(j,1) = 1; 
%               elseif(norm(p2-pe) < 1e-1), edgeDone(j,2) = 1; 
%               elseif(norm(p2-pe) < 1e-1), edgeDone(j,2) = 1; 
               end;
              

          end
        end 
    end
    
    axis equal
    axis([xminG, xmaxG, 0, 10]); hold on;
    %if(j >2 ), voronoi(data(1:j,1), data(1:j,2)); 5, end
    
   name = sprintf('fig%04d.png', iter)
   print(name,'-dpng')
   %pause(0.01);
end




plot(data(:,1), data(:,2), 'ro'); hold on;

for i = 1 : size(edges,1), plot([edges(i,1); edges(i,3)], [edges(i,2); edges(i,4)], '--k', 'LineWidth',2); hold on; end
plot(data(:,1), data(:,2), 'ro'); hold on;
axis([0 10 0 10])
plot(data(:,1), data(:,2), 'ro'); hold on;
voronoi(data(:,1), data(:,2))