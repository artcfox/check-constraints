
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <png.h>
#include <math.h>

#include "pixel_node.h"

int decode_png(char* pngfile, uint8_t* buffer, size_t bufferlen, uint32_t* w, uint32_t* h)
{
  // decode PNG file
  FILE *fp = fopen(pngfile, "rb");
  if (!fp) {
    fprintf(stderr, "Error: Unable to open \"%s\"\n", pngfile);
    return -1;
  }

  unsigned char header[8]; // NUM_SIG_BYTES = 8
  fread(header, 1, sizeof(header), fp);
  if (png_sig_cmp(header, 0, sizeof(header))) {
    fprintf(stderr, "'%s' is not a png file\n", pngfile);
    fclose(fp);
    return -1;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                               (png_voidp)NULL, NULL, NULL);
  if (!png_ptr) {
    fprintf(stderr, "png_create_read_struct() failed\n");
    fclose(fp);
    return -1;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    fprintf(stderr, "png_create_info_struct() failed\n");
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    fclose(fp);
    return -1;
  }

  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info) {
    fprintf(stderr, "png_create_info_struct() failed\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fclose(fp);
    return -1;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "longjmp() called\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);
    return -1;
  }

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, sizeof(header));
  png_read_info(png_ptr, info_ptr);

  png_uint_32 width, height;
  int bit_depth, color_type, interlace_method, compression_method,
    filter_method;
  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
               &interlace_method, &compression_method, &filter_method);

  // Do any custom transformations necessary here
  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png_ptr);

  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png_ptr);

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png_ptr);

  if (bit_depth == 16)
    png_set_strip_16(png_ptr);

  png_color_16 black_bg = {0, 0, 0, 0, 0};
  png_set_background(png_ptr, &black_bg, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1);

  png_read_update_info(png_ptr, info_ptr);
  // End custom transformations

  png_uint_32 rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  png_uint_32 pixel_size = rowbytes / width;

  if (width > PNG_UINT_32_MAX / pixel_size)
    png_error(png_ptr, "Image is too wide to process in memory");

  png_bytep data = (png_bytep)png_malloc(png_ptr, height * rowbytes);
  png_bytepp row_pointers = (png_bytepp)png_malloc(png_ptr, height * sizeof(png_bytep));

  for (png_uint_32 i = 0; i < height; ++i)
    row_pointers[i] = &data[i * rowbytes];

  png_set_rows(png_ptr, info_ptr, row_pointers);
  png_read_image(png_ptr, row_pointers);

  // data now contains the byte data in the format RGB
  int retval = 0;

  if (bufferlen < height * rowbytes) {
    if (bufferlen != 0)
      fprintf(stderr, "The buffer passed to decode_png is not large enough for '%s'.\n", pngfile);
    *h = height;
    *w = width;
    retval = -1;
  } else {
    memcpy(buffer, data, height * rowbytes);
    *w = width;
    *h = height;
  }

  png_free(png_ptr, row_pointers);
  png_free(png_ptr, data);

  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  fclose(fp);

  return retval;
}

#define MAX_UNIQUE_PIXELS_PER_ROW (4)

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s filename.png\n", argv[0]);
    return -1;
  }

  // Figure out the size of the image
  uint32_t width;
  uint32_t height;
  (void)decode_png(argv[1], 0, 0, &width, &height);

  // Allocate enough memory to hold the decoded image
  size_t bufferlen = width * height * 3;
  uint8_t* buffer = malloc(bufferlen);

  // Decode the image
  if (decode_png(argv[1], buffer, bufferlen, &width, &height) == -1)
    return -1;

  // Create a red-black tree to use as a set for keeping track of the unique pixels per row
  pixel_node_t myNil;
  rbtree_node_t *myNilRef = (rbtree_node_t *)&myNil;
  rbtree_t tree;
  rbtree_init(&tree, myNilRef, sizeof(pixel_node_t), pixel_node_compare);

  // Scan through the decoded image one row at a time
  for (uint8_t h = 0; h < height; h++) {

    // Ensure the tree is empty
    for (rbtree_node_t *itr = rbtree_minimum(&tree); itr != myNilRef; itr = rbtree_minimum(&tree)) {
      rbtree_delete(&tree, itr);
      free(itr);
    }

    // Loop over all the pixels in a row, adding them to the set
    for (uint8_t w = 0; w < width; w++) {
      PIXEL_DATA pixel = { *(buffer + h * width * 3 + w * 3 + 0),
                           *(buffer + h * width * 3 + w * 3 + 1),
                           *(buffer + h * width * 3 + w * 3 + 2) };
      rbtree_node_t* p = pixel_node_new(&pixel);
      if (!rbtree_setinsert(&tree, p))
        free(p);
    }

#if defined(DEBUG_RBTREE)
    fprintf(stdout, "\n\n\n");
    rbtree_print_dot(&tree, stdout, pixel_node_print);
    fprintf(stdout, "\n\n\n");
#endif

    // Count the number of unique pixels for a given row (the size of the set)
    int unique_pixel_count = 0;
    for (rbtree_node_t *itr = rbtree_minimum(&tree); itr != myNilRef; itr = rbtree_successor(&tree, itr))
      unique_pixel_count++;

    printf("Row: %u, Unique Pixels: %d", h, unique_pixel_count);

    // If the count is greater than the max unique pixels per row, then print out the RGB values of the pixels in that row
    if (unique_pixel_count > MAX_UNIQUE_PIXELS_PER_ROW)
      for (rbtree_node_t *itr = rbtree_minimum(&tree); itr != myNilRef; itr = rbtree_successor(&tree, itr))
        printf(", (%u, %u, %u)", ((pixel_node_t*)itr)->pixel.r, ((pixel_node_t*)itr)->pixel.g, ((pixel_node_t*)itr)->pixel.b);

    printf("\n");

  }

  // Cleanup
  for (rbtree_node_t *itr = rbtree_minimum(&tree); itr != myNilRef; itr = rbtree_minimum(&tree)) {
    rbtree_delete(&tree, itr);
    free(itr);
  }
  rbtree_destroy(&tree);
  free(buffer);

  return 0;
}
