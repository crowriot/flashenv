// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#include "flashattributes.h"

const char* FlashAttributes::BooleanTrue = "true";
const char* FlashAttributes::BooleanFalse = "false";

const char* FlashAttributes::Quality[FlashAttributes::QualityCount] = {
    "low", "autolow", "autohigh", "medium", "high", "best"
};

const char* FlashAttributes::Scale[FlashAttributes::ScaleCount] = {
    "showall", "noborder", "exactfit", "noscale"
};

FlashAttributes::FlashAttributes()
    : menu(FlashAttributes::BooleanTrue)
    , quality(FlashAttributes::Quality[FlashAttributes::DefaultQuality])
    , scale(FlashAttributes::Scale[FlashAttributes::DefaultScale])
{
}

FlashAttributes::FlashAttributes(int argc, char* argv[])
    : menu(FlashAttributes::BooleanTrue)
    , quality(FlashAttributes::Quality[FlashAttributes::DefaultQuality])
    , scale(FlashAttributes::Scale[FlashAttributes::DefaultScale])
{

}
