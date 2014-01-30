// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include "inputhook.h"


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
    int from;

    for (from=0; from<KEYMAPX11_SOURCE_SIZE; ++from)
    {
        int from_idx = from/8;
        int from_bit = from&7;

        if (keys_return[from_idx] & (1<<from_bit))
        {
            int ito;
            for (ito=0; ito<KEYMAPX11_TARGET_SIZE; ++ito)
            {
                KeyCode to = S_KeyMap[from][ito];
                if (!to)
                    break;

                int to_idx = to/8;
                int to_bit = to&7;

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


void RegisterKeyMapping(const KeyMapX11 key_map, int size)
{
    if (size==C_KeyMapX11_Size)
        memcpy(S_KeyMap, key_map, C_KeyMapX11_Size);
    else
        fprintf(stderr,"RegisterKeyMapping failed: sizes are not the same.\n");
}


// ---

static PointerChange S_PointerChange = {0,1,1, 0,1,1};

void SetPointerChange(PointerChange change)
{
    S_PointerChange = change;

    printf("hooked SetPointerChange: %d %d %d, %d %d %d\n",
            S_PointerChange.off_x, S_PointerChange.div_x, S_PointerChange.mul_x,
            S_PointerChange.off_y, S_PointerChange.div_y, S_PointerChange.mul_y);
}

/// XQueryPointer function pointer declaration
typedef int (*XQUERYPOINTERFN)(
      Display *display,
      Window w, Window *root_return, Window *child_return,
      int *root_x_return, int *root_y_return,
      int *win_x_return, int *win_y_return,
      unsigned int *mask_return);

static XQUERYPOINTERFN x11_XQueryPointer = 0;

int XQueryPointer(Display *display,
                Window w, Window *root_return, Window *child_return,
                int *root_x_return, int *root_y_return,
                int *win_x_return, int *win_y_return,
                unsigned int *mask_return)
{
    if (x11_XQueryPointer==0) {
        x11_XQueryPointer = (XQUERYPOINTERFN)dlsym(RTLD_NEXT,"XQueryPointer");
    }

    int r = (*x11_XQueryPointer)(display, w, root_return, child_return, root_x_return, root_y_return, win_x_return, win_y_return, mask_return);

    if (root_x_return)
    {
        *root_x_return -= S_PointerChange.off_x;
        *root_x_return *= S_PointerChange.mul_x;
        *root_x_return /= S_PointerChange.div_x;
    }
    if (root_y_return)
    {
        *root_y_return -= S_PointerChange.off_y;
        *root_y_return *= S_PointerChange.mul_y;
        *root_y_return /= S_PointerChange.div_y;
    }
    if (win_x_return)
    {
        *win_x_return -= S_PointerChange.off_x;
        *win_x_return *= S_PointerChange.mul_x;
        *win_x_return /= S_PointerChange.div_x;
    }
    if (win_y_return)
    {
        *win_y_return -= S_PointerChange.off_y;
        *win_y_return *= S_PointerChange.mul_y;
        *win_y_return /= S_PointerChange.div_y;;
    }

#ifdef _DEBUG
    if (root_x_return && root_y_return && win_x_return && win_y_return)
    {
        printf("hooked XQueryPointer: %p %p %d %d %d %d\n", display, w, *root_x_return, *root_y_return, *win_x_return, *win_y_return);
    }
#endif

    return r;
}
