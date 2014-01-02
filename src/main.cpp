// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------


#include <iostream>
#include "flashplayer.h"
#include "flashwindow.h"
#include <string.h>

#define GSEAL(x) x
#include <gtk/gtk.h>

/* -------- */

#define PANDORA

/* -------- */

#define SCREENWIDTH       800
#define SCREENHEIGHT      480
#define SCREENBITS        16
#define STRETCH           false

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


    KeyMap keymap;
    if(LoadKeyMap("./keymap.ini",argv[argc-1],&keymap))
    {
        flash_window.SetKeyMap(keymap);
    }


    bool stretch = STRETCH;
    for (int iarg=0; iarg<argc; ++iarg)
    {
        char* arg = argv[iarg];
        if (arg[0]=='-')
        {
            if (!strcmp(arg,"--stretch"))
            {
                stretch = true;
            }
            if (!strcmp(arg,"--no-stretch"))
            {
                stretch = false;
            }
        }
    }

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
    if (!flash_player.LoadFile(argv[argc-1],SCREENWIDTH,SCREENHEIGHT,stretch))
    {
        return 1;
    }

    flash_player.Run();

    flash_player.Shutdown();

    return 0;
}
