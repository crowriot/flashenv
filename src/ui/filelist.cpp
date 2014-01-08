// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#include "filelist.h"

#include <dirent.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <string.h>
#include <cstdlib>
#include <algorithm>
#include <iostream>

const FileStat EmptyFileStat;

/* -------- */

static std::string my_realpath(const char* dir0)
{
    if (dir0[0]=='~') {
        char* home = getenv("HOME");
        if (home) {
            std::string path(home);
            path += dir0+1;
            return path;
        }
    } else {
        char tmp[PATH_MAX];
        realpath(dir0,tmp);
        return std::string(tmp);
    }
    return "";
}
static std::string my_realpath(const std::string& path)
{
    return my_realpath(path.c_str());
}

static bool FileExists( const std::string& file )
{
    struct stat st;
    return stat(file.c_str(),&st)>=0;
}

static int SortCompare( const FileStat& L, const FileStat& R )
{
    int r = 0;

    if ( L.IsDir()==R.IsDir() )
        r = strcasecmp(L.GetName().c_str(), R.GetName().c_str());
    else
        r = R.IsDir() - L.IsDir();

    return r < 0 ? -1 : r > 0 ? 1 : 0;
}

/* -------- */

FileStat::FileStat()
    : m_IsDir(false)
{

}

FileStat::FileStat(const std::string& dir, const std::string& file)
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
}

bool FileStat::IsFlashFile() const
{
    return m_Extension == ".swf";
}

bool FileStat::IsDir() const
{
    return m_IsDir;
}

const std::string& FileStat::GetName() const
{
    return m_File;
}

const std::string& FileStat::GetDir() const
{
    return m_Dir;
}

const std::string& FileStat::GetPath() const
{
    return m_Path;
}

const std::string& FileStat::GetText() const
{
    return m_Text;
}

bool FileStat::operator==( const FileStat& other ) const
{
    return m_Path == other.m_Path;
}

bool FileStat::operator<( const FileStat& other ) const
{
    return SortCompare( *this, other ) < 0;
}


/* -------- */


int ListDir( const FileStat& current_file, FileList* entries )
{
    std::string directory = current_file.IsDir() ? current_file.GetPath() : current_file.GetDir();

    DIR* dir = opendir(directory.c_str());
    if (!dir)
    {
        std::cerr << "Failed to open directory: " << directory << std::endl;
        return 0;
    }

    struct dirent* entry = 0;
    while ((entry=readdir(dir))!=0)
    {
        if (entry->d_name[0]=='.' && entry->d_name[1]!='.')
            continue;

        FileStat file_info(directory.c_str(), entry->d_name);
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

int Find(const FileStat& file, const FileList& list, int default_rval)
{
    int i=-1,n = list.size();
    while (++i<n)
    {
        if (list[i]==file)
            return i;
    }
    return default_rval;
}
