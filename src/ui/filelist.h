// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#ifndef FILELIST_H
#define FILELIST_H

#include <vector>
#include <string>


class FileStat
{
public:
    /// constructor
    FileStat();
    /// constructor taking directory and filename
    FileStat(const std::string& dir, const std::string& name);

    /// returns true if the file has .swf extension
    bool IsFlashFile() const;
    /// returns true if FileStat is pointing to a directory.
    bool IsDir() const;

    /// returns the 'name' part
    const std::string& GetName() const;
    /// returns the directory part
    const std::string& GetDir() const;
    /// returns the full path
    const std::string& GetPath() const;

    /// returns the text for displaying
    const std::string& GetText() const;


    /// equal operator
    bool operator==( const FileStat& other ) const;
    /// sorting operator
    bool operator<( const FileStat& other ) const;


private:
    std::string m_Dir;
    std::string m_File;
    std::string m_Path;
    std::string m_Extension;
    std::string m_Text;
    bool m_IsDir;
};

/// empty FileStat pointing nowhere.
extern const FileStat EmptyFileStat;

/// list of files
typedef std::vector<FileStat> FileList;

/// returns the list of files inside the directory of |current_file|
int ListDir( const FileStat& current_file, FileList* entries );

/// returns the index to the |file| in |list|
/// returns default_rval if the file was not found.
int Find(const FileStat& file, const FileList& list, int default_rval);

#endif


