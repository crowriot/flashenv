// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------

#include "keymap.h"
#include <iostream>
#include <gdk/gdkkeys.h>

extern "C" {
#include "iniparser-3.0/src/iniparser.h"
}

using namespace std;

bool IsPandoraKey(guint keyval)
{
    return keyval==DPAD_Left
         || keyval==DPAD_Right
         || keyval==DPAD_Up
         || keyval==DPAD_Down

         || keyval==DPAD_A
         || keyval==DPAD_B
         || keyval==DPAD_X
         || keyval==DPAD_Y

         || keyval==Start
         || keyval==Select

         || keyval==Trigger_Left
         || keyval==Trigger_Right
    ;
}


bool LoadKeyMap(const char* inifile, const char* swffile, KeyMap* key_map)
{
    cout << __FUNCTION__ << endl;

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

    int keyvalues[] = {DPAD_Left,DPAD_Right,DPAD_Up,DPAD_Down,DPAD_A,DPAD_B,DPAD_X,DPAD_Y,Start,Select,Trigger_Left,Trigger_Right};
    char* keynames[] = {"DPAD_Left","DPAD_Right","DPAD_Up","DPAD_Down","DPAD_A","DPAD_B","DPAD_X","DPAD_Y","Start","Select","Trigger_Left","Trigger_Right"};
    int nkeys = sizeof(keyvalues)/sizeof(*keyvalues);


    char tmp[256];
    for (int k=0; k<nkeys; ++k)
    {
        sprintf(tmp,"%s:%s",section,keynames[k]);

        char* mapped = iniparser_getstring(d,tmp,"");
        guint keyval;
        if(mapped[0] && (keyval=gdk_keyval_from_name(mapped))!=GDK_VoidSymbol)
        {
            cout << "\tMapped " << keynames[k] << " to " << mapped << " (0x" << hex << keyval << ")" << endl;

            (*key_map)[keyvalues[k]] = keyval;
        }
        else
        {
            cout << "\t" << keynames[k] << " not mapped " << endl;
        }
    }

    iniparser_freedict(d);

    return true;
}

