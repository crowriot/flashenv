// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------

#include "flashplayer.h"
#include "flashwindow.h"
#include "flashattributes.h"
#include "defines.h"
#include "npn.h"
#include "mimetypes.h"
#include <dlfcn.h>
#include <iostream>
#include <iomanip>
#include <string.h>


/* -------- */

#define FLASHPLAYERLIB "./flashplayer/libflashplayer.so"

#define CHECK_ERROR_RETURN(desc) \
    if (err!=NPERR_NO_ERROR) \
    { cerr << desc << " failed. Error #" << err << endl; return false; } \
    else \
    { cout << desc << " done." << endl; }

/* -------- */

using namespace std;


string ExtractFilepath( const string& path )
{
    size_t p = path.rfind( '/' );
    if ( p==string::npos ) p = path.rfind('\\');
    if ( p==string::npos ) return path;
    return path.substr( 0, p+1 );
}

/* -------- */

FlashPlayer::FlashPlayer( FlashWindow& flash_window, FlashAttributes& flash_attributes )
    : m_Window(flash_window)
    , m_Attributes(flash_attributes)
    , m_FlashPlayerLib(0)
{
    memset(&NPPluginFuncs_,0,sizeof(NPPluginFuncs));
    memset(&NPNetscapeFuncs_,0,sizeof(NPNetscapeFuncs));
    memset(&NPP_,0,sizeof(NPP));

    NPP_.ndata = this;

    flash_window.SetPlayer(this);

    InitializeNPN(&NPNetscapeFuncs_);
}


FlashPlayer::~FlashPlayer()
{
    m_Window.SetPlayer(0);

    if (m_FlashPlayerLib)
        dlclose(m_FlashPlayerLib);
    m_FlashPlayerLib = 0;
}


bool FlashPlayer::LoadLibrary()
{
    m_FlashPlayerLib = dlopen(FLASHPLAYERLIB,RTLD_NOW);

    if (!m_FlashPlayerLib)
    {
        cerr << dlerror() << endl;
        return false;
    }

    NP_Initialize = reinterpret_cast<NP_InitializeFunc>(dlsym(m_FlashPlayerLib,"NP_Initialize"));
    if (NP_Initialize==0)
    {
        cerr << "NP_Initialize not found." << endl;
        return false;
    }
    NP_Shutdown = reinterpret_cast<NP_ShutdownFunc>(dlsym(m_FlashPlayerLib,"NP_Shutdown"));
    if (NP_Shutdown==0)
    {
        cerr << "NP_Shutdown not found." << endl;
        return false;
    }

    return true;
}

bool FlashPlayer::InitPlugin()
{
    NPNetscapeFuncs_.size = sizeof(NPNetscapeFuncs_);
    NPPluginFuncs_.size = sizeof(NPPluginFuncs_);

    NPError err = (*NP_Initialize)(&NPNetscapeFuncs_,&NPPluginFuncs_);

    if (err!=NPERR_NO_ERROR)
    {
        cerr << "NP_Initialize failed. Error #" << err << endl;
        return false;
    }

    PrintPluginFuncs();

    if (NPPluginFuncs_.newp==NULL)
    {
        cerr << "Function 'newp' must not be NULL." << endl;
        return false;
    }

    return true;
}



