// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#ifndef FLASHATTRIBUTES_H
#define FLASHATTRIBUTES_H


/// Flash plugin attributes
/// http://helpx.adobe.com/flash/kb/flash-object-embed-tag-attributes.html
class FlashAttributes
{
public:
    /// "true"
    static const char* BooleanTrue;
    /// "false"
    static const char* BooleanFalse;

    /// quality attr enumeration
    enum { QualityCount = 6, DefaultQuality=1 };
    /// quality attributes
    static const char* Quality[QualityCount];

    /// scale attr enumeration
    enum { ScaleCount=4, DefaultScale=0 };
    ///scale attributes
    static const char* Scale[ScaleCount];

public:
    /// constructor
    /// intializes the attributes to their default values.
    FlashAttributes();

    /// constructor
    /// intializes attributes using the command line
    FlashAttributes(int argc, char* argv[]);

public:
    const char* menu;
    const char* quality;
    const char* scale;
};



#endif


