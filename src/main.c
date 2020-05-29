#include <stdbool.h>
#include <stdlib.h>

#include "gba.h"
#include "gbaio.h"
#include "jsapi.h"

static void bsod(const char* msg)
{
    m4_draw_rect_fill_screen(4);
    const char* title = "FATAL ERROR";
    const size_t num_letters = 11 / 2;
    // Center the title.
    set_text_dx(DEFAULT_TEXT_DX + 1);
    gba_printf(60 - num_letters * (DEFAULT_TEXT_DX + 1), 10, 0, title);

    set_text_dx(DEFAULT_TEXT_DX);
    gba_printf(10, 30, 0, msg);
}

int main()
{
    srand(RAND_SEED);
    register_vblank_isr();
    initialize_text_writer();
    initialize_graphics();

    bool init_ok = initialize_jerry();
    if (!init_ok)
    {
        bsod("Parsing or initialization error.");
    }

    while (init_ok)
    {
        vblank_intr_wait();
        flip_vram_page();
        poll_key();
        call_ontick_handler();
    }

    cleanup_jerry();
    return 0;
}
