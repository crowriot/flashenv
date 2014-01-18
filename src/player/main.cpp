// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------


#include "flashplayer.h"
#include "flashwindow.h"
#include "flashattributes.h"
#include "defines.h"
#include <string.h>
#include <iostream>

#ifndef GSEAL
#define GSEAL(x) x
#endif
#include <gtk/gtk.h>

/* -------- */

using namespace std;

/* -------- */

int main( int argc, char* argv[] )
{
    gtk_init(&argc,&argv);

    FlashWindow flash_window;

    if (argc<2)
    {
        cerr << "Usage: flashenv [arguments] <path/to/swf>" << endl;
        return 0;
    }

    const char* swffile = argv[argc-1];


/// load keymappings
    KeyMapGdk keymapgdk;
    KeyMapX11 keymapx11; memset(keymapx11,0,C_KeyMapX11_Size);
    if (LoadKeyMap(GAMECONFIG_INI_FILE,swffile,&keymapgdk,&keymapx11))
    {
        flash_window.SetKeyMap(keymapgdk);
        SetKeyMapX11(keymapx11);
    }

/// load attributes
    FlashAttributes flash_attributes;
    LoadAttributes(GAMECONFIG_INI_FILE,swffile,&flash_attributes);


/// initialize player
    FlashPlayer flash_player(flash_window, flash_attributes);

/// load flash plugin
    if (!flash_player.LoadLibrary())
    {
        return 1;
    }

/// do basic plugin initialization
    if (!flash_player.InitPlugin())
    {
        return 1;
    }

/// load the wanted file
    if (!flash_player.LoadFile(swffile))
    {
        return 1;
    }

    flash_player.Run();

    flash_player.Shutdown();

    return 0;
}
