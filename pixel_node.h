#pragma once

#include <string.h>

#include "rbtree/rbtree.h"
#include "rbtree/rbtree+setinsert.h"
#include "rbtree/rbtree+debug.h"

struct PIXEL_DATA;
typedef struct PIXEL_DATA PIXEL_DATA;
struct PIXEL_DATA {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} __attribute__ ((packed));

typedef struct _pixel_node_t { rbtree_node_t super;
  struct PIXEL_DATA pixel;
} pixel_node_t;

rbtree_node_t *pixel_node_new(struct PIXEL_DATA *pixel)
{
  pixel_node_t *self = malloc(sizeof(pixel_node_t));
  memcpy(&self->pixel, pixel, sizeof(self->pixel));
  return (rbtree_node_t *)self;
}

int pixel_node_compare(const rbtree_node_t *x, const rbtree_node_t *y)
{
  uint8_t Rx = ((const pixel_node_t *)x)->pixel.r;
  uint8_t Ry = ((const pixel_node_t *)y)->pixel.r;
  uint8_t Gx = ((const pixel_node_t *)x)->pixel.g;
  uint8_t Gy = ((const pixel_node_t *)y)->pixel.g;
  uint8_t Bx = ((const pixel_node_t *)x)->pixel.b;
  uint8_t By = ((const pixel_node_t *)y)->pixel.b;

  uint32_t Cx = ((uint32_t)Rx << 16) + ((uint32_t)Gx << 8) + Bx;
  uint32_t Cy = ((uint32_t)Ry << 16) + ((uint32_t)Gy << 8) + By;

  if (Cx == Cy)
    return 0;
  else if (Cx < Cy)
    return -1;
  else
    return 1;
}

// The printing function needs a delimiter character around the label
// This can be escaped double quotes, or < and > to use supported HTML
// elements (like <table>).
void pixel_node_print(FILE *stream, const rbtree_node_t *self)
{
  uint8_t Rx = ((const pixel_node_t *)self)->pixel.r;
  uint8_t Gx = ((const pixel_node_t *)self)->pixel.g;
  uint8_t Bx = ((const pixel_node_t *)self)->pixel.b;
  fprintf(stream, "<<b>%u,%u,%u</b>>", Rx, Gx, Bx);
}
