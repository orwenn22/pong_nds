#include "dave.h"

#include <stdio.h>

static inline u16 read_u16(FILE *f) {
  u16 high = fgetc(f);
  u16 low = fgetc(f);
  return ((high&0xff) << 8) | (low&0xff);
}

DaveFile *dave_parse(const char *filepath) {
  FILE *in_file = fopen(filepath, "r");
  if(in_file == NULL) {
  printf("dave_parse : failed to open file\n");
    return NULL;
  }

  //Check for signature
  char sig[5] = { 0 };
  fgets(sig, 5, in_file);
  if(strcmp(sig, "Dave") != 0) {
    fclose(in_file);
    printf("dave_parse : wrong signature\n");
    return NULL;
  }

  //Allocate structure
  DaveFile *dave_file = (DaveFile *) malloc(sizeof(DaveFile));
  dave_file->pixels = NULL;
  dave_file->palette = NULL;

  //Get bpp and size
  dave_file->bpp = (u8) getc(in_file);
  dave_file->width = read_u16(in_file);
  dave_file->height = read_u16(in_file);
  //printf("%ibpp %i x %i\n",dave_file->bpp, dave_file->width, dave_file->height);

  //Sprite size must be a multiple of 8
  if(dave_file->width%8 != 0 || dave_file->height%8 != 0) {
    free(dave_file);
    fclose(in_file);
    printf("dave_parse : wrong size\n");
    return NULL;
  }
  
  switch(dave_file->bpp) {
    case 8: {
      //Allocate gfx
      dave_file->pixels_len = sizeof(u8) * dave_file->width * dave_file->height;
      dave_file->pixels = (u8 *) malloc(dave_file->pixels_len);
      //printf("pixels len %i\n", dave_file->pixels_len); 

      //Load gfx
      size_t i_stop = dave_file->width * dave_file->height;
      for(size_t i = 0; i < i_stop; ++i) dave_file->pixels[i] = (u8) fgetc(in_file);

      //Allocate palette
      //printf("ftell %x\n", ftell(in_file));
      dave_file->palette_len = ((u8) fgetc(in_file)) * sizeof(u16);    //this is the size in byte, not the number of colors
      //printf("Palette len %i\n", dave_file->palette_len);
      dave_file->palette = (u16 *) malloc(dave_file->palette_len);

      //Load palette
      i_stop = dave_file->palette_len / sizeof(u16);
      for(size_t i = 0; i < i_stop; ++i) dave_file->palette[i] = read_u16(in_file);

      break;
    }

    default:
      free(dave_file);
      dave_file = NULL;
      break;
  }

  //Check end signature
  fgets(sig, 5, in_file);
  if(strcmp(sig, "evaD") != 0) {
    fclose(in_file);
    printf("dave_parse : wrong end signature\n");
    printf("%x %x %x %x\n", sig[0], sig[1], sig[2], sig[3]);
    return NULL;
  }

  fclose(in_file);
  return dave_file;
}


void dave_free(DaveFile *dave) {
  if(dave == NULL) return;

  if(dave->pixels != NULL) {
    free(dave->pixels);
    dave->pixels = NULL;
  }

  if(dave->palette != NULL) {
    free(dave->palette);
    dave->palette = NULL;
  }

  free(dave);
}

