import sys, math, png, random

# the dejung constants, chosen randomly
a = 0.0234
b = -0.01245
c = -a
d = -b

# the dejung iteration
def next_point(x, y):
	global a, b, c, d
	x1 = math.sin(y * a) - math.cos(x * b) + random.uniform(-0.001, 0.001)
	y1 = math.sin(x * c) - math.cos(y * d) + random.uniform(-0.001, 0.001)
	return (x1, y1)

def expose(buffer, width, x, y):
	buffer[y][x] += 1

width = 1024
run_size = 1024
iterations = 1024 * 10240

def de_jung(width, run_size, iterations):
	buffer = [[1] * width for i in range(width)]
	w2 = width/2.0
	w5 = width/5.0
	point = (0.0, 0.0)
	for iter in range(iterations / run_size):
		for i in range(run_size):
			point = next_point(*point)
			point = map(lambda x: x * w5 + w2, point)
			if iter == 5 and i == 124:
				print point
			expose(buffer, width, int(point[0]), int(point[1]))
		print iter + " ",
	return buffer

buffer = de_jung(width, run_size, iterations)

maxval = []
for i in range(width):
	maxval.append(max(buffer[i]))

maxval = max(maxval)
logscale = 255.0 / math.log(maxval)

for i in range(width):
	buffer[i] = map(lambda x: int(math.log(x) * logscale), buffer[i])

image = png.from_array(buffer, "L")
image.save("out.png")