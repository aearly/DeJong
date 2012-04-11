#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/*
 * Good ol' globals
 */

// the dejung constants, chosen randomly
double a = 0.0134;
double b = -0.00743;
double c, d;
// buffer for image data
double *buffer;
// how big
int width = 1024;
// how many iterations
int iterations = 1024 * 1024 * 1000;
// pretty colors to use
double c1[3] = {1.0, 0.0, 1.0};
double c2[3] = {0.0, 1.0, 1.0};

// standard linear interpolation function
double lerp(double a, double b, double t)
{
	return (1-t) * a + b * t;
}

// the dejung iteration
void next_point(double x, double y, 
                  double *x1, double *y1)
{
	*x1 = sin(y * a) - cos(x * b);
	*y1 = sin(x * c) - cos(y * d);
}

void expose_pixel(int width, double distance,
	              double x, double y)
{
	// the floor of the values, to use as the 
	// array indexes
	int bx = (int)x;
	int by = (int)y;

	// The remainder
	double xc = x - (double)bx;
	double yc = y - (double)by;

	// calculate the partial pixel values
	double xy = (1 - xc) * (1 - yc);
	double xy1 = (1 - xc) * yc;
	double x1y = xc * (1 - yc);
	double x1y1 = xc * yc;

	// interpolate the color based on the 
	// distance param
	double c[3];
	c[0] = lerp(c1[0], c2[0], distance);
	c[1] = lerp(c1[1], c2[1], distance);
	c[2] = lerp(c1[2], c2[2], distance);

	// update buffer
	buffer[(((by  )*width + (bx  )) * 3) + 0] += xy * c[0];
	buffer[(((by  )*width + (bx  )) * 3) + 1] += xy * c[1];
	buffer[(((by  )*width + (bx  )) * 3) + 2] += xy * c[2];

	buffer[(((by  )*width + (bx+1)) * 3) + 0] += x1y * c[0];
	buffer[(((by  )*width + (bx+1)) * 3) + 1] += x1y * c[1];
	buffer[(((by  )*width + (bx+1)) * 3) + 2] += x1y * c[2];

	buffer[(((by+1)*width + (bx  )) * 3) + 0] += xy1 * c[0];
	buffer[(((by+1)*width + (bx  )) * 3) + 1] += xy1 * c[1];
	buffer[(((by+1)*width + (bx  )) * 3) + 2] += xy1 * c[2];

	buffer[(((by+1)*width + (bx+1)) * 3) + 0] += x1y1 * c[0];
	buffer[(((by+1)*width + (bx+1)) * 3) + 1] += x1y1 * c[1];
	buffer[(((by+1)*width + (bx+1)) * 3) + 2] += x1y1 * c[2];
}

void de_jong()
{
	// buffer is a width x width RGB array of doubles
	buffer = malloc(width * width * 3 * sizeof(double));
	for(int i = 0; i < width*width*3; i++)
		buffer[i] = 1.0;

	double w2 = ((double)width)/2.0;
	double w5 = ((double)width)/5.0;
	double rw2 = 1.0 / ((double)width * 2);
	double lastx, lasty, x, y, distance;
	lastx = lasty = x = y = 0.0;

	for(uint i = 0; i < iterations; i++)
	{
		next_point(lastx, lasty, &x, &y);
		x = x * w5 + w2;
		y = y * w5 + w2;
		distance = (fabs(x - lastx) + fabs(y - lasty)) * rw2;
		expose_pixel(width, distance, x, y);
		lastx = x;
		lasty = y;
	}
}

int main (int argc, char **argv)
{
	c = -a;
	d = -b;
	de_jong();
	printf("Done iterating. Writing to file...\n");
	// Output to file
	FILE *fp = fopen("out.dat", "wb");
	fwrite(buffer, sizeof(double), width * width * 3, fp);
	fclose(fp);

	return 0;
}
