// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------


#include "filebrowser.h"
#include "widget.h"
#include "defines.h"


FileBrowser::FileBrowser( TTF_Font* font )
    : m_CurrentFile(-1)
    , m_HistoryIndex(-1)
    , m_Search(0)
{

    SDL_Rect rect = {LEFTOFFSET,TOPOFFSET,WIDGETWIDTH,WIDGETHEIGHT};

    for (int i=0; i<FILEWIDGETCOUNT; ++i)
    {
        FileWidget* widget = new FileWidget();

        widget->SetRect(rect);
        widget->SetFont(font);

        m_FileWidgets.push_back(widget);

        rect.y += WIDGETHEIGHT;
    }
// current directory
    SDL_Rect cd_rect = {LEFTOFFSETDIR,TOPOFFSETDIR,WIDGETWIDTH,WIDGETHEIGHT};
    m_DirWidget.SetRect(cd_rect);
    m_DirWidget.SetFont(font);
}

FileBrowser::~FileBrowser()
{
    for (size_t i=0,n=m_FileWidgets.size(); i<n; ++i)
    {
        delete m_FileWidgets[i];
    }
}

bool FileBrowser::SetStartFile(const FileStat& start_file)
{
    if (UpdateFileList(start_file))
    {
        m_CurrentFile = Find(start_file,m_FileList,0);
        m_History.clear();
        m_History.push_back(start_file);
        m_HistoryIndex = 0;

        UpdateWidgets();
        return true;
    }
    return false;
}

const FileStat& FileBrowser::GetCurrentFile() const
{
    if (m_CurrentFile>=0 && m_CurrentFile<m_FileList.size())
    {
        return m_FileList[m_CurrentFile];
    }
    return EmptyFileStat;
}

bool FileBrowser::UpdateFileList(const FileStat& file)
{
    FileList file_list;
    ListDir(file,&file_list);
    if (file_list.size())
    {
        m_FileList = file_list;
        m_CurrentFile = 0;

        UpdateWidgets();

        return true;
    }
    return false;
}

void FileBrowser::Next()
{
    if (m_FileList.size())
    {
        int last_file = m_CurrentFile;
        ++ m_CurrentFile %= m_FileList.size();

        if (m_CurrentFile!=last_file)
            UpdateWidgets();
    }
}

void FileBrowser::Prev()
{
    if (m_FileList.size())
    {
        int last_file = m_CurrentFile;
        -- m_CurrentFile;

        if (m_CurrentFile<0)
            m_CurrentFile = m_FileList.size()-1;

        if (last_file!=m_CurrentFile)
            UpdateWidgets();
    }
}

void FileBrowser::Enter()
{
    if (GetCurrentFile().IsDir())
    {
        if (m_HistoryIndex+1<m_History.size())
            m_History.erase(m_History.begin()+m_HistoryIndex+1,m_History.end());

        m_HistoryIndex = m_History.size();
        m_History.push_back(GetCurrentFile());

        UpdateFileList(GetCurrentFile());
    }
}

void FileBrowser::Search(char chr)
{
    int search_start[] = {
        m_CurrentFile,
        -1
    };

    for (int start=0; start<2;++start)
    {
        for (int i=search_start[start]+1, n=m_FileList.size(); i<n; ++i)
        {
            const FileStat& file = m_FileList[i];
            char file_chr = file.GetName().size() ? file.GetName()[0] : 0;

            if (tolower(file_chr)==tolower(chr))
            {
                m_Search = chr;
                m_CurrentFile = i;
                UpdateWidgets();
                return;
            }
        }
    }

    m_Search = 0;
}

void FileBrowser::NextHistory()
{
    if (m_History.size() && m_HistoryIndex+1<m_History.size())
    {
        m_HistoryIndex ++;
        m_CurrentFile = 0;
        UpdateFileList(m_History[m_HistoryIndex]);
    }
}

void FileBrowser::PrevHistory()
{
    if (m_History.size() && m_HistoryIndex-1>=0)
    {
        m_HistoryIndex --;
        m_CurrentFile = 0;
        UpdateFileList(m_History[m_HistoryIndex]);
    }
}

void FileBrowser::BlitTo(SDL_Surface* screen)
{
    for(size_t i=0,n=m_FileWidgets.size(); i<n; ++i)
    {
        m_FileWidgets[i]->BlitTo(screen);
    }
    m_DirWidget.BlitTo(screen);
}

void FileBrowser::UpdateWidgets()
{
    const int nfiles = m_FileList.size();
    const int nwidgets = m_FileWidgets.size();
    const int half = nwidgets/2 ;

    int file = m_CurrentFile - half ;

    if (file<0) file = 0;
    if (file+nwidgets>=nfiles) file = nfiles - nwidgets;
    if (file<0) file = 0;

    int iwidget = 0;

    while (iwidget<nwidgets)
    {
        if ( file < nfiles )
        {
            m_FileWidgets[iwidget]->SetFileStat(m_FileList[file]);
        }
        else
        {
            m_FileWidgets[iwidget]->SetFileStat(EmptyFileStat);
        }

        m_FileWidgets[iwidget]->SetSelected(m_CurrentFile==file);

        ++ file;
        ++ iwidget;
    }

    if (m_CurrentFile>=0 && m_CurrentFile<m_FileList.size())
    {
        m_DirWidget.SetText(m_FileList[m_CurrentFile].GetDir());
    }
    else
    {
        m_DirWidget.SetText("");
    }
}

