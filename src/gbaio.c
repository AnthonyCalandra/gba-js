#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gbagraphics.h"
#include "toncfont.h"
#include "gbaio.h"
#include "config.h"

uint32_t _current_key = 0;
uint32_t _previous_key = 0;
uint8_t txt_lut[256];
text_writer gptxt;

void initialize_text_writer()
{
  memset(&gptxt, 0, sizeof(text_writer));
  gptxt.dx = DEFAULT_TEXT_DX;
  gptxt.dy = DEFAULT_TEXT_DY;
  gptxt.dst = _video_buffer;
  gptxt.font = (uint32_t*) tonc_font_tiles;
  gptxt.char_map = txt_lut;
  gptxt.char_width = NULL;
  for (int32_t ii = 0; ii < 96; ii++)
  {
    gptxt.char_map[ii + 32] = ii;
  }
}

void set_text_dx(int8_t dx)
{
    gptxt.dx = dx;
}

void gba_printf(int32_t x, int32_t y, uint8_t color_index, const char* format, ...)
{
  va_list arguments;
  char buffer[512];
  va_start(arguments, format);
  vsnprintf(buffer, sizeof(buffer), format, arguments);
  m4_puts(x, y, &gptxt, buffer, color_index);
  va_end(arguments);
}
