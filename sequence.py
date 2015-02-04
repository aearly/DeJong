import sys, os, subprocess, time, math


def parse_args (argv):
  opts = {"argv": []}
  while len(argv):
    arg = argv.pop(0)
    if arg[0] == '-':
      name = arg.split('-')[-1]
      if arg[1] == '-':
        opts[name] = True
      else:
        opts[name] = argv.pop(0)
    else:
      opts["argv"].append(arg)

  return opts

opts = parse_args(sys.argv[1:])

print opts

width = opts["w"] if "w" in opts else "256"
x = float(opts["x"]) if "x" in opts else 1.25
y = float(opts["y"]) if "y" in opts else 2.33
r = float(opts["r"]) if "r" in opts else 0.15
iterations = opts["i"] if "i" in opts else "20"
scale = opts["scale"] if "scale" in opts else "20"
steps = float(opts["steps"]) if "steps" in opts else 30.0

directory = opts["directory"] if "directory" in opts else str(int(round(time.time() * 1000)))

os.mkdir(directory)

for step in range(int(steps)):
  theta = math.pi * 2.0 * float(step) / steps
  a = x + r * math.cos(theta)
  b = y + r * math.sin(theta)
  outfile = "%s/%04d-%1.3fx%1.3f.png" % (directory, step, a, b)

  cmd = "./dejong -w %s -s %s -a %1.3f -b %1.3f -i %s -o %s" % (width, scale, a, b, iterations, outfile)
  print outfile
  ret = subprocess.call(cmd, shell=True)


