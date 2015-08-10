data = load('data.txt');
numData = size(data,1); numFeat = 2;
A = -diag(data(:,numFeat+1)) * [data(:,1:2), ones(numData,1)]
c = -ones(numData,1)
H = eye(numFeat+1); H(numFeat+1,numFeat+1) = 0
f = zeros(numFeat+1,1);
quadprog(H,f,A,c)
xs = [0:0.01:1]
ws = quadprog(H,f,A,c)
ys = -(ws(1) * xs + ws(3)) / ws(2)
plot(data(:,1), data(:,2), 'o')
hold on;
plot(xs, ys, '-k')