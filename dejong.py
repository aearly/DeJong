import sys, math, png, random

# the dejung constants, chosen randomly
a = 0.0234
b = -0.01245
c = -a
d = -b

# pixel buffers
rbuffer = []
gbuffer = []
bbuffer = []

# color1
c1 = (1.0, 0.0, 1.0)
c2 = (0.0, 1.0, 1.0)

def lerp(a, b, t):
	return (1-t) * a + b * t;

# the dejung iteration
def next_point(x, y):
	global a, b, c, d
	x1 = math.sin(y * a) - math.cos(x * b)# + random.uniform(-0.001, 0.001)
	y1 = math.sin(x * c) - math.cos(y * d)# + random.uniform(-0.001, 0.001)
	return (x1, y1)

def expose(width, distance, x, y):
	global rbuffer, gbuffer, bbuffer
	# the floor of the coords
	bx = int(x)
	by = int(y)
	# the remainder
	xc = x - bx
	yc = y - by
	# calculate the 4 partial values
	xy = (1 - xc) * (1 - yc)
	xy1 = (1 - xc) * yc
	x1y = xc * (1 - yc)
	x1y1 = xc * yc
	# increment the components of this pixel in the buffer
	c = map(lambda x: lerp(x[0], x[1], distance), zip(c1,  c2))
	rbuffer[by  ][bx  ] += xy * c[0]
	rbuffer[by  ][bx+1] += x1y * c[0]
	rbuffer[by+1][bx  ] += xy1 * c[0]
	rbuffer[by+1][bx+1] += x1y1 * c[0]

	gbuffer[by  ][bx  ] += xy * c[1]
	gbuffer[by  ][bx+1] += x1y * c[1]
	gbuffer[by+1][bx  ] += xy1 * c[1]
	gbuffer[by+1][bx+1] += x1y1 * c[1]

	bbuffer[by  ][bx  ] += xy * c[2]
	bbuffer[by  ][bx+1] += x1y * c[2]
	bbuffer[by+1][bx  ] += xy1 * c[2]
	bbuffer[by+1][bx+1] += x1y1 * c[2]


width = 1024
run_size = 1024
iterations = 1024 * 10240

def de_jung(width, run_size, iterations):
	global rbuffer, gbuffer, bbuffer
	rbuffer = [[1] * width for i in range(width)]
	gbuffer = [[1] * width for i in range(width)]
	bbuffer = [[1] * width for i in range(width)]
	w2 = width/2.0
	w5 = width/5.0
	rw2 = 1.0 / (width * 2)
	point = (0.0, 0.0)
	lastpoint = (0.0, 0.0)
	for iter in range(iterations / run_size):
		for i in range(run_size):
			point = next_point(*point)
			point = map(lambda x: x * w5 + w2, point)
			distance = (math.fabs(point[0] - lastpoint[0]) + math.fabs(point[1] - lastpoint[1])) * rw2
			expose(width, distance, *point)
			lastpoint = point

de_jung(width, run_size, iterations)

# find red normalization
rmaxval = []
for i in range(width):
	rmaxval.append(max(rbuffer[i]))

rmaxval = max(rmaxval)
rlogscale = 255.0 / math.log(rmaxval)

# find green normalization
gmaxval = []
for i in range(width):
	gmaxval.append(max(gbuffer[i]))

gmaxval = max(gmaxval)
glogscale = 255.0 / math.log(gmaxval)

# find blue normalization
bmaxval = []
for i in range(width):
	bmaxval.append(max(bbuffer[i]))

bmaxval = max(bmaxval)
blogscale = 255.0 / math.log(bmaxval)

# normalize pixels, append to buffer
buffer = []
for i in range(width):
	rbuffer[i] = map(lambda x: int(math.log(x) * rlogscale), rbuffer[i])
	gbuffer[i] = map(lambda x: int(math.log(x) * glogscale), gbuffer[i])
	bbuffer[i] = map(lambda x: int(math.log(x) * blogscale), bbuffer[i])
	buffer += reduce(lambda x, y: list(x) + list(y), zip(rbuffer[i], gbuffer[i], bbuffer[i]))

#image = png.from_array(buffer, "RGB")
#image.save("out.png")
writer = png.Writer(size=(width, width), bitdepth=8)
writer.write_array(open("out.png", "w"), buffer)