// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------


#ifndef FLASHPLAYER_H
#define FLASHPLAYER_H

#include <string>
#include "npapi/npfunctions.h"


class FlashWindow;
class FlashAttributes;


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
    bool LoadFile( const char* file, int window_width, int window_height, const FlashAttributes& attr );

    /// returns the file the plugin was initialized with
    std::string GetFile() const;

    /// returns the directory of the file
    std::string GetPath() const;

    /// run.
    void Run();

    /// shutdown / deinitialize
    void Shutdown();

    /// send event to the plugin
    void SendEvent( void* );

protected:
    void PrintPluginFuncs();

private:
    FlashWindow& m_Window;
    std::string m_File;
    std::string m_Path;

    void* m_FlashPlayerLib;
    NP_InitializeFunc NP_Initialize;
    NP_ShutdownFunc NP_Shutdown;
    NPNetscapeFuncs NPNetscapeFuncs_;
    NPPluginFuncs NPPluginFuncs_;

    NPP_t NPP_;
    NPObject* m_FlashPlugin;
    //NPSavedData NPSavedData_;


};



#endif

