#pragma once
#include "drawing.h"


typedef struct {

  uint16_t x;
  uint16_t y;
  uint16_t speed;
  xpm_image_t img;
  

} Sprite;

Sprite * create_sprite (xpm_image_t sprite_img, uint16_t x, uint16_t y, uint16_t speed);



