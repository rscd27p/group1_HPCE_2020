# Course: MP-6171 High Performance Embedded Systems
# Tecnologico de Costa Rica (www.tec.ac.cr)
# Developers Name: Andres Gomez (agomez10010@gmail.com) and Randy Cespedes (rcespedes27dds@gmail.com)
# This script is structured in Octave (https://www.gnu.org/software/octave/)
# General purpose: Composite Trapezoidal Rule Prototyping
# Input: Lower value, upper value and sub-intervals
# Output: numerical approximation

# Define constant values for test case 1
a1 = 0;
b1 = 10;
n1 = 200;
expected_result1 = 1.47112726584903;

# Define constant values for test case 2
a2 = 3.5;
b2 = 45;
n2 = 150;
expected_result2 = 0.256335035737283;

# **** Define the functions to use ****

# This function calculates the value of f(x) = y = 1 / (1+ x2)
function y = f(x)
	y = 1 / (1 + x*x);
end

# This function calculates the integral of a function using the composite trapezoidal rule description
function integral = Trapezoidal(lower, upper, subInterval)
	# Start by calculating h
	h = (upper - lower) / subInterval;

	# Calculate initial value for result
	integral = f(lower) + f(upper);

	# Now add the iterative portion
	sum = 0;
	for j = 1:(subInterval-1)
		sum = sum + f(lower + (j*h));
	end

	# Calculate final value for integral result
	integral = h*(integral + 2*sum)/2;
end


# **** Exercise function with test values here and print results ****

integral_result1 = Trapezoidal(a1, b1, n1);
fprintf("Test 1 >> Integral Result: %.10f. Expected Result: %.10f\n\n", integral_result1, expected_result1)

integral_result2 = Trapezoidal(a2, b2, n2);
fprintf("Test 2 >> Integral Result: %.10f. Expected Result: %.10f\n\n", integral_result2, expected_result2)

