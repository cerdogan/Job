syms D y0 b0 m0 cx cy
eq = ((y0 + D - b0) / m0 - cx).^2 + (y0 + D - cy).^2 - D*D;
coeffs(eq, D)