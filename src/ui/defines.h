// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#ifndef DEFINES_H
#define DEFINES_H


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
#define SELECTIONCOLOR    110,110,110,255
#define CONFIGFILE        "flashenvui.cfg"
#define CONFIGFILEVERSION 1
#define LOGOBORDER        2
#define FILEWIDGETCOUNT   21

#define SLOGANOFFSETX     10
#define SLOGANOFFSETY     10

#ifdef PANDORA
#define SDL_VIDEOMODE (SDL_SWSURFACE|SDL_FULLSCREEN|SDL_DOUBLEBUF)
#else
#define SDL_VIDEOMODE (SDL_HWSURFACE|SDL_DOUBLEBUF)
#endif

#define RUNSWF "./runswf.sh"

#endif

