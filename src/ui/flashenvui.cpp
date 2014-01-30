// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------


#include <cstdlib>
#include <algorithm>
#include <SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>
#include <vector>
#include <string>
#include <dirent.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <strings.h>
#include <iostream>
#include "defines.h"
#include "widget.h"
#include "filelist.h"
#include "filebrowser.h"
#include "config.h"
#include "gameconfig.h"

using namespace std;


/** widgets **/

Widget* PickWidget(int mx, int my, const vector<Widget*>& widgets )
{
    for (int i=0,n=widgets.size(); i<n; i++)
    {
        if (widgets[i]->Pick(mx,my))
            return widgets[i];
    }

    return NULL;
}

/** main */

int main ( int argc, char** argv )
{
    if (TTF_Init()<0)
    {
        cerr << "Unable to init TTF: " << TTF_GetError()  << endl;
        return 1;
    }
    // initialize SDL video
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0)
    {
        cerr << "Unable to init SDL: " << SDL_GetError()  << endl;
        return 1;
    }

    if ((SDL_VIDEOMODE&SDL_FULLSCREEN)==0) {
        SDL_putenv("SDL_VIDEO_CENTERED=center"); //Center the game Window
    }

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
#ifdef PANDORA
    SDL_ShowCursor(0);
#endif

    // create a new window
    SDL_Surface* screen = SDL_SetVideoMode(SCREENWIDTH, SCREENHEIGHT, SCREENBITS, SDL_VIDEOMODE);
    if ( !screen )
    {
        cerr << "Unable to set " << SCREENWIDTH << "x" << SCREENHEIGHT << " video mode. Error: " << SDL_GetError() << endl;
        return 1;
    }


// load background image
    SDL_Surface* background = IMG_Load(BACKGROUNDIMAGE);

// fooonts
    TTF_Font* fontbig = TTF_OpenFont(FONTFACE,FONTHEIGHTBIG);

// window stuff
    SDL_Surface* icon = IMG_Load(ICONIMAGE);
    SDL_WM_SetIcon(icon,NULL);
    SDL_WM_SetCaption("flashenv.ui","flashenv.ui");

// compo logo
    SDL_Surface* logo = IMG_Load(LOGOIMAGE);
    SDL_Rect logorect = {screen->w-logo->w-LOGOBORDER,screen->h-logo->h-LOGOBORDER,logo->w,logo->h};

// compo desc
    TextWidget compo_txt;
    compo_txt.SetFont(fontbig);
    compo_txt.SetText("Pandora Alive and Kicking Coding Competition 2014");
    compo_txt.UpdateSurfaces();
    SDL_Rect r = compo_txt.GetRect();
    r.x = screen->w - compo_txt.GetTextWidth() - SLOGANOFFSETX;
    r.y = SLOGANOFFSETY;
    compo_txt.SetRect(r);

// the browser
    FileBrowser browser(fontbig);

// config widget
    GameConfigWindow game_config(fontbig);
    game_config.LoadINI();

// start file
#ifdef PANDORA
    FileStat start_file("/media/","");
#else
    FileStat start_file(".","");
#endif

// config file
    FileStat config_file;
    bool config_file_loaded = LoadConfig(&config_file);

// initial file listing
// either form the config or fallback to a 'should work' directory
    if (!config_file_loaded || !browser.SetStartFile(config_file))
        browser.SetStartFile(start_file);

//main loop
    FileStat runswf;


    bool done = false;
    while (!done && !runswf.IsFlashFile())
    {
        SDL_BlitSurface(background,0,screen,0);

        SDL_BlitSurface(logo,0,screen,&logorect);

        compo_txt.BlitTo(screen);

        browser.BlitTo(screen);

        game_config.BlitTo(screen);

        SDL_Flip(screen);

// using waitevent not poll ... no per-frame updated needed
        SDL_Event event;
        if (SDL_WaitEvent(&event))
        {

            switch (event.type)
            {
            case SDL_QUIT:
                done = true;
                break;

            case SDL_KEYDOWN:
                if (game_config.IsShown())
                {
                    game_config.OnKeyDown(event);
                    break;
                }
                if (browser.OnKeyDown(event))
                {
                    break;
                }
                switch (event.key.keysym.sym)
                {
                case SDLK_PANDORA_Y:
                    if (browser.GetCurrentFile().IsFlashFile())
                    {
                        game_config.ConfigFile(browser.GetCurrentFile());
                        game_config.Show();
                    }
                    break;
                case SDLK_PANDORA_B:
                case SDLK_PANDORA_A:
                case SDLK_PANDORA_X:
                case SDLK_RETURN:
                    if (browser.GetCurrentFile().IsFlashFile())
                    {
                        runswf = browser.GetCurrentFile();
                    }
                    break;
                case SDLK_q:
                    if (event.key.keysym.mod&KMOD_CTRL)
                    {
                        done = true;
                    }
                    break;
                case SDLK_ESCAPE:
                    done = true;
                    break;
                default:
                    break;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                break;
            }
        }
    }

    SDL_FreeSurface(background);
    SDL_FreeSurface(icon);
    SDL_FreeSurface(logo);

    TTF_CloseFont(fontbig);

    SDL_Quit();
    TTF_Quit();

#ifdef PANDORA
    system("sync");
#endif

    if (runswf.IsFlashFile())
    {
        SaveConfig(runswf);

        string cmdline = RUNSWF;
        cmdline += " \"" + runswf.GetPath() +"\"";
        cmdline += " \"" + string(argv[0]) +"\"";
        cout << "cmdline=" << cmdline << endl;
        system(cmdline.c_str());
    }


    return 0;
}
