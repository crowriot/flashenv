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


#define SCREENWIDTH       800
#define SCREENHEIGHT      480
#define SCREENBITS        16
#define BACKGROUNDIMAGE   "res/background.png"
#define ICONIMAGE         "res/icon.png"
#define CLOSEIMAGE        "res/close.png"
#define FONTFACE          "res/DroidSans.ttf"
#define LOGOIMAGE         "res/logo.png"
#define FONTHEIGHTBIG     16
#define FONTHEIGHTSMALL   12

#define LEFTOFFSET        20
#define TOPOFFSET         80

#define TOPOFFSETDIR      50
#define LEFTOFFSETDIR     20

#define WIDGETWIDTH       400
#define WIDGETHEIGHT      (FONTHEIGHTBIG+2)
#define FONTCOLOR         200,200,200,0
#define FONTCOLORSELECTED 200,50,50,0
#define BACKGROUNDCOLOR   100,100,100,0
#define SELECTIONCOLOR    110,110,110,255
#define CONFIGFILE        "flashenvui.cfg"
#define CONFIGFILEVERSION 1
#define LOGOBORDER        2
#define FILEWIDGETCOUNT   21


#ifdef PANDORA
#define SDL_VIDEOMODE (SDL_SWSURFACE|SDL_FULLSCREEN|SDL_DOUBLEBUF)
#else
#define SDL_VIDEOMODE (SDL_HWSURFACE|SDL_DOUBLEBUF)
#endif

#define RUNSWF "./runswf.sh"


using namespace std;


/** utils */

void Replace( string& inout, const string& find, const string& replace )
{
    size_t pos=0;
    while ((pos=inout.find(find,pos))!=string::npos)
    {
        inout.replace(pos,find.length(),replace);
        pos += replace.length();
    }
}

/** file stuff **/


string my_realpath(const char* dir0)
{
    if (dir0[0]=='~') {
        char* home = getenv("HOME");
        if (home) {
            string path(home);
            path += dir0+1;
            return path;
        }
    } else {
        char tmp[PATH_MAX];
        realpath(dir0,tmp);
        return string(tmp);
    }
    return "";
}
string my_realpath(const string& path)
{
    return my_realpath(path.c_str());
}

bool FileExists( const string& file )
{
    struct stat st;
    return stat(file.c_str(),&st)>=0;
}



class FileInfo
{
public:
    FileInfo()
        : m_IsDir(false)
    {

    }
    FileInfo(const string& dir, const string& file)
        : m_Dir(dir)
        , m_File(file)
    {
        m_Path = my_realpath(m_Dir+"/"+m_File);
        struct stat stat;
        lstat(m_Path.c_str(),&stat);
        m_IsDir = S_ISDIR(stat.st_mode);

        const char* ext = 0;
        if (!m_IsDir && (ext=strrchr(file.c_str(),'.')))
        {
            m_Extension = ext;
        }
        transform(m_Extension.begin(), m_Extension.end(), m_Extension.begin(), ::tolower);

        if (m_IsDir)
            m_Text = "[" + m_File + "]";
        else
            m_Text = m_File;

//      cout << "dir=" << dir << " file=" << file << " path=" << m_Path << endl;
    }

    bool IsFlashFile() const
    {
        return m_Extension == ".swf";
    }

    bool IsDir() const
    {
        return m_IsDir;
    }

    const string& GetFile() const
    {
        return m_File;
    }

    const string& GetDir() const
    {
        return m_Dir;
    }

    const string& GetPath() const
    {
        return m_Path;
    }

    const string& GetText() const
    {
        return m_Text;
    }

    bool operator==( const FileInfo& other ) const
    {
//      cout << m_Path << " == " << other.m_Path << endl;
        return m_Path == other.m_Path;
    }

    bool operator<( const FileInfo& other ) const
    {
        return SortCompare( *this, other ) < 0;
    }

    static int SortCompare( const FileInfo& L, const FileInfo& R )
    {
        int r = 0;

        if ( L.IsDir()==R.IsDir() )
            r = strcasecmp(L.GetFile().c_str(), R.GetFile().c_str());
        else
            r = R.IsDir() - L.IsDir();

        return r < 0 ? -1 : r > 0 ? 1 : 0;
    }



private:
    string m_Dir;
    string m_File;
    string m_Path;
    string m_Extension;
    string m_Text;
    bool m_IsDir;
};

static const FileInfo EmptyFileInfo;

/** file list **/

