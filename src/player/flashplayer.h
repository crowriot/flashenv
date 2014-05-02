// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------


#ifndef FLASHPLAYER_H
#define FLASHPLAYER_H

#include <string>
#include "../third_party/npapi/npfunctions.h"


class FlashWindow;
class FlashAttributes;


/// FlashPlayer
class FlashPlayer
{
public:
    /// constructor
    FlashPlayer(FlashWindow&, FlashAttributes& attrs);
    /// destructor
    ~FlashPlayer();

    /// returns the flash players window
    inline const FlashWindow& GetWindow() const { return m_Window; }
    /// returns the flash player attributes
    inline const FlashAttributes& GetAttributes() const { return m_Attributes; }

    /// loads the flash player library and initializes required
    /// functions.
    /// returns true on success.
    bool LoadLibrary();

    /// initializes the flashplayer plugin
    bool InitPlugin();

    /// loads the given |file| and initializes the plugin window
    /// set stretch_contents set the scale mode to "exactFit" (may lead to distortion)
    /// otherwise it's set to "showAll" (keeps aspect)
    bool LoadFile( const char* file );

    /// returns the file the plugin was initialized with
    std::string GetFile() const;

    /// returns the directory of the file
    std::string GetPath() const;

    /// returns the name of the file
    std::string GetName() const;

    /// returns the location of the flash file - either the file or the
    /// spoofed location from attributes
    std::string GetLocation() const;

    /// returns the plugin function pointer table
    const NPPluginFuncs& GetPluginFuncs() const;

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
    FlashAttributes& m_Attributes;

    std::string m_File;
    std::string m_Path;
    std::string m_Name;
    std::string m_Location;

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

