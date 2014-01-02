// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------


#ifndef FLASHPLAYER_H
#define FLASHPLAYER_H

#include <string>
#include "npapi/npfunctions.h"


class FlashWindow;

/// FlashPlayer
class FlashPlayer
{
public:
    /// constructor
    FlashPlayer( FlashWindow& flash_window );
    /// destructor
    ~FlashPlayer();

    /// loads the flash player library and initializes required
    /// functions.
    /// returns true on success.
    bool LoadLibrary();

    /// initializes the flashplayer plugin
    bool InitPlugin();

    /// loads the given |file| and initializes the plugin window
    /// to width and height as window resolution
    /// set stretch_contents set the scale mode to "exactFit" (may lead to distortion)
    /// otherwise it's set to "showAll" (keeps aspect)
    bool LoadFile( const char* file, int width, int height, bool stretch_contents );

    /// run.
    void Run();

    /// shutdown / deinitialize
    void Shutdown();

protected:
    void PrintPluginFuncs();

private:
    FlashWindow& m_Window;

    void* m_FlashPlayerLib;
    NP_InitializeFunc NP_Initialize;
    NP_ShutdownFunc NP_Shutdown;
    NPNetscapeFuncs NPNetscapeFuncs_;
    NPPluginFuncs NPPluginFuncs_;

    NPP_t NPP_;
    //NPSavedData NPSavedData_;


};



#endif

