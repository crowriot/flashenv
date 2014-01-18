// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------


#ifndef DRAWHOOK_H
#define DRAWHOOK_H

/// number of pages to draw to
#define FB_PAGES 1

/// name of the hooking library
#define DRAWHOOK_LIBRARY_NAME "libpreflashenv"

/// function name to enable frame buffer target drawing
#define ENABLEFRAMEBUFFERDRAW_FUNCTION_NAME "EnableFramebufferDraw"

/// function prototype frame buffer target drawing
typedef void (*EnableFramebufferDrawFN)( int );

#endif

