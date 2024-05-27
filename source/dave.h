#ifndef DAVE_H
#define DAVE_H

#include <nds.h>
#include <stdint.h>

struct DaveFile_s {
  size_t width;
  size_t height;
  u8 bpp;
  
  size_t pixels_len;    //number of bytes occupied py pixels
  u8 *pixels;
  
  size_t palette_len;    //number of bytes occupied by palette
  u16 *palette;
};
typedef struct DaveFile_s DaveFile;

DaveFile *dave_parse(const char *filepath);
void dave_free(DaveFile *dave);


#endif
