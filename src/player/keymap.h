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

/* -------- */

/// gdk key value enumeration
enum PandoraKeysGdk
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

/* -------- */

/// raw keycode enumeration
enum PandoraKeysRaw
{
    RawPnd_DPAD_Left = 113,
    RawPnd_DPAD_Right = 114,
    RawPnd_DPAD_Up = 111,
    RawPnd_DPAD_Down = 116,

    RawPnd_DPAD_A = 110,
    RawPnd_DPAD_B = 115,
    RawPnd_DPAD_X = 117,
    RawPnd_DPAD_Y = 112,

    RawPnd_Start = 64,
    RawPnd_Select = 37,

    RawPnd_Trigger_Left = 62,
    RawPnd_Trigger_Right = 105,
};


enum
{
    PandoraKeyCount = 12
};

extern const int   C_PandoraKeys_RawValues[PandoraKeyCount];
extern const char* C_PandoraKeys_IniNames[PandoraKeyCount];
extern const char* C_PandoraKeys_DisplayNames[PandoraKeyCount];
extern const int C_PandoraKeys_GdkValues[PandoraKeyCount];

/* -------- */


/// map from keyval to gdk key
typedef std::map<guint, std::vector<SimpleKey> > KeyMapGdk;

/// returns true if the keyval is a pandora key
bool IsPandoraKey( guint keyval );

/// load keymaps from given ini file and the given swffile
bool LoadKeyMap( const char* inifile, const char* swffile, KeyMapGdk* key_map, KeyMapX11* key_map_x11 );

/// set global x11 keymap
void SetKeyMapX11( const KeyMapX11 );


#endif

