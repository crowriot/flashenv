// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include "keyboardhook.h"

using namespace std;

// key map from flashenv
static KeyMapX11 S_KeyMap;

#define KEYMAP_SIZE 32

typedef int (*XQUERYKEYMAPFN)(Display*, char [KEYMAP_SIZE]);


int XQueryKeymap( Display* display, char keys_return[KEYMAP_SIZE] )
{
    static XQUERYKEYMAPFN x11_XQueryKeymap = 0;
    if (!x11_XQueryKeymap) {
        x11_XQueryKeymap = (XQUERYKEYMAPFN)dlsym(RTLD_NEXT, "XQueryKeymap");
    }

  // printf("XQueryKeymap called\n");

    int rval = x11_XQueryKeymap(display, keys_return);

    for (int from=0; from<256; ++from)
    {
        KeyCode to = S_KeyMap[from];

        int from_idx = from/8;
        int from_bit = from&7;

        if (to!=0)
        {
            int to_idx = to/8;
            int to_bit = to&7;

            if (keys_return[from_idx] & (1<<from_bit))
            {
                //printf("mapping detected %d -> %d\n", from, to);

                keys_return[to_idx] |= (1<<to_bit);
            }
        }
        //if (keys_return[from_idx] & (1<<from_bit))
        //{
        //    printf("Key pressed %d\n",from);
        //}
    }

    return rval;
}

extern "C" void RegisterKeyMapping(const KeyMapX11 key_map)
{
    memcpy(S_KeyMap, key_map, sizeof(KeyMapX11));
}


