#ifndef GBAIO_H
#define GBAIO_H 1

#include <stdint.h>

#define REG_KEYINPUT  (* (volatile uint32_t*) 0x4000130)

/**
 * Key codes.
 */
#define KEY_A 0x1
#define KEY_B 0x2
#define KEY_SELECT  0x4
#define KEY_START 0x8
#define KEY_RIGHT 0x10
#define KEY_LEFT  0x20
#define KEY_UP  0x40
#define KEY_DOWN  0x80
#define KEY_R 0x100
#define KEY_L 0x200

/**
 * Maintain state of the current and previous keys in order to implement
 * key states (is up, is down, held, etc.).
 */
extern uint32_t _current_key;
extern uint32_t _previous_key;

/**
 * Poll user input synchronously.
 */
static inline void poll_key()
{
  _previous_key = _current_key;
  // The bits in the REG_KEYINPUT register is set to high when a key is up and
  // low when a key is down.
  _current_key = ~(REG_KEYINPUT) & 0x03FF;
}

static inline uint32_t current_key_state()
{
  return _current_key;
}

static inline uint32_t previous_key_state()
{
  return _previous_key;
}

static inline uint32_t is_key_down(uint32_t key)
{
  return _current_key & key;
}

/**
 * Credit to TONC for the font system.
 */
typedef struct
{
  volatile uint16_t* dst;
  uint32_t* font;
  uint8_t* char_map;
  uint8_t* char_width;
  int8_t dx, dy;
  uint16_t flags;
} text_writer;

extern uint8_t txt_lut[256];
extern text_writer gptxt;

void initialize_text_writer();
void set_text_dx(int8_t dx);

/**
 * Similar to printf in the C Standard Library, with the exception that it prints
 * to the game canvas.
 * @param x The x coordinate to begin drawing.
 * @param y The y coordinate to begin drawing.
 * @param color_index The color located at the index in the palette table to use.
 * @param format The string format; same as the Standard Library's printf format.
 * @param ... Arguments to the format.
 */
void gba_printf(int32_t x, int32_t y, uint8_t color_index, const char* format, ...);

#endif
