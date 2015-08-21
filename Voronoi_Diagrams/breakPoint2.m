% @file breakPoint2.m
% @author Can Erdogan
% @date 2015-08-20
% @brief Given two points and a horizontal line, computes the circle that
% the two points are on. Simpler than breakPoint1.

function [] = breakPoint2 ()

    A = [2,7];
    B = [7,5];
    A = [4.00, 8.17];
    B = [2.68, 6.07];
    
    C = [0,6.07-0.01];
    D = [10,6.07-0.01];
    P = breakPoint(A,B,C,D)
    figure(2);
    plot(P(1), P(2), 'o'); hold on;
    P2 = test(A,B,C(2))

    
%     C = [0,3];
%     D = [10,3];
%     for i = 1 : 10
%         C2 = [C(1), C(2) - i * 0.3];
%         D2 = [D(1), D(2) - i * 0.3];
%         P = breakPoint(A,B,C2,D2)
%         figure(2);
%         plot(P(1), P(2), 'o'); hold on;
%         P2 = test(A,B,C2(2))
%         assert(norm(P-P2) < 1e-4);
%     end
    
end

function returnVal = test (C, D, y0) 

    x1 = C(1); y1 = C(2); x2 = D(1); y2 = D(2);
    m2 = (y2 - y1) / (x2 - x1); m = (-1/m2);
    d = norm(C-D)/2;
    k = (y2 + y1) / 2.0 - y0;
    
    a = 1 / (m * m);
    b = - 2 * (1 + 1 / (m * m)) * k;
    c = k * k * ( 1 + 1 / (m * m) ) + d * d;
    b * b - 4 * a * c;
    delta = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
    
    yn = y0 + delta;
    xn = ((x1 + x2) / 2) - 1 / m * (k - delta);
    returnVal = [xn,yn];
end

function returnVal = breakPoint (C, D, A, B) 


    figure(1);
    plot(C(1), C(2), 'o'); hold on;
    plot(D(1), D(2), 'o'); hold on;
    plot([A(1); B(1)], [A(2); B(2)], '-'); hold on;
    axis equal
    axis([0 20 0 20]); hold on;

    % Find intersection of CD and AB lines
    m1 = (B(2) - A(2)) / (B(1) - A(1));
    b1 = A(2) - m1 * A(1);
    m2 = (D(2) - C(2)) / (D(1) - C(1));
    b2 = C(2) - m2 * C(1);
    x = (b2 - b1) / (m1 - m2);
    y = m1 * x + b1;
    plot(x,y,'ro'); hold on;
    F = [x,y];

    % Find J
    d = sqrt(norm(C-F) * norm(D-F));
    dir = (A - B) / norm(A-B);
    J = F + dir * d;
    plot(J(1), J(2),'ko'); hold on;

    % Find the two bijectors of the triangle (C,D,J)
    b = (C+D)/2;
    bperp = (C - D) / norm(C-D);
    bdir = [-bperp(2), bperp(1)];
    b2 = b + bdir * 100;
    plot([b(1); b2(1)], [b(2); b2(2)], 'r-'); hold on;
    c = (J+D)/2;
    cperp = (D - J) / norm(D-J);
    cdir = [-cperp(2), cperp(1)];
    c2 = c + cdir * 100;
    plot([c(1); c2(1)], [c(2); c2(2)], 'r-'); hold on;

    % Find their intersection: center of the circle
    m1 = (b2(2) - b(2)) / (b2(1) - b(1));
    b1 = b(2) - m1 * b(1);
    m2 = (c2(2) - c(2)) / (c2(1) - c(1));
    b2 = c(2) - m2 * c(1);
    x = (b2 - b1) / (m1 - m2);
    y = m1 * x + b1;
    C = [x,y];
    plot(C(1), C(2),'ko'); hold on;

    % Draw the circle
    r = norm(C-D);
    for i = 1 : 32
        a = (2 * pi) * (i / 32.0);
        a2 = (2 * pi) * ((i+1) / 32.0);
        p1 = C + r * [cos(a), sin(a)];
        p2 = C + r * [cos(a2), sin(a2)];
        plot([p1(1);p2(1)],[p1(2);p2(2)],'r-'); hold on;
    end

    axis equal
    axis([0 20 0 20]); hold on;
    clear points

    returnVal = C;
    C;
end
