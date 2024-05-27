#include <nds.h>
#include <filesystem.h>

#include <stdio.h>
#include "dave.h"

struct rectangle_s {
   int x, y, w, h;
};
typedef struct rectangle_s rectangle_t;

bool CheckCollisionRect(rectangle_t *rec1, rectangle_t *rec2) {
   return !((rec1->x+rec1->w < rec2->x) || (rec1->x > rec2->x+rec2->w) || (rec1->y+rec1->h < rec2->y) || (rec1->y > rec2->y+rec2->h));
}

//Assume the bg at bg_map is of size BgSize_T_256x256
void DrawValue(u16 *bg_map, int tile_x, int tile_y, unsigned int value) {
  char buf[16] = { 0 };
  snprintf(buf, 15, "%u", value);    //TODO : be independant of libc
  for(int i = 0; buf[i] != 0 && i < 16; ++i) {
    bg_map[tile_x+i + tile_y*32] = buf[i] - '0' + 1;
  }
}

int main(int argc, char **argv) {
  int p1_x = 10;
  int p1_y = 50;
  int p2_x = 216;
  int p2_y = 50;
  int b_x = 50;
  int b_y = 50;
  int b_vx = 1;
  int b_vy = 1;
  int s1 = 0;
  int s2 = 0;

  nitroFSInit(NULL);

  videoSetMode(MODE_5_2D);

  consoleDemoInit();

  oamInit(&oamMain, SpriteMapping_1D_32, false);
  oamEnable(&oamMain);

  //Init bg0 with font sheet
  int bg0 = bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0, 1);
  u16 *bg0_gfx = bgGetGfxPtr(bg0);
  u16 *bg0_map = bgGetMapPtr(bg0);
  DaveFile *digits_dave = dave_parse("gfx/digits.dave");
  dmaCopy(digits_dave->pixels, bg0_gfx, digits_dave->pixels_len);
  dmaCopy(digits_dave->palette, BG_PALETTE, digits_dave->palette_len);
  dave_free(digits_dave);

  //Init paddle and ball sprite
  u16 *paddle_sprite = oamAllocateGfx(&oamMain, SpriteSize_8x32, SpriteColorFormat_256Color);
  DaveFile *paddle_dave = dave_parse("gfx/paddle.dave");
  dmaCopy(paddle_dave->pixels, paddle_sprite, digits_dave->pixels_len);
  dmaCopy(paddle_dave->palette, SPRITE_PALETTE, paddle_dave->palette_len);
  dave_free(paddle_dave);

  SPRITE_PALETTE[15] = ARGB16(1, 0b11111, 0, 0);
  u16 *ball_sprite = oamAllocateGfx(&oamMain, SpriteSize_8x8, SpriteColorFormat_256Color);
  for (int i = 0; i < 8 * 8; ++i) ((u8 *)ball_sprite)[i] = 15;

  // player 1
  oamSet(&oamMain,                    // oam
         0,                           // id
         p1_x, p1_y,                  // position
         0,                           // priority
         0,                           // transparent color/palette index
         SpriteSize_8x32,             // size
         SpriteColorFormat_256Color,  // format
         paddle_sprite,               // gfx
         -1,                          // affine index
         true,                        // double drawing size
         false, false, false, false); // stuff

  // player 2
  oamSet(&oamMain,                    // oam
         1,                           // id
         p2_x, p2_y,                  // position
         0,                           // priority
         0,                           // transparent color/palette index
         SpriteSize_8x32,             // size
         SpriteColorFormat_256Color,  // format
         paddle_sprite,               // gfx
         -1,                          // affine index
         true,                        // double drawing size
         false, false, false, false); // stuff

  // ball
  oamSet(&oamMain,                    // oam
         2,                           // id
         p2_x, p2_y,                  // position
         0,                           // priority
         0,                           // transparent color/palette index
         SpriteSize_8x8,              // size
         SpriteColorFormat_256Color,  // format
         ball_sprite,                 // gfx
         -1,                          // affine index
         true,                        // double drawing size
         false, false, false, false); // stuff


  while (1) {
    DrawValue(bg0_map, 3, 3, s1);
    DrawValue(bg0_map, 25, 3, s2); 
    oamSetXY(&oamMain, 0, p1_x, p1_y);
    oamSetXY(&oamMain, 1, p2_x, p2_y);
    oamSetXY(&oamMain, 2, b_x, b_y);

    // Synchronize game loop to the screen refresh
    swiWaitForVBlank();
    oamUpdate(&oamMain);

    // Handle user input
    // -----------------

    scanKeys();

    uint16_t keys = keysHeld();
    uint16_t keys_down = keysDown();

    if (keys & KEY_START)
      break;

    p1_y += ((keys & KEY_DOWN) != 0) - ((keys & KEY_UP) != 0);
    p2_y += ((keys & KEY_B) != 0) - ((keys & KEY_X) != 0);

    b_x += b_vx;
    b_y += b_vy;
    if (b_y < 0 || b_y > 192 - 8) b_vy = -b_vy;

    if(b_x < -8) {
      ++s2;
      b_x = 50;
      b_y = 50;
    }
    else if(b_x > 256) {
      ++s1;
      b_x = 50;
      b_y = 50;
    }

    rectangle_t rec_p1 = {p1_x, p1_y, 8, 32};
    rectangle_t rec_p2 = {p2_x, p2_y, 8, 32};
    rectangle_t rec_b  = {b_x , b_y , 8,  8};

    if(CheckCollisionRect(&rec_p1, &rec_b)) {
      b_vx = 1;
    }
    if(CheckCollisionRect(&rec_p2, &rec_b)) {
      b_vx = -1;
    }

  }

  return 0;
}

