clf
load result
drawGaussian(result(1,:), result(2:3,:))
drawGaussian(result(4,:), result(5:6,:))
drawGaussian(result(7,:), result(8:9,:))
data = load('data2.txt');
plot(data(:,1), data(:,2), 'o');
title('Expectation-Maximization: Mixture of Gaussians Example', 'FontSize', 24);