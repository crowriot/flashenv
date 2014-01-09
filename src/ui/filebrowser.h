// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include "filelist.h"
#include "widget.h"


/// FileWidget
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

    virtual std::string GetText() const
    {
        return m_FileStat.GetText();
    }
private:
    FileStat m_FileStat;
};

typedef std::vector<FileWidget*> FileWidgetList;

/* -------- */

/// SDL file browser
class FileBrowser
{
public:
    /// constructor
    FileBrowser( TTF_Font* font );
    /// destructor
    ~FileBrowser();

    /// initial file
    /// returns true if any files could be found within the directory of it
    bool SetStartFile( const FileStat& );

    /// returns the current file list
    const FileList& GetFileList() const { return m_FileList; }

    /// returns the currently selected file
    /// -1 on error
    const int GetCurrentFileIndex() const { return m_CurrentFile; }

    /// returns FileInfo of the currently selected file or
    /// EmptyFileStat on error.
    const FileStat& GetCurrentFile() const;


    /** navigation **/

    /// next file in list
    void Next();
    /// previous file in list
    void Prev();
    /// enter if it's a directory
    void Enter();
    /// search
    void Search(char chr);

    /// next in directory history
    void NextHistory();
    /// prev in directory history
    void PrevHistory();

    /** drawing **/

    /// draw widgets to the screen
    void BlitTo(SDL_Surface* screen);


protected:
    /// update the file list using the given file stat structure
    bool UpdateFileList( const FileStat& );

    /// update widgets filestats
    void UpdateWidgets();


private:
    int m_CurrentFile;
    FileList m_FileList;
    int m_HistoryIndex;
    FileList m_History;
    FileWidgetList m_FileWidgets;
    TextWidget m_DirWidget;
    char m_Search;
};


#endif


