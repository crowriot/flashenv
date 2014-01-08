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

    int gdkkeyvalues[] = {GdkPnd_DPAD_Left,GdkPnd_DPAD_Right,GdkPnd_DPAD_Up,GdkPnd_DPAD_Down,GdkPnd_DPAD_A,GdkPnd_DPAD_B,GdkPnd_DPAD_X,GdkPnd_DPAD_Y,GdkPnd_Start,GdkPnd_Select,GdkPnd_Trigger_Left,GdkPnd_Trigger_Right};
    int rawkeyvalues[] = {KeyCodePnd_DPAD_Left,KeyCodePnd_DPAD_Right,KeyCodePnd_DPAD_Up,KeyCodePnd_DPAD_Down,KeyCodePnd_DPAD_A,KeyCodePnd_DPAD_B,KeyCodePnd_DPAD_X,KeyCodePnd_DPAD_Y,KeyCodePnd_Start,KeyCodePnd_Select,KeyCodePnd_Trigger_Left,KeyCodePnd_Trigger_Right};
    const char* keynames[] = {"DPAD_Left","DPAD_Right","DPAD_Up","DPAD_Down","DPAD_A","DPAD_B","DPAD_X","DPAD_Y","Start","Select","Trigger_Left","Trigger_Right"};
    int nkeys = sizeof(keynames)/sizeof(*keynames);

    Display* display = XOpenDisplay(0);


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
                    cout << "\tGDK mapped " << keynames[k] << " to " << keyname << " (" << keyval << ")" << endl;

                    (*key_map_gdk)[gdkkeyvalues[k]].push_back(keyval);
                }

                KeyCode kc_to = XKeysymToKeycode(display,XStringToKeysym(keyname));
                if (kc_to!=0)
                {
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

    return true;
}

void SetKeyMapX11(const KeyMapX11 keymap)
{
    RegisterKeyMappingFN reg =
        (RegisterKeyMappingFN)dlsym(dlopen(LIBRARY_NAME,RTLD_LAZY),REGISTERKEYMAPPING_FUNCTION_NAME);

    if (reg)
    {
        (*reg)(keymap,C_KeyMapX11_Size);
    }
    else
    {
        cerr << "! " REGISTERKEYMAPPING_FUNCTION_NAME " not found" << endl;
    }
}

