data = load('data.txt');
numData = size(data,1); numFeat = 2;
H = zeros(numData);
for i = 1 : numData
    for j = 1 : numData
        H(i,j) = data(i, 3) * data(j, 3) * dot(data(i,1:2), data(j,1:2));
    end
end
f = -ones(numData,1);

I = -eye(numData);
i0 = zeros(numData,1);

E = data(:,3)';
e0 = 0;

l = eps^.5;
H = H + l * eye(numData);
a = quadprog(H,f,I,i0,E,e0)

w = (a .* data(:,3))' * data(:,1:2)
i=min(find((a>0.1)&(data(:,3)==1)))
G=data(:,1:2)*data(:,1:2)'
b=1-G(i,:)*(a.*data(:,3))

plot(data(:,1), data(:,2), 'o')
hold on;
plot(xs, ys, '-k')