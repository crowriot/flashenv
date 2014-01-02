// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------


#ifndef KEYMAP_H
#define KEYMAP_H

#include <map>
#include <gdk/gdktypes.h>
#include <gdk/gdkkeysyms.h>

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

enum PandoraKeys
{
    DPAD_Left = GDK_Left,
    DPAD_Right = GDK_Right,
    DPAD_Up = GDK_Up,
    DPAD_Down = GDK_Down,

    DPAD_A = GDK_Home,
    DPAD_B = GDK_End,
    DPAD_X = GDK_Page_Down,
    DPAD_Y = GDK_Page_Up,

    Start = GDK_Alt_L,
    Select = GDK_Control_L,

    Trigger_Left = GDK_Shift_R,
    Trigger_Right = GDK_Control_R
};

typedef std::map<guint, SimpleKey> KeyMap;

/// returns true if the keyval is a pandora key
extern bool IsPandoraKey( guint keyval );

/// load keymap from given ini file and the given swffile
bool LoadKeyMap( const char* inifile, const char* swffile, KeyMap* key_map );


#endif

