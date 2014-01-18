// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#include "flashattributes.h"
#include "defines.h"
#include <iostream>

extern "C" {
#include "../third_party/iniparser-3.0/src/iniparser.h"
}

/* -------- */

using namespace std;

/* -------- */

const char* FlashAttributes::BooleanTrue = "true";
const char* FlashAttributes::BooleanFalse = "false";

const char* FlashAttributes::Quality[FlashAttributes::QualityCount] = {
    "low", "autolow", "autohigh", "medium", "high", "best"
};

const char* FlashAttributes::Scale[FlashAttributes::ScaleCount] = {
    "showall", "noborder", "exactfit", "noscale"
};

/* -------- */

FlashAttributes::FlashAttributes()
    : menu(FlashAttributes::BooleanTrue)
    , quality(FlashAttributes::Quality[FlashAttributes::DefaultQuality])
    , scale(FlashAttributes::Scale[FlashAttributes::DefaultScale])
    , window_src_width(SCREENWIDTH)
    , window_src_height(SCREENHEIGHT)
    , window_trg_width(0)
    , window_trg_height(0)
{
}

char* AttrPath( char* tmp, const char* section, const char* attr )
{
    sprintf(tmp,"%s:%s",section,attr);
    return tmp;
}

#define GET_INT_VALUE( var, attr ) \
    attributes-> var = iniparser_getint(d,AttrPath(tmp,section,attr), attributes-> var)

#define GET_STR_VALUE( var, attr ) \
    attributes-> var = iniparser_getstring(d, AttrPath(tmp,section,attr), (char*)attributes-> var .c_str())


bool LoadAttributes(const char* inifile, const char* swffile, FlashAttributes* attributes)
{
    DEBUG_FUNCTION_NAME

    const char* section = strrchr(swffile,'/');
    if (!section)
        section = swffile;
    else
        section ++;

    cout << "\tSection=" << section << endl;

    dictionary* d = iniparser_load(const_cast<char*>(inifile));
    if (d==0)
    {
        cerr << "Failed to load " << inifile << endl;
        return false;
    }


    char tmp[256];
    GET_INT_VALUE(window_src_width , "WindowWidth");
    GET_INT_VALUE(window_src_height, "WindowHeight");
    GET_INT_VALUE(window_src_width , "width");
    GET_INT_VALUE(window_src_height, "height");
    GET_INT_VALUE(window_trg_width , "FramebufferWidth");
    GET_INT_VALUE(window_trg_height, "FramebufferHeight");

    GET_STR_VALUE(menu, "menu");
    GET_STR_VALUE(quality, "quality");
    GET_STR_VALUE(scale,"scale");

    iniparser_freedict(d);
    return true;
}


