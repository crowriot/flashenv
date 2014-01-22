// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#ifndef UI_DEFINES_H
#define UI_DEFINES_H


#define SCREENWIDTH       800
#define SCREENHEIGHT      480
#define SCREENBITS        16

#define BACKGROUNDIMAGE   "res/background.png"
#define ICONIMAGE         "res/icon.png"
#define FONTFACE          "res/DroidSans.ttf"
#define LOGOIMAGE         "res/logo.png"
#define FONTHEIGHTBIG     16

#define LEFTOFFSET        20
#define TOPOFFSET         80

#define TOPOFFSETDIR      50
#define LEFTOFFSETDIR     20

#define WIDGETWIDTH       400
#define WIDGETHEIGHT      (FONTHEIGHTBIG+2)
#define FONTCOLORNORMAL   200,200,200,0
#define FONTCOLORSELECTED 200,50,50,0

#define BACKGROUNDCOLOR   100,100,100,0
#define SHADOWCOLOR       20,20,20,0
#define SELECTIONCOLOR    110,110,110,255
#define CONFIGFILE        "flashenvui.cfg"
#define CONFIGFILEVERSION 1
#define LOGOBORDER        2
#define FILEWIDGETCOUNT   21

#define CONFIGSHADOWOFFSETX 3
#define CONFIGSHADOWOFFSETY 3
#define CONFIGLINEHEIGHT  (FONTHEIGHTBIG+4)
#define CONFIGWIDTH       600
#define CONFIGHEIGHT      390
#define CONFIGEDITOFFSET  200
#define CONFIGOFFSETY     60
#define CONFIGBORDER      4

#define SLOGANOFFSETX     10
#define SLOGANOFFSETY     10

// left
#define SDLK_PANDORA_A SDLK_HOME
// right
#define SDLK_PANDORA_B SDLK_END
// up
#define SDLK_PANDORA_Y SDLK_PAGEUP
// down
#define SDLK_PANDORA_X SDLK_PAGEDOWN

#ifdef PANDORA
#define SDL_VIDEOMODE (SDL_SWSURFACE|SDL_FULLSCREEN|SDL_DOUBLEBUF)
#else
#define SDL_VIDEOMODE (SDL_HWSURFACE|SDL_DOUBLEBUF)
#endif

#define RUNSWF "./runswf.sh"

#endif

