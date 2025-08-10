#include "misc.h"

// palette ---------------------------------------------------------------------
const COLOUR    videoColour[16] =
{
    {.r = 0x00, .g = 0x00, .b = 0x00},      // black
    {.r = 0x00, .g = 0x00, .b = 0xff},      // blue
    {.r = 0xff, .g = 0x00, .b = 0x00},      // red
    {.r = 0xff, .g = 0x00, .b = 0xff},      // magenta
    {.r = 0x00, .g = 0xff, .b = 0x00},      // green
    {.r = 0x00, .g = 0xaa, .b = 0xff},      // light blue
    {.r = 0xff, .g = 0xff, .b = 0x00},      // yellow
    {.r = 0xff, .g = 0xff, .b = 0xff},      // white
    {.r = 0x80, .g = 0x80, .b = 0x80},      // mid grey
    {.r = 0x00, .g = 0x55, .b = 0xff},      // mid blue
    {.r = 0xaa, .g = 0x00, .b = 0x00},      // mid red
    {.r = 0x55, .g = 0x00, .b = 0x00},      // dark red
    {.r = 0x00, .g = 0xaa, .b = 0x00},      // mid green
    {.r = 0x00, .g = 0x55, .b = 0x00},      // dark green
    {.r = 0xff, .g = 0x80, .b = 0x00},      // orange
    {.r = 0x80, .g = 0x40, .b = 0x00}       // brown
};

// timer -----------------------------------------------------------------------
void Timer_Set(TIMER *timer, int numerator, int divisor)
{
    timer->acc = 0;
    timer->rate = numerator / divisor;
    timer->remainder = numerator - timer->rate * divisor;
    timer->divisor = divisor;
}

int Timer_Update(TIMER *timer)
{
    timer->acc += timer->remainder;
    if (timer->acc < timer->divisor)
    {
        return timer->rate;
    }

    timer->acc -= timer->divisor;

    return timer->rate + 1;
}

// external --------------------------------------------------------------------
int Video_Viewport(int width, int height, int *x, int *y, int *w, int *h)
{
    int multiply;
    
    // Calculate scale factor to fit the game (256x192) into the window
    // while maintaining 4:3 aspect ratio
    int scale_x = width / WIDTH;
    int scale_y = height / HEIGHT;
    multiply = (scale_x < scale_y) ? scale_x : scale_y;
    
    if (multiply < 1) multiply = 1;
    
    // Calculate the actual display size
    *w = WIDTH * multiply;
    *h = HEIGHT * multiply;
    
    // Center the viewport in the window
    *x = (width - *w) / 2;
    *y = (height - *h) / 2;

    return multiply;
}
