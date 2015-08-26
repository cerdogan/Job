function [P,res] = inter (p0, p1, pc, y0)

      m0 = (p1(2) - p0(2)) / (p1(1) - p0(1));
      b0 = p1(2) - p1(1) * m0;
      
      cx = pc(1); cy = pc(2);
      a = (cx + (b0 - y0)/m0)^2 + (cy - y0)^2;
      b = 2*y0 - 2*cy - (2*(cx + (b0 - y0)/m0))/m0;
      c = 1/m0^2;
      Dr2 = (-b - sqrt(b*b-4*a*c))/(2*c);
      
      %Dr = min(double(solve(eq,D)));
      %[Dr, Dr2]
      y = y0 + Dr2;
      x = (y - b0) / m0;
      if(imag(Dr2) ~= 0), P = [0,0]; res = 0; return; end;
      
      P = [x,y];
      res = 1;
      
      len = norm(p1-p0);
      dir = (p1 - p0) / len;
      proj = dot(dir, (p1-P));
      if(proj > len || proj < 0), res = 0; end;
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