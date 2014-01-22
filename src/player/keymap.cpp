// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------

#include "keymap.h"
#include "defines.h"
#include <iostream>
#include <gdk/gdkkeys.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <linux/input.h>
#include <dlfcn.h>

extern "C" {
#include "../third_party/iniparser-3.0/src/iniparser.h"
}

using namespace std;

const int C_PandoraKeys_GdkValues[PandoraKeyCount] = {
    GdkPnd_DPAD_Left,
    GdkPnd_DPAD_Right,
    GdkPnd_DPAD_Up,
    GdkPnd_DPAD_Down,
    GdkPnd_DPAD_A,
    GdkPnd_DPAD_B,
    GdkPnd_DPAD_X,
    GdkPnd_DPAD_Y,
    GdkPnd_Start,
    GdkPnd_Select,
    GdkPnd_Trigger_Left,
    GdkPnd_Trigger_Right
};

const int C_PandoraKeys_RawValues[PandoraKeyCount] = {
    RawPnd_DPAD_Left,
    RawPnd_DPAD_Right,
    RawPnd_DPAD_Up,
    RawPnd_DPAD_Down,
    RawPnd_DPAD_A,
    RawPnd_DPAD_B,
    RawPnd_DPAD_X,
    RawPnd_DPAD_Y,
    RawPnd_Start,
    RawPnd_Select,
    RawPnd_Trigger_Left,
    RawPnd_Trigger_Right
};

const char* C_PandoraKeys_IniNames[PandoraKeyCount] = {
    "DPAD_Left",
    "DPAD_Right",
    "DPAD_Up",
    "DPAD_Down",
    "DPAD_A",
    "DPAD_B",
    "DPAD_X",
    "DPAD_Y",
    "Start",
    "Select",
    "Trigger_Left",
    "Trigger_Right"
};


const char* C_PandoraKeys_DisplayNames[PandoraKeyCount] = {
    "DPAD Left",
    "DPAD Right",
    "DPAD Up",
    "DPAD Down",
    "DPAD A",
    "DPAD B",
    "DPAD X",
    "DPAD Y",
    "Start",
    "Select",
    "Trigger Left",
    "Trigger Right"
};

bool IsPandoraKey(guint keyval)
{
    return keyval==GdkPnd_DPAD_Left
         || keyval==GdkPnd_DPAD_Right
         || keyval==GdkPnd_DPAD_Up
         || keyval==GdkPnd_DPAD_Down

         || keyval==GdkPnd_DPAD_A
         || keyval==GdkPnd_DPAD_B
         || keyval==GdkPnd_DPAD_X
         || keyval==GdkPnd_DPAD_Y

         || keyval==GdkPnd_Start
         || keyval==GdkPnd_Select

         || keyval==GdkPnd_Trigger_Left
         || keyval==GdkPnd_Trigger_Right
    ;
}


bool LoadKeyMap(const char* inifile, const char* swffile, KeyMapGdk* key_map_gdk, KeyMapX11* key_map_x11)
{
    DEBUG_FUNCTION_NAME

    const char* section = strrchr(swffile,'/');
    if (!section)
        section = swffile;
    else
        section ++;

    cout << "\tSection=" << section << endl;

    dictionary* d = iniparser_load(const_cast<char*>(inifile));
    if (d==0)
    {
        cerr << "Failed to load " << inifile << endl;
        return false;
    }

    const int* gdkkeyvalues = C_PandoraKeys_GdkValues;
    const int* rawkeyvalues = C_PandoraKeys_RawValues;
    const char** keynames = C_PandoraKeys_IniNames;
    int nkeys = PandoraKeyCount;

    Display* display = XOpenDisplay(0);
    bool any_key_mapped = false;

    char tmp[256];
    for (int k=0; k<nkeys; ++k)
    {
        sprintf(tmp,"%s:%s",section,keynames[k]);

        char* mapped = iniparser_getstring(d,tmp,"");
        guint keyval;
        if(mapped[0])
        {
            char* keyname = strtok(mapped,KEY_NAME_SEPARATOR);

            if (!keyname)
                keyname = mapped;

            do
            {
                if ((keyval=gdk_keyval_from_name(keyname))!=GDK_VoidSymbol)
                {
                    any_key_mapped = true;

                    cout << "\tGDK mapped " << keynames[k] << " to " << keyname << " (" << keyval << ")" << endl;

                    (*key_map_gdk)[gdkkeyvalues[k]].push_back(keyval);
                }

                KeyCode kc_to = XKeysymToKeycode(display,XStringToKeysym(keyname));
                if (kc_to!=0)
                {
                    any_key_mapped = true;

                    int kc_from  = rawkeyvalues[k];

                    cout << "\tX11 mapped " << keynames[k] << "/" << (int)kc_from << " to " << (int)kc_to << endl;

                    int ito = 0;
                    while ((*key_map_x11)[kc_from][ito]!=0 && ito<KEYMAPX11_TARGET_SIZE)
                    {
                        ++ito;
                    }

                    if (ito<KEYMAPX11_TARGET_SIZE)
                    {
                        (*key_map_x11)[kc_from][ito] = kc_to;
                    }
                }

            } while ((keyname=strtok(NULL,KEY_NAME_SEPARATOR)));
        }
        else
        {
            cout << "\t" << keynames[k] << " not mapped " << endl;
        }
    }

    iniparser_freedict(d);

    XCloseDisplay(display);

    return any_key_mapped;
}

void SetKeyMapX11(const KeyMapX11 keymap)
{
    RegisterKeyMappingFN reg =
        (RegisterKeyMappingFN)dlsym(dlopen(INPUTHOOK_LIBRARY_NAME,RTLD_LAZY),REGISTERKEYMAPPING_FUNCTION_NAME);

    if (reg)
    {
        (*reg)(keymap,C_KeyMapX11_Size);
    }
    else
    {
        cerr << "! " REGISTERKEYMAPPING_FUNCTION_NAME " not found" << endl;
    }
}

