// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#ifndef KEYBOARDHOOK_H
#define KEYBOARDHOOK_H

#include <X11/Xlib.h>

#define KEYMAPX11_SOURCE_SIZE 256
#define KEYMAPX11_TARGET_SIZE 32

/// keycode mapping
typedef KeyCode KeyMapX11[KEYMAPX11_SOURCE_SIZE][KEYMAPX11_TARGET_SIZE];

/// size of keymap table
const int C_KeyMapX11_Size = sizeof(KeyCode) * KEYMAPX11_SOURCE_SIZE * KEYMAPX11_TARGET_SIZE;

/// function prototype for keymap registering
typedef void (*RegisterKeyMappingFN)( const KeyMapX11 keymap, int size );


#endif