bool FlashPlayer::LoadFile(const char* file)
{
    DEBUG_FUNCTION_NAME

    m_File = file;
    m_Path = ExtractFilepath(m_File);

    const char *xargv[]= {
        "allowResize",
        "allowscriptaccess",
        "quality",
        "wmode",
        "allowFullScreen",
        "width",
        "height",
        "scale",
        "menu",
        //"bgcolor",
        "play",
        "loop",
        "allowFullScreenInteractive",
    };
    const char *xargm[] = {
        "true",
        "always",
        m_Attributes.quality.c_str(),
        "direct",
        "true",
        "100%",
        "100%",
        m_Attributes.scale.c_str(),
        m_Attributes.menu.c_str(),
        //"#000000",
        "true",
        "true",
        "true",
    };
	const int xargc = sizeof(xargv)/sizeof(*xargv);

	for (int arg=0; arg<xargc; ++arg)
	{
	    cout << "\t" << xargv[arg] << "=" << xargm[arg] << endl;
	}

    NPSavedData* data = new NPSavedData;
    memset(data,0,sizeof(NPSavedData));

    NPError err = (*NPPluginFuncs_.newp)(MIMETYPE_OCTET_STREAM,
                                         &NPP_,
                                         NP_EMBED,
                                         xargc,
                                         const_cast<char**>(xargv),
                                         const_cast<char**>(xargm),
                                         0);
    CHECK_ERROR_RETURN("NP_New")

    NPObject object;
    err = (*NPPluginFuncs_.getvalue)(&NPP_,NPPVpluginScriptableNPObject,&object);
    CHECK_ERROR_RETURN("NP_GetValue NPPVpluginScriptableNPObject")

    m_Window.SetDimensions(m_Attributes.window_src_width, m_Attributes.window_src_height, m_Attributes.window_trg_width, m_Attributes.window_trg_height);
    m_Window.InitializeNPWindow();

    err = (*NPPluginFuncs_.setwindow)(&NPP_,m_Window.GetNPWindow());
    CHECK_ERROR_RETURN("NP_SetWindow");


	FILE* pp = fopen(file,"rb");
    if (!pp)
    {
        cerr << "File '" << file << "' not found." << endl;
        return false;
    }

    const NPBool seekable = 0;
    uint16_t stream_type = NP_NORMAL;
    NPStream stream;
    memset(&stream,0,sizeof(stream));
    stream.url = strdup(file);

    fseek(pp, 0L, SEEK_END);
    stream.end = ftell(pp);
    fseek(pp, 0L, SEEK_SET);


	err = NPPluginFuncs_.newstream(&NPP_,MIMETYPE_SWF,&stream,seekable,&stream_type);
	CHECK_ERROR_RETURN("NP_NewStream");

    bool success = true;
	int len=0;
    char buffer[8192];
    while((len=fread(buffer, 1, sizeof(buffer), pp)) != 0 && success)
    {
        // cout << "\tlen=" << len << endl;
        int offset = 0;
        while (offset<len)
        {
            int to_write = NPPluginFuncs_.writeready(&NPP_, &stream);
            // cout << "\twrite_ready=" << to_write << endl;
            if (to_write>len) to_write = len;
            int written = NPPluginFuncs_.write(&NPP_,&stream, offset, to_write, buffer);
            // cout << "\twritten=" << written << endl;
            if (written<=0)
                break;
            offset += written;
        }
        success = offset == len;
    }
    fclose(pp);

    err = (*NPPluginFuncs_.destroystream)(&NPP_,&stream,success ? NPRES_DONE : NPRES_NETWORK_ERR);
	CHECK_ERROR_RETURN("NPN_DestroyStream");

    return true;
}

std::string FlashPlayer::GetFile() const
{
    return m_File;
}

std::string FlashPlayer::GetPath() const
{
    return m_Path;
}

const NPPluginFuncs& FlashPlayer::GetPluginFuncs() const
{
    return NPPluginFuncs_;
}

void FlashPlayer::Run()
{
    DEBUG_FUNCTION_NAME

    m_Window.Show();

    m_Window.RunLoop();
}

void FlashPlayer::Shutdown()
{
    DEBUG_FUNCTION_NAME

    NPSavedData* saved = new NPSavedData;
    NPPluginFuncs_.destroy(&NPP_,&saved);
    NP_Shutdown();

    delete saved;
}

void FlashPlayer::SendEvent( void* event_data )
{
    if (NPPluginFuncs_.event)
        NPPluginFuncs_.event(&NPP_, event_data);
}

#define PRINT_FUNC(func,var) \
    cout << #func << " " << (void*)NPPluginFuncs_. var << endl

void FlashPlayer::PrintPluginFuncs()
{
    PRINT_FUNC(NPP_NewProcPtr,newp);
    PRINT_FUNC(NPP_DestroyProcPtr,destroy);
    PRINT_FUNC(NPP_SetWindowProcPtr,setwindow);
    PRINT_FUNC(NPP_NewStreamProcPtr,newstream);
    PRINT_FUNC(NPP_DestroyStreamProcPtr,destroystream);
    PRINT_FUNC(NPP_StreamAsFileProcPtr,asfile);
    PRINT_FUNC(NPP_WriteReadyProcPtr,writeready);
    PRINT_FUNC(NPP_WriteProcPtr,write);
    PRINT_FUNC(NPP_PrintProcPtr,print);
    PRINT_FUNC(NPP_HandleEventProcPtr,event);
    PRINT_FUNC(NPP_URLNotifyProcPtr,urlnotify);
    //void*,javaClass);
    PRINT_FUNC(NPP_GetValueProcPtr,getvalue);
    PRINT_FUNC(NPP_SetValueProcPtr,setvalue);
    PRINT_FUNC(NPP_GotFocusPtr,gotfocus);
    PRINT_FUNC(NPP_LostFocusPtr,lostfocus);
    PRINT_FUNC(NPP_URLRedirectNotifyPtr,urlredirectnotify);
    PRINT_FUNC(NPP_ClearSiteDataPtr,clearsitedata);
    PRINT_FUNC(NPP_GetSitesWithDataPtr,getsiteswithdata);
    PRINT_FUNC(NPP_DidCompositePtr,didComposite);
}

#undef PRINT_FUNC