int ListDir( const FileInfo& current_file, vector<FileInfo>* entries )
{
    string directory = current_file.IsDir() ? current_file.GetPath() : current_file.GetDir();

    DIR* dir = opendir(directory.c_str());
    if (!dir)
    {
        cerr << "Failed to open directory: " << directory << endl;
        return 0;
    }

    struct dirent* entry = 0;
    while ((entry=readdir(dir))!=0)
    {
        if (entry->d_name[0]=='.' && entry->d_name[1]!='.')
            continue;

        FileInfo file_info(directory.c_str(), entry->d_name);
#ifdef PANDORA
        if (file_info.IsDir() || file_info.IsFlashFile())
#endif
        {
            entries->push_back(file_info);
        }
    }
    closedir(dir);

    std::sort(entries->begin(), entries->end());

    return entries->size();
}

int Find(const FileInfo& info, const vector<FileInfo>& list, int default_rval)
{
    int i=-1,n = list.size();
    while (++i<n)
    {
        if (list[i]==info)
            return i;
    }
    return default_rval;
}

/** widget **/

/// simple widget class
class Widget
{
public:
    /// constructor
    Widget()
        : m_TextSurface(NULL)
        , m_TextSelectedSurface(NULL)
        , m_Selected(0)
    {
        memset(&m_TextRect,0,sizeof(m_TextRect));
    }

    /// destructor
    virtual ~Widget()
    {
        if (m_TextSurface) SDL_FreeSurface(m_TextSurface);
        if (m_TextSelectedSurface) SDL_FreeSurface(m_TextSelectedSurface);
    }


    /** selection **/

    /// pick the widget, returns true if mx and my are within the widget bounds
    bool Pick( int mx, int my )
    {
        return mx>=m_TextRect.x && mx<=m_TextRect.x+m_TextRect.w
             && my>=m_TextRect.y && my<=m_TextRect.y+m_TextRect.h;
    }

    /// set selection
    void SetSelected( int select )
    {
        m_Selected = select;
    }
    /// get selection
    int GetSelected() const
    {
        return m_Selected;
    }

    /** sdl surface **/

    virtual string GetText() const = 0;

    void UpdateText(TTF_Font* font)
    {
        string text = GetText();
        if ( m_CurrentText!=text )
        {
            m_CurrentText = text;

            if (m_TextSurface)
                SDL_FreeSurface(m_TextSurface);
            if (m_TextSelectedSurface)
                SDL_FreeSurface(m_TextSelectedSurface);

            SDL_Color clr = {FONTCOLOR};
            m_TextSurface = TTF_RenderText_Blended(font,text.c_str(),clr);

            int old_style = TTF_GetFontStyle(font);
            TTF_SetFontStyle(font,TTF_STYLE_BOLD);
            SDL_Color sel_clr = {FONTCOLORSELECTED};
            m_TextSelectedSurface = TTF_RenderText_Blended(font,text.c_str(),sel_clr);
            TTF_SetFontStyle(font,old_style);
        }
    }

    /** icon rect **/

    void SetRect( const SDL_Rect& rect )
    {
        m_TextRect = rect;
    }

    void BlitTo(SDL_Surface* target)
    {
        SDL_Rect target_rect = m_TextRect;

        if (m_Selected && m_TextSelectedSurface) {
            SDL_BlitSurface(m_TextSelectedSurface,NULL,target,&target_rect);
        }
        else
        if (m_TextSurface) {
            SDL_BlitSurface(m_TextSurface,NULL,target,&target_rect);
        }
    }

private:
    SDL_Rect m_TextRect;
    SDL_Surface* m_TextSurface;
    SDL_Surface* m_TextSelectedSurface;
    string m_CurrentText;
    int m_Selected;
};


class FileWidget : public Widget
{
public:
    /// set the file info for this widget
    void SetFileInfo(const FileInfo& file_info)
    {
        m_FileInfo = file_info;
    }
    /// returns the fileinfo
    const FileInfo& GetFileInfo() const
    {
        return m_FileInfo;
    }

    virtual string GetText() const
    {
        return m_FileInfo.GetText();
    }

private:
    FileInfo m_FileInfo;
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

void InitWidgets( vector<Widget*>* widget_list )
{
    SDL_Rect rect = {LEFTOFFSET,TOPOFFSET,WIDGETWIDTH,WIDGETHEIGHT};

    for (int i=0; i<FILEWIDGETCOUNT; ++i)
    {
        Widget* widget = new FileWidget();

        widget->SetRect(rect);

        widget_list->push_back(widget);

        rect.y += WIDGETHEIGHT;
    }
}

void UpdateWidgets(const vector<Widget*>& widgets, const vector<FileInfo>& file_info, int current_file, TTF_Font* font)
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
            static_cast<FileWidget*>(widgets[iwidget])->SetFileInfo(file_info[file]);
        }
        else
        {
            static_cast<FileWidget*>(widgets[iwidget])->SetFileInfo(EmptyFileInfo);
        }

        widgets[iwidget]->SetSelected(current_file==file);

        widgets[iwidget]->UpdateText(font);

        ++ file;
        ++ iwidget;
    }
}

