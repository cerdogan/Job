function P = inter (p0, p1, pc, y0)

      m0 = (p1(2) - p0(2)) / (p1(1) - p0(1))
      b0 = p1(2) - p1(1) * m0;
      
      syms D;
      eq = ((y0 + D - b0) / m0 - pc(1)).^2 + (y0 + D - pc(2)).^2 - D*D;
      Dr = min(double(solve(eq,D)));
      y = y0 + Dr;
      x = (y - b0) / m0;
      
      
      P = [x,y]
%     if(p1(1) < p0(1))
%         temp = p0;
%         p0 = p1;
%         p1 = temp;
%     end
%     
%     % Find the closest point on the line |p0,p1| to pc
%     vc0 = pc - p0;
%     dir = (p1 - p0) / norm(p1-p0);
%     perp = [-dir(2), dir(1)];
%     proj = dot(perp, vc0);
%     pn = pc - proj * perp;
%     plot(pn(1), pn(2), 'mo', 'MarkerSize', 7); hold on;
%     
%     % Get the desired distance from p0 towards p1
%     delta = abs(pc(2) - y0)
%     deltaHorz = sqrt((delta/2)*(delta/2) - proj * proj)
%     
%     % See if the projection is before p0 or not 
%     horzProj = dot(vc0, dir);
%     if(horzProj < 0), deltaHorz = deltaHorz + horzProj; end;
%         
%     
%     P = pn + dir * horzProj;
end