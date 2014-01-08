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
#include "config.h"

using namespace std;


/** widget **/


class FileWidget : public Widget
{
public:
    /// set the file info for this widget
    void SetFileStat(const FileStat& file_info)
    {
        m_FileStat = file_info;
    }
    /// returns the FileStat
    const FileStat& GetFileStat() const
    {
        return m_FileStat;
    }

    virtual string GetText() const
    {
        return m_FileStat.GetText();
    }

private:
    FileStat m_FileStat;
};

class TextWidget : public Widget
{
public:
    void SetText(const string& text)
    {
        m_Text = text;
    }
    string GetText() const
    {
        return m_Text;
    }

private:
    string m_Text;
};

/** widgets */

void FreeWidgets( vector<Widget*>& widget )
{
    for (int i=0,n=widget.size(); i<n; i++ ) {
        if (widget[i]) delete widget[i];
    }
    widget.clear();
}

void InitWidgets( vector<Widget*>* widget_list, TTF_Font* font )
{
    SDL_Rect rect = {LEFTOFFSET,TOPOFFSET,WIDGETWIDTH,WIDGETHEIGHT};

    for (int i=0; i<FILEWIDGETCOUNT; ++i)
    {
        Widget* widget = new FileWidget();

        widget->SetRect(rect);
        widget->SetFont(font);

        widget_list->push_back(widget);

        rect.y += WIDGETHEIGHT;
    }
}

void UpdateWidgets(const vector<Widget*>& widgets, const vector<FileStat>& file_info, int current_file)
{
    const int nfiles = file_info.size();
    const int nwidgets = widgets.size();
    const int half = nwidgets/2 ;

    int file = current_file - half ;

    if (file<0) file = 0;
    if (file+nwidgets>=nfiles) file = nfiles - nwidgets;
    if (file<0) file = 0;

    int iwidget = 0;

    while (iwidget<nwidgets)
    {
        if ( file < nfiles )
        {
            static_cast<FileWidget*>(widgets[iwidget])->SetFileStat(file_info[file]);
        }
        else
        {
            static_cast<FileWidget*>(widgets[iwidget])->SetFileStat(EmptyFileStat);
        }

        widgets[iwidget]->SetSelected(current_file==file);

        ++ file;
        ++ iwidget;
    }
}

void DrawWidgets(SDL_Surface *target, const vector<Widget*>& widgets)
{
    for (int i=0,n=widgets.size(); i<n; i++ )
    {
        widgets[i]->BlitTo(target);
    }
}

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

// current directory
    SDL_Rect cd_rect = {LEFTOFFSETDIR,TOPOFFSETDIR,WIDGETWIDTH,WIDGETHEIGHT};
    TextWidget current_dir_widget;
    current_dir_widget.SetRect(cd_rect);

    vector<Widget*> widget_list; InitWidgets(&widget_list, fontbig);

// start file
#ifdef PANDORA
    FileStat start_file("/media/","");
#else
    FileStat start_file(".","");
#endif

// config file
    FileStat config_file;
    bool config_file_loaded = LoadConfig(&config_file);

// setup initial file list
    int current_file = 0;
    vector<FileStat> file_list;
    if (config_file_loaded && ListDir(config_file,&file_list)!=0)
        current_file = Find(config_file,file_list,0);
    else
        ListDir(start_file,&file_list);

    std::vector<FileStat> file_history; file_history.push_back(file_list[current_file]);
    int history_pos = 0;

//main loop
    FileStat runswf;
    bool update_widgets = true;


    bool done = false;
    while (!done && !runswf.IsFlashFile())
    {
        SDL_BlitSurface(background,0,screen,0);

        SDL_BlitSurface(logo,0,screen,&logorect);

        if (update_widgets)
        {
            UpdateWidgets(widget_list,file_list,current_file);

            if (current_file<file_list.size())
            {
                const FileStat& fi = file_list[current_file];
                current_dir_widget.SetText(fi.GetDir());
            }
            else
            {
                current_dir_widget.SetText("");
            }
        }

        DrawWidgets(screen, widget_list);

        current_dir_widget.BlitTo(screen);

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
                switch(event.key.keysym.sym)
                {
                default: break;
                case 'q':
                    if (event.key.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL))
                        done = true;
                    break;
                case SDLK_ESCAPE:
                    done = true;
                    break;
                case SDLK_UP:
                    if (event.key.keysym.mod & (KMOD_RSHIFT|KMOD_LSHIFT))
                        current_file -= FILEWIDGETCOUNT/2;
                    else
                        current_file --;
                    if (current_file<0)
                        current_file = file_list.size()-1;
                    update_widgets = true;
                break;
                case SDLK_DOWN:
                    if (event.key.keysym.mod & (KMOD_RSHIFT|KMOD_LSHIFT))
                        current_file += FILEWIDGETCOUNT/2;
                    else
                        current_file ++;
                    current_file %= file_list.size();
                    update_widgets = true;
                break;
                case SDLK_LEFT:
                    if (file_history.size() && history_pos-1>=0)
                    {
                        history_pos --;
                        file_list.clear();
                        ListDir(file_history[history_pos],&file_list);
                        current_file = 0;
                        update_widgets = true;
                    }
                    break;
                case SDLK_RIGHT:
                    if (file_history.size() && history_pos+1<file_history.size())
                    {
                        history_pos ++;
                        file_list.clear();
                        ListDir(file_history[history_pos],&file_list);
                        current_file = 0;
                        update_widgets = true;
                    }
                    break;
#ifdef PANDORA
                case SDLK_HOME:
                case SDLK_END:
                case SDLK_PAGEUP:
                case SDLK_PAGEDOWN:
#endif
                case SDLK_RETURN:
                    {
                        if (current_file<file_list.size())
                        {
                            FileStat fi = file_list[current_file];
                            if (fi.IsDir())
                            {
                                if (history_pos+1<file_history.size())
                                    file_history.erase(file_history.begin()+history_pos+1,file_history.end());

                                history_pos = file_history.size();
                                file_history.push_back(fi);

                                file_list.clear();
                                ListDir(fi,&file_list);
                                current_file = 0;
                                update_widgets = true;
                            }
                            else
                            if (fi.IsFlashFile())
                            {
                                runswf = fi;
                            }
                        }
                    }
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

    FreeWidgets(widget_list);

    TTF_CloseFont(fontbig);

    SDL_Quit();
    TTF_Quit();


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
