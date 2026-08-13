/* Never use Silk functions that don't recive a buffer size. */
/* Hopefully we can catch all places where we make this mistake (by crashing). */
#define SILK_PIXELBUFFER_WIDTH 0x7fffffff
#define SILK_PIXELBUFFER_HEIGHT 0x7fffffff
#define SILK_IMPLEMENTATION
#include "./silk/silk.h"
