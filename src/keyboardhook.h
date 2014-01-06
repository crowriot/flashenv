// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#ifndef KEYBOARDHOOK_H
#define KEYBOARDHOOK_H

#include <X11/Xlib.h>

///
typedef KeyCode KeyMapX11[256];

/// function prototype for keymap registering
typedef void (*RegisterKeyMappingFN)( const KeyMapX11 keymap );


#endif

