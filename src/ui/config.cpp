// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#include "defines.h"
#include "config.h"
#include "filelist.h"
#include <string>
#include <cstdio>


static void WriteString( FILE* fp, const std::string& str )
{
    int l = str.size();
    fwrite(&l,sizeof(int),1,fp);
    fwrite(str.c_str(),sizeof(char)*l,1,fp);
}

static std::string ReadString( FILE* fp )
{
    int l=0;
    if (fread(&l,sizeof(int),1,fp)!=1) {
        return "";
    }
    std::string rval; rval.resize(l);
    fread((char*)rval.c_str(),sizeof(char)*l,1,fp);
    return rval;
}


bool SaveConfig( const FileStat& swf )
{
    FILE* fp = fopen(CONFIGFILE,"wb");
    if(fp)
    {
        int version = CONFIGFILEVERSION;
        fwrite(&version,sizeof(int),1,fp);
        WriteString(fp,swf.GetDir());
        WriteString(fp,swf.GetName());
        fclose(fp);
        return true;
    }
    return false;
}


bool LoadConfig( FileStat* file_stat )
{
    FILE* fp = fopen(CONFIGFILE,"rb");
    if(fp)
    {
        int version=0;
        if (fread(&version,sizeof(int),1,fp)!=1 || version!=CONFIGFILEVERSION)
        {
            fclose(fp);
            return false;
        }
        std::string dir = ReadString(fp);
        std::string file = ReadString(fp);
        *file_stat = FileStat(dir,file);
        return true;
    }
    return false;
}


