// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------


#ifndef KEYMAP_H
#define KEYMAP_H

#include <map>
#include <vector>
#include <gdk/gdktypes.h>
#include <gdk/gdkkeysyms.h>
#include "../preload/inputhook.h"


#define KEY_NAME_SEPARATOR " "

/// simple key class for Gdk remapping
class SimpleKey
{
public:
    SimpleKey()
        : keyval(0)
        , modifier((GdkModifierType)0)
    {

    }

    SimpleKey(int keyval)
        : keyval(keyval)
        , modifier((GdkModifierType)0)
    {

    }

    int keyval;
    GdkModifierType modifier;
};

/// gdk key value enumeration
enum GdkPandoraKeys
{
    GdkPnd_DPAD_Left = GDK_Left,
    GdkPnd_DPAD_Right = GDK_Right,
    GdkPnd_DPAD_Up = GDK_Up,
    GdkPnd_DPAD_Down = GDK_Down,

    GdkPnd_DPAD_A = GDK_Home,
    GdkPnd_DPAD_B = GDK_End,
    GdkPnd_DPAD_X = GDK_Page_Down,
    GdkPnd_DPAD_Y = GDK_Page_Up,

    GdkPnd_Start = GDK_Alt_L,
    GdkPnd_Select = GDK_Control_L,

    GdkPnd_Trigger_Left = GDK_Shift_R,
    GdkPnd_Trigger_Right = GDK_Control_R
};

/// raw keycode enumeration
enum RawPandoraKeys
{
    KeyCodePnd_DPAD_Left = 113,
    KeyCodePnd_DPAD_Right = 114,
    KeyCodePnd_DPAD_Up = 111,
    KeyCodePnd_DPAD_Down = 116,

    KeyCodePnd_DPAD_A = 110,
    KeyCodePnd_DPAD_B = 115,
    KeyCodePnd_DPAD_X = 117,
    KeyCodePnd_DPAD_Y = 112,

    KeyCodePnd_Start = 64,
    KeyCodePnd_Select = 37,

    KeyCodePnd_Trigger_Left = 62,
    KeyCodePnd_Trigger_Right = 105,
};

/// map from keyval to gdk key
typedef std::map<guint, std::vector<SimpleKey> > KeyMapGdk;

/// returns true if the keyval is a pandora key
extern bool IsPandoraKey( guint keyval );

/// load keymaps from given ini file and the given swffile
bool LoadKeyMap( const char* inifile, const char* swffile, KeyMapGdk* key_map, KeyMapX11* key_map_x11 );

/// set global x11 keymap
void SetKeyMapX11( const KeyMapX11 );


#endif

