// Written by Naoki Shibata shibatch.sf.net@gmail.com 
// http://ito-lab.naist.jp/~n-sibata/

// This software is in public domain. You can use and modify this code
// for any purpose without any obligation.


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "baum.h"

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage : %s <data> [<radius>] [<X position>] [<Y position>]\n", argv[0]);
    fprintf(stderr, "This program draws a marker corresponding to data (a number between 0 and 500)\n");
    fprintf(stderr, "in SVG format, and write it to output.html.\n");
    fflush(stderr);
    exit(-1);
  }

  int data = 0;
  if (argc > 1) data = atoi(argv[1]);

  if (data < 0 || data > 500) {
    fprintf(stderr, "Data out of range.\nData must be an integer between 0 and 500.\n");
    exit(-1);
  }

  double radius = 350, posx = 50, posy = 50;
  if (argc > 2) radius = atof(argv[1]);
  if (argc > 3) posx = atof(argv[2]);
  if (argc > 4) posy = atof(argv[3]);

  FILE *fp = fopen("output.html", "w");
  if (fp == NULL) {
    fprintf(stderr, "Could not open output.html.\n");
    exit(-1);
  }

  fprintf(fp, "<!DOCTYPE html>\n");
  fprintf(fp, "<html>\n");
  fprintf(fp, "<body>\n");

  baum_fprintMarkerSVG(fp, data, radius, posx, posy);

  fprintf(fp, "</body>\n");
  fprintf(fp, "</html>\n");

  fclose(fp);

  exit(0);
}
