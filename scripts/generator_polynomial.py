import galois

def calculate_generator_polynomial(t):
    GF256 = galois.GF(2**8)
    alpha = GF256.primitive_element
    
    generator = galois.Poly([1], field=GF256)
    
    for i in range(1, 2*t + 1):
        factor = galois.Poly([1, alpha**i], field=GF256)
        generator = generator * factor
    
    return generator

max_errors = 16
gen_poly = calculate_generator_polynomial(max_errors)
big_endian = list(gen_poly.coeffs)
little_endian = big_endian[::-1]

for i in range (0,len(little_endian)):
    print(little_endian[i])
