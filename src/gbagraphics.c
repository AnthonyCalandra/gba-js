#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "gbacolor.h"
#include "gbagraphics.h"
#include "gbabios.h"
#include "gbaio.h"

#define CLAMP(n, a, b) (((n) < (a) ? (a) : (n)) > (b) ? (b) : ((n) < (a) ? (a) : (n)))

VideoBuffer _video_buffer = M4_PAGE1;

void initialize_graphics()
{
    REG_DISPCNT = DCNT_BG2 | DCNT_MODE4;
    // Initialize the palette with your basic RGB, black, and white colors.
    M4_PALETTE[COLOR_WHITE] = to_rgb16(255, 255, 255);
    M4_PALETTE[COLOR_BLACK] = to_rgb16(0, 0, 0);
    M4_PALETTE[COLOR_RED] = to_rgb16(255, 0, 0);
    M4_PALETTE[COLOR_GREEN] = to_rgb16(0, 255, 0);
    M4_PALETTE[COLOR_BLUE] = to_rgb16(0, 0, 255);
}

/**
 * Return the pixel at the given (x, y) coordinate.
 */
uint16_t* m4_get_pixel(int32_t x, int32_t y)
{
  // Divide by 2 since the buffer is storing 2 pixels per index.
  return (uint16_t*) &_video_buffer[(y * SCREEN_WIDTH + x) >> 1];
}

void flip_vram_page()
{
  if (REG_DISPCNT & DISPCNT_TOGGLE_PAGE)
  {
    // Page 2 currently enabled. Clear and switch to page 1.
    REG_DISPCNT &= ~DISPCNT_TOGGLE_PAGE;
    _video_buffer = (uint16_t*) M4_PAGE2;
    cpu_zero_memory(M4_PAGE2, M4_PAGE_SIZE_IN_WORDS);
  }
  else
  {
    // Flip to page 2.
    REG_DISPCNT |= DISPCNT_TOGGLE_PAGE;
    _video_buffer = (uint16_t*) M4_PAGE1;
    cpu_zero_memory(M4_PAGE1, M4_PAGE_SIZE_IN_WORDS);
  }
}

void m4_draw_pixel(int32_t x, int32_t y, uint32_t color_index)
{
  uint16_t *dst = m4_get_pixel(x, y);
  if (x & 1)
  {
    // Odd x-coordinate.
    *dst = (*dst & 0x00FF) | (color_index << 8);
  }
  else
  {
    // Even x-coordinate.
    *dst = (*dst & 0xFF00) | color_index;
  }
}

void m4_draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color_index)
{
  x0 = CLAMP(x0, 0, SCREEN_WIDTH - 1);
  x1 = CLAMP(x1, 0, SCREEN_WIDTH - 1);
  y0 = CLAMP(y0, 0, SCREEN_HEIGHT - 1);
  y1 = CLAMP(y1, 0, SCREEN_HEIGHT - 1);

  int32_t dx = abs(x1 - x0);
  int32_t dy = abs(y1 - y0);
  int32_t sx = (x0 < x1) ? 1 : -1;
  int32_t sy = (y0 < y1) ? 1 : -1;
  int32_t err = dx - dy;
  m4_draw_pixel(x0, y0, color_index);
  while (x0 != x1 || y0 != y1)
  {
    int32_t epsilon = err << 1;
    if (epsilon > -dy)
    {
      err -= dy;
      x0 += sx;
    }
    if (epsilon < dx)
    {
      err += dx;
      y0 += sy;
    }

    m4_draw_pixel(x0, y0, color_index);
  }
}

void m4_draw_circle_fill(int32_t x0, int32_t y0, uint32_t radius, uint32_t color_index)
{
  if (radius < 1)
  {
    return;
  }

  int32_t x = radius;
  int32_t y = 0;
  int32_t decision_over_2 = 1 - x;
  while (x >= y)
  {
    m4_draw_line(y + x0, x + y0, x0 - y, x + y0, color_index);
    m4_draw_line(x + x0, y + y0, x0 - x, y + y0, color_index);
    m4_draw_line(y + x0, y0 - x, x0 - y, y0 - x, color_index);
    m4_draw_line(x + x0, y0 - y, x0 - x, y0 - y, color_index);
    y++;
    if (decision_over_2 <= 0)
    {
      decision_over_2 += (y << 1) + 1;
    }
    else
    {
      x--;
      decision_over_2 += ((y - x) << 1) + 1;
    }
  }
}

void m4_draw_rect_fill_screen(uint32_t color_index)
{
    memset((void*) _video_buffer, color_index, M4_PAGE_SIZE_IN_WORDS * sizeof(void*));
}

void m4_draw_rect_fill(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color_index)
{
  int32_t dx = abs(x1 - x0);
  int32_t dy = abs(y1 - y0);
  int32_t x = (x0 < x1) ? x0 : x1;
  int32_t y = (y0 < y1) ? y0 : y1;
  for (dy++; dy > 0; dy--, y++)
  {
    m4_draw_line(x, y, x + dx, y, color_index);
  }
}

void m4_puts(int32_t x, int32_t y, text_writer* gptxt, const char* str, uint8_t color_index)
{
  unsigned char c;
  while ((c = *str++) != 0)
  {
    int32_t ix, iy;
    // Point to glyph; each row is one byte.
    uint8_t *pch = (uint8_t*) &gptxt->font[gptxt->char_map[c] << 1];
    for (iy = 0; iy < 8; iy++)
    {
      uint32_t row = pch[iy];
      // Plot pixels until row-byte is empty
      for (ix = x; row > 0; row >>= 1, ix++)
      {
        if (row & 1)
        {
          m4_draw_pixel(ix + x, iy + y, color_index);
        }
      }
    }

    x += gptxt->dx;
  }
}
