import sys, png, math, struct

filename = "out.dat"
width = 1024

buffer = []
f = open(filename, "rb")
chunk = f.read()

maxval = 0.0
for i in range(0, width*width*3*8, 8):
	val = struct.unpack("d", chunk[i : i+8])[0]
	buffer.append(val)
	if val > maxval: maxval = val

print "file read, writing to png"
print "maxval: %d" % maxval

try:
	logscale = 255.0 / math.log(maxval)
except:
	print "Domain error: %d" % maxval
	sys.exit(1)

buffer = map(lambda x: int(math.log(math.fabs(x) + 1) * logscale), buffer)

writer = png.Writer(size=(width, width), bitdepth=8)
writer.write_array(open("out.png", "w"), buffer)

