#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <png.h>
#include "commander/commander.c"
#include "writepng/writepng.c"

typedef double Double4 __attribute__ ((vector_size (sizeof(double) * 4)));

/*
 * Good ol' globals
 */

// the dejung constants, chosen randomly
double a = 3.4;
double b = -5.43;
double c, d;
// buffer for image data
Double4 *buffer;
uch *png_buffer;
char *outfile = "out.png";
// how big
int width = 1024;
// how many iterations
int iterations = 1024 * 1024 * 1;
// scale type.  2 = log
int scale_type = 2;
// pretty colors to use
Double4 c1 = {1.0, 0.2, 0.0, 1.0};
Double4 c2 = {0.0, 1.0, 1.0, 1.0};
Double4 black = {0.0, 0.0, 0.0, 1.0};

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

static void set_width(command_t* cmd)
{
  width = atoi(cmd->arg);
}

static void set_scale_type(command_t* cmd)
{
  scale_type = atoi(cmd->arg);
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

  if (!wpng_info->outfile) {
    fprintf(stderr, "couldn't open outfile.\n");
    exit(1);
  }

  wpng_info->image_data = png_buffer;
  wpng_info->row_pointers = NULL;
  wpng_info->filter = FALSE;
  wpng_info->interlaced = FALSE;
  wpng_info->have_bg = FALSE;
  wpng_info->have_time = FALSE;
  wpng_info->have_text = 0;
  wpng_info->gamma = 1.0 / 2.2;

}

// standard linear interpolation function
double lerp(double a, double b, double t)
{
  return (1-t) * a + b * t;
}

Double4 lerp4(Double4 a, Double4 b, double t) {
  return (1-t) * a + b * t;
}

// the dejong iteration
void next_point(double x, double y,
                  double *x1, double *y1)
{
  *x1 = sin(y * a) - cos(x * b);
  *y1 = sin(x * c) - cos(y * d);
}

Double4 cl1 = {0.0, 0.0, 1.0, 1.0};
Double4 cl2 = {1.0, 0.0, 0.0, 1.0};
Double4 cl3 = {0.0, 1.0, 0.0, 1.0};

Double4 getColor(double dx, double dy) {
  double distance = (fabs(dx) + fabs(dy)) / 4.0;
  if (distance < 0) distance = 0;
  if (distance > 1.0) distance = 1.0;

  return lerp4(c1, c2, distance);
}

Double4 getColor2(double dx, double dy) {
  double distance = (fabs(dx) + fabs(dy)) / 2.0;
  if (distance < 0) distance = 0;
  if (distance > 2.0) distance = 2.0;

  if (distance <= 1.0) {
    return lerp4(cl1, cl2, distance);
  } else {
    return lerp4(cl2, cl3, distance);
  }
}

void de_jong()
{

  double w2 = ((double)width)/2.0;
  double w5 = ((double)width)/4.0 - 1.0;
  double lastx, lasty, x, y, dx, dy;

  void expose_pixel()
  {
    // scale to fit the dimennsions of the image
    int sx = x * w5 + w2;
    int sy = y * w5 + w2;

    // the floor of the values, to use as the
    // array indexes
    int bx = (int)sx;
    int by = (int)sy;

    // The remainder
    double xc = sx - (double)bx;
    double yc = sy - (double)by;

    // calculate the partial pixel intensities
    double xy = (1 - xc) * (1 - yc);
    double xy1 = (1 - xc) * yc;
    double x1y = xc * (1 - yc);
    double x1y1 = xc * yc;

    // interpolate the color based on the
    // distance param
    Double4 c = getColor(dx, dy);

    // update buffer
    buffer[(by  )*width + bx  ] += xy * c;
    buffer[(by  )*width + bx+1] += x1y * c;
    buffer[(by+1)*width + bx  ] += xy1 * c;
    buffer[(by+1)*width + bx+1] += x1y1 * c;
  }

  // buffer is a width x width RGBA array of Double4 vectors
  printf("Allocating buffer...\n");
  buffer = aligned_alloc(sizeof(Double4), width * width * sizeof(Double4));
  printf("Initializing buffer...\n");
  for(int i = 0; i < width*width; i++) {
    buffer[i] = black;
    /*buffer[i][0] = 0.0;
    buffer[i][1] = 0.0;
    buffer[i][2] = 0.0;
    buffer[i][3] = 1.0;*/
  }

  lastx = lasty = x = y = 0.0;

  for(uint i = 0; i < iterations; i++)
  {
    next_point(lastx, lasty, &x, &y);
    dx = x - lastx;
    dy = y - lasty;
    lastx = x;
    lasty = y;
    expose_pixel();
  }
}

double max4(Double4 vec) {
  double max = 0;
  if (vec[0] > max) max = vec[0];
  if (vec[1] > max) max = vec[1];
  if (vec[2] > max) max = vec[2];
  return max;
}

void normalize_buffer()
{
  uch scale_linear(double v) {
    return (uch)(255.0 * v);
  }

  uch scale_quad(double v) {
    return (uch)(255.0 * sqrt(v));
  }

  uch scale_log(double v) {
    return (uch)((255.0 / log(2)) * log(v + 1));
  }

  uch scale_log10(double v) {
    return (uch)((255.0 / log(scale_type + 1)) * log(v * scale_type + 1));
  }

  double max = 0;
  long buffer_size = width * width;
  double scale_factor;
  uch (*scale)(double);
  Double4 color;
  double colorMax;

  switch (scale_type) {
  case 0:
    scale = &scale_linear; break;
  case 1:
    scale = &scale_quad; break;
  case 2:
    scale = &scale_log;
  case 3:
  default:
    scale = &scale_log10; break;
  }

  for(uint i = 0; i < buffer_size; i++) {
    color = buffer[i];
    colorMax = max4(color);
    if (colorMax > max) {
      max = colorMax;
    }
  }

  printf("Max value: %20.3f\n", max);

  scale_factor = 1.0 / (float)max;

  png_buffer = malloc(buffer_size * 3 * sizeof(uch));

  for (uint i = 0; i < buffer_size; i++) {
    color = buffer[i];
    png_buffer[i * 3 + 0] = (*scale)(color[0] * scale_factor);
    png_buffer[i * 3 + 1] = (*scale)(color[1] * scale_factor);
    png_buffer[i * 3 + 2] = (*scale)(color[2] * scale_factor);
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
  command_option(&cmd, "-a", "--a <a>", "DeJong 'a' constant", set_a);
  command_option(&cmd, "-b", "--b <b>", "DeJong 'b' constant", set_b);
  command_option(&cmd, "-i", "--iterations [n]", "iteration factor", set_iterations);
  command_option(&cmd, "-w", "--width [n]", "width (and height)", set_width);
  command_option(&cmd, "-o", "--outfile [file]", "output file", set_outfile);
  command_option(&cmd, "-s", "--scaletype [n]", "scaling type. "
    "0 = linear, 1 = quad, 2 = log", set_scale_type);
  command_parse(&cmd, argc, argv);

  if (cmd.argc > 0) a = atof(cmd.argv[0]);
  if (cmd.argc > 1) b = atof(cmd.argv[1]);

  c = -a;
  d = -b;

  printf("a: %1.3f, b: %1.3f\n", a, b);

  clock_t time = clock();
  de_jong();


  time = clock() - time;
  printf("Done iterating. Elapsed time: %dms\n",
    (int)((double)time / ((double)CLOCKS_PER_SEC / 1000.0f)));
  printf("Normalizing...\n");
  normalize_buffer();

  write_png();

  return 0;
}
