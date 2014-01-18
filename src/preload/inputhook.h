// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#ifndef INPUTHOOK_H
#define INPUTHOOK_H

#include <X11/Xlib.h>

/// 256 source keys
#define KEYMAPX11_SOURCE_SIZE 256

/// 32 target keys
#define KEYMAPX11_TARGET_SIZE 32

/// name of the hooking library
#define INPUTHOOK_LIBRARY_NAME "libpreflashenv"

/// function name to register keymapping
#define REGISTERKEYMAPPING_FUNCTION_NAME "RegisterKeyMapping"

/// keycode mapping
typedef KeyCode KeyMapX11[KEYMAPX11_SOURCE_SIZE][KEYMAPX11_TARGET_SIZE];

/// size of keymap table
const int C_KeyMapX11_Size = sizeof(KeyCode) * KEYMAPX11_SOURCE_SIZE * KEYMAPX11_TARGET_SIZE;

/// function prototype for keymap registering
typedef void (*RegisterKeyMappingFN)( const KeyMapX11 keymap, int size );



/// PointerChange structure
/// XQueryPointer values get modified by this.
struct _PointerChange
{
    int off_x;
    int div_x;
    int mul_x;

    int off_y;
    int div_y;
    int mul_y;
};

typedef struct _PointerChange PointerChange;

#define SETPOINTERCHANGE_FUNCTION_NAME "SetPointerChange"

/// funtion prototype for pointer change
typedef void (*SetPointerChangeFN)(PointerChange change);

#endif

