// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------

// https://github.com/idaunis/simple-linux-flash-embed

#include "flashplayer.h"
#include "flashwindow.h"
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

#define DEBUG_FUNCTION_NAME cout << __FUNCTION__ << endl;

/* -------- */

using namespace std;

/* -------- */

FlashPlayer::FlashPlayer( FlashWindow& flash_window )
    : m_FlashPlayerLib(0)
    , m_Window(flash_window)
{
    memset(&NPPluginFuncs_,0,sizeof(NPPluginFuncs));
    memset(&NPNetscapeFuncs_,0,sizeof(NPNetscapeFuncs));
    memset(&NPP_,0,sizeof(NPP));
    //memset(&NPSavedData_,0,sizeof(NPSavedData));

    InitializeNPN(&NPNetscapeFuncs_,&NPPluginFuncs_);
}


FlashPlayer::~FlashPlayer()
{
    if (m_FlashPlayerLib) dlclose(m_FlashPlayerLib);
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



bool FlashPlayer::LoadFile(const char* file, int width, int height, bool stretch)
{
    DEBUG_FUNCTION_NAME

    char cwidth[32]; sprintf(cwidth,"%d",width);
    char cheight[32]; sprintf(cheight,"%d", height);
    char* exactFit = "exactFit";
    char* showAll = "showAll";

    char *xargv[]= {"allowResize","allowscriptaccess", "name", "quality", "wmode", "allowFullScreen", "width","height","scale"};
    char *xargm[]= {"true", "always", "test", "best", "direct","true","100%","100%",(stretch ?exactFit:showAll)};
	const int xargc = sizeof(xargv)/sizeof(*xargv);

    NPSavedData* data = new NPSavedData;
    memset(data,0,sizeof(NPSavedData));

    NPError err = (*NPPluginFuncs_.newp)(MIMETYPE_SWF,
                                         &NPP_,
                                         NP_EMBED,
                                         xargc,
                                         xargv,
                                         xargm,
                                         0);
    CHECK_ERROR_RETURN("NP_New")

    NPObject object;
    err = (*NPPluginFuncs_.getvalue)(&NPP_,NPPVpluginScriptableNPObject,&object);
    CHECK_ERROR_RETURN("NP_GetValue NPPVpluginScriptableNPObject")

    m_Window.InitializeNPWindow(width,height);
    m_Window.Show();

    err = (*NPPluginFuncs_.setwindow)(&NPP_,m_Window.GetNPWindow());
    CHECK_ERROR_RETURN("NP_SetWindow");

    uint16_t stream_type = 0;
    NPStream stream;
    memset(&stream,0,sizeof(stream));
    stream.url = strdup(file);

	err = NPPluginFuncs_.newstream(&NPP_,MIMETYPE_SWF,&stream,0/*seekable*/,&stream_type);
	CHECK_ERROR_RETURN("NP_NewStream");

	FILE *pp;
	char buffer[8192];
	pp = fopen(file,"rb");
	int len;
	while((len=fread(buffer, 1, sizeof(buffer), pp)) != 0)
	{
		NPPluginFuncs_.writeready(&NPP_, &stream);
		NPPluginFuncs_.write(&NPP_, &stream, 0, len, buffer);
	}
	fclose(pp);


    err = (*NPPluginFuncs_.destroystream)(&NPP_,&stream,NPRES_DONE);
	CHECK_ERROR_RETURN("NPN_DestroyStream");

    return true;
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

#if 0
NPStream* FlashPlayer::CreateStream(const char* file)
{
    NPStream* stream = new NPStream();
    memset(stream,0,sizeof(NPStream));

    stream->url = strdup(file);

    return stream;
}
#endif

