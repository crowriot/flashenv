// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------


#include "flashplayer.h"
#include "flashwindow.h"
#include "flashattributes.h"
#include "defines.h"
#include <string.h>
#include <iostream>

#define GSEAL(x) x
#include <gtk/gtk.h>

/* -------- */

using namespace std;

/* -------- */

int main( int argc, char* argv[] )
{
    gtk_init(&argc,&argv);

    FlashWindow flash_window;
    FlashPlayer flash_player(flash_window);

    if (argc<2)
    {
        cerr << "Usage: flashenv [arguments] <path/to/swf>" << endl;
        return 0;
    }


    KeyMapGdk keymapgdk;
    KeyMapX11 keymapx11; memset(keymapx11,0,C_KeyMapX11_Size);
    if(LoadKeyMap("./keymap.ini",argv[argc-1],&keymapgdk,&keymapx11))
    {
        flash_window.SetKeyMap(keymapgdk);
        SetKeyMapX11(keymapx11);
    }

    FlashAttributes attrs(argc-1,argv);


// load flash plugin
    if (!flash_player.LoadLibrary())
    {
        return 1;
    }

// do basic plugin initialization
    if (!flash_player.InitPlugin())
    {
        return 1;
    }

// load the wanted file
    if (!flash_player.LoadFile(argv[argc-1],SCREENWIDTH,SCREENHEIGHT,attrs))
    {
        return 1;
    }

    flash_player.Run();

    flash_player.Shutdown();

    return 0;
}