void DrawWidgets(SDL_Surface *target, const vector<Widget*>& widgets)
{
    for (int i=0,n=widgets.size(); i<n; i++ ) {
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

/** "config" file **/

void WriteString( FILE* fp, const string& str )
{
    int l = str.size();
    fwrite(&l,sizeof(int),1,fp);
    fwrite(str.c_str(),sizeof(char)*l,1,fp);
}

string ReadString( FILE* fp )
{
    int l=0;
    if (fread(&l,sizeof(int),1,fp)!=1) {
        return "";
    }
    string rval; rval.resize(l);
    fread((char*)rval.c_str(),sizeof(char)*l,1,fp);
    return rval;
}

void SaveConfig( const FileInfo& swf )
{
    FILE* fp = fopen(CONFIGFILE,"wb");
    if(fp) {
        int version = CONFIGFILEVERSION;
        fwrite(&version,sizeof(int),1,fp);
        WriteString(fp,swf.GetDir());
        WriteString(fp,swf.GetFile());
        fclose(fp);
    }
}

FileInfo LoadConfig()
{
    FILE* fp = fopen(CONFIGFILE,"rb");
    if(fp)
    {
        int version=0;
        if (fread(&version,sizeof(int),1,fp)!=1 || version!=CONFIGFILEVERSION) {
            fclose(fp);
            return EmptyFileInfo;
        }
        string dir = ReadString(fp);
        string file = ReadString(fp);
        return FileInfo(dir,file);
    }
    return EmptyFileInfo;
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
    SDL_Color fontcolor = {FONTCOLOR};


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


    vector<Widget*> widget_list; InitWidgets(&widget_list);

// start file
#ifdef PANDORA
    FileInfo start_file("/media/","");
#else
    FileInfo start_file(".","");
#endif

// config file
    FileInfo config_file = LoadConfig();

// setup initial file list
    int current_file = 0;
    vector<FileInfo> file_list;
    if (ListDir(config_file,&file_list)!=0)
        current_file = Find(config_file,file_list,0);
    else
        ListDir(start_file,&file_list);

    std::vector<FileInfo> file_history; file_history.push_back(file_list[current_file]);
    int history_pos = 0;

//main loop
    FileInfo runswf;
    bool update_widgets = true;


    bool done = false;
    while (!done && !runswf.IsFlashFile())
    {
        SDL_BlitSurface(background,0,screen,0);

        SDL_BlitSurface(logo,0,screen,&logorect);

        if (update_widgets)
        {
            UpdateWidgets(widget_list,file_list,current_file,fontbig);

            if (current_file<file_list.size())
            {
                const FileInfo& fi = file_list[current_file];
                current_dir_widget.SetText(fi.GetDir());
            }
            else
            {
                current_dir_widget.SetText("");
            }
            current_dir_widget.UpdateText(fontbig);
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
                case SDLK_ESCAPE: done = true; break;
                case SDLK_UP:
                    -- current_file;
                    if (current_file<0) current_file = file_list.size()-1;
                    update_widgets = true;
                break;
                case SDLK_DOWN:
                    ++ current_file %= file_list.size();
                    update_widgets = true;
                break;
#ifdef PANDORA
                case SDLK_HOME:
                case SDLK_END:
                case SDLK_PAGEUP:
                case SDLK_PAGEDOWN:
#endif
                case SDLK_LEFT:
                    if (file_history.size() && history_pos-1>=0)
                    {
                        history_pos --;
                        file_list.clear();
                        ListDir(file_history[history_pos-1],&file_list);
                        current_file = 0;
                        update_widgets = true;
                    }
                    break;
                case SDLK_RIGHT:
                    if (file_history.size() && history_pos+1<file_history.size())
                    {
                        history_pos ++;
                        file_list.clear();
                        ListDir(file_history[history_pos+1],&file_list);
                        current_file = 0;
                        update_widgets = true;
                    }
                    break;
                case SDLK_RETURN:
                    {
                        if (current_file<file_list.size())
                        {
                            FileInfo fi = file_list[current_file];
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
