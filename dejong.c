#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <png.h>
#include "commander/commander.c"
#include "writepng/writepng.c"

/*
 * Good ol' globals
 */

// the dejung constants, chosen randomly
double a = 0.0134;
double b = -0.00743;
double c, d;
// buffer for image data
double *buffer;
uch *png_buffer;
char *outfile = "out.png";
// how big
int width = 1024;
// how many iterations
int iterations = 1024 * 1024 * 1;
// pretty colors to use
double c1[3] = {1.0, 0.0, 1.0};
double c2[3] = {0.0, 1.0, 1.0};

static void set_a(command_t* cmd)
{
  a = atof(cmd->arg);
}

static void set_b(command_t* cmd)
{
  b = atof(cmd->arg);
}

static void set_iterations(command_t* cmd)
{
  iterations = 1024 * 1024 * atoi(cmd->arg);
}

static void set_outfile(command_t* cmd)
{
  int filename_len = strlen(cmd->arg);

 	outfile = malloc(sizeof(char) * filename_len);

  memcpy(outfile, cmd->arg, filename_len);
}

void create_wpng(mainprog_info* wpng_info)
{
  wpng_info->width = width;
  wpng_info->height = width;
  wpng_info->pnmtype = 6; //rgb
  wpng_info->sample_depth = 8;


  wpng_info->infile = NULL;
  wpng_info->outfile = fopen(outfile, "wb");
  wpng_info->image_data = png_buffer;
  wpng_info->row_pointers = NULL;
  wpng_info->filter = FALSE;
  wpng_info->interlaced = FALSE;
  wpng_info->have_bg = FALSE;
  wpng_info->have_time = FALSE;
  wpng_info->have_text = 0;
  wpng_info->gamma = 1.0;

}

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
    buffer[i] = 0.0;

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

void normalize_buffer()
{
	double max = 0;
	long buffer_size = width * width * 3;
	double scale_factor;

	for(uint i = 0; i < buffer_size; i++) {
		if (buffer[i] > max) {
			max = buffer[i];
		}
	}

	scale_factor = 255.0 / (float)max;

	png_buffer = malloc(buffer_size * sizeof(uch));

	for (uint i = 0; i < buffer_size; i++) {
		png_buffer[i] = (uch)((double)buffer[i] * scale_factor);
	}
}

void write_png()
{
  mainprog_info wpng_info;

  printf("Writing to file...\n");
  create_wpng(&wpng_info);

  writepng_init(&wpng_info);

  uch* row = png_buffer;

  for (uint i = 0; i < width; i++) {
  	wpng_info.image_data = row;
  	writepng_encode_row(&wpng_info);
  	row += width * 3 * sizeof(uch);
  }

  writepng_encode_finish(&wpng_info);
  writepng_cleanup(&wpng_info);
  fclose(wpng_info.outfile);
}

int main (int argc, char **argv)
{
  command_t cmd;
  command_init(&cmd, argv[0], "0.0.1");
  command_option(&cmd, "-a", "--a [a]", "DeJong 'a' constant", set_a);
  command_option(&cmd, "-b", "--b [b]", "DeJong 'b' constant", set_b);
  command_option(&cmd, "-i", "--iterations [n]", "iteration factor", set_iterations);
  command_option(&cmd, "-o", "--outfile [file]", "output file", set_outfile);
  command_parse(&cmd, argc, argv);

  c = -a;
  d = -b;

  printf("a: %1.3f, b: %1.3f\n", a, b);


  de_jong();


  printf("Done iterating. Normalizing...\n");
  normalize_buffer();

  write_png();

  return 0;
}
