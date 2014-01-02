// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------

// https://github.com/idaunis/simple-linux-flash-embed
// http://code.google.com/p/screenweaver-hx/source/browse/trunk/src/np_host.c?r=64
// http://www.opensource.apple.com/source/WebCore/WebCore-955.66.1/plugins/

#include "npn.h"
#include "mimetypes.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>

using namespace std;

/* -------- */

#define USERAGENT "flashenv/1.0"

#define DEBUG_FUNCTION_NAME cout << __FUNCTION__ << endl;

/* -------- */

static NPPluginFuncs* S_PluginFuncs = NULL;

/* -------- */

#define NP_FALSE 0
#define NP_TRUE 1

#define NO_IDENTIFIER		((NPIdentifier)0)
#define	SPECIAL_IDENTIFIER  0x0FEEBBCC
#define SPECIAL_METHOD_NAME "swhxCall"

#define FLASH_REQUEST		"__flash__request"
#define FSCMD			"_DoFSCommand"
#define INVOKE_RESPONSE "<invoke name=\"%s\" returntype=\"javascript\"><arguments><null/></arguments></invoke>"



/* properties */

typedef intptr_t int_val;

static char **np_ids = NULL;
static int_val np_id_count = 0;

static NPIdentifier resolveNPId( const char *id ) {
	int_val i;
	for(i=0;i<np_id_count;i++)
		if( strcmp(np_ids[i],id) == 0 )
			return (NPIdentifier)(i+1);
	if( strcmp(id,SPECIAL_METHOD_NAME) == 0 )
		return (NPIdentifier)SPECIAL_IDENTIFIER;
	return NO_IDENTIFIER;
}

static NPIdentifier addNPId( const char *id ) {
	NPIdentifier newid = resolveNPId(id);
	if( newid == NO_IDENTIFIER ) {
		np_id_count++;
		printf("New npid added: %i == %s\n",np_id_count, id);
		np_ids = (char**)realloc(np_ids,np_id_count*sizeof(char*));
		np_ids[np_id_count-1] = strdup(id);
		return (NPIdentifier)np_id_count;
	}
	return newid;
}

static const char *getNPId( NPIdentifier id ) {
	int_val index = ((int_val)id)-1;
	if( index >= 0 && index < np_id_count )
		return np_ids[index];
	if( id == (NPIdentifier)SPECIAL_IDENTIFIER )
		return SPECIAL_METHOD_NAME;
	return NULL;
}

static int matchNPId(NPIdentifier id, const char *str) {
	const char *strid = getNPId(id);
	return ( strid != NULL && strcmp(strid,str) == 0 );
}

void freeNPIds() {
	while( np_id_count )
		free(np_ids[--np_id_count]);
	free(np_ids);
}

/* window class */

static bool NPN_InvokeProc( NPP npp, NPObject *npobj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result ) ;
static bool NPN_InvokeDefaultProc( NPP npp, NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result ) ;
static bool NPN_GetPropertyProc( NPP npp, NPObject *npobj, NPIdentifier propertyName, NPVariant *result ) ;
static bool NPN_SetPropertyProc( NPP npp, NPObject *npobj, NPIdentifier propertyName, const NPVariant *value ) ;
static bool NPN_RemovePropertyProc( NPP npp, NPObject *npobj, NPIdentifier propertyName ) ;
static bool NPN_HasPropertyProc( NPP npp, NPObject *npobj, NPIdentifier propertyName ) ;
static bool NPN_HasMethodProc( NPP npp, NPObject *npobj, NPIdentifier methodName ) ;

static NPObject *NPN_CreateObjectProc( NPP npp, NPClass *aClass );
static NPObject *NPN_RetainObjectProc( NPObject *npobj );

static NPClass __gen_class =
	{ NP_CLASS_STRUCT_VERSION
	, (NPAllocateFunctionPtr) malloc
	, (NPDeallocateFunctionPtr) free
	, 0
	, (NPHasMethodFunctionPtr) NPN_HasMethodProc
	, (NPInvokeFunctionPtr) NPN_InvokeProc
	, (NPInvokeDefaultFunctionPtr)NPN_InvokeDefaultProc
	, (NPHasPropertyFunctionPtr) NPN_HasPropertyProc
	, (NPGetPropertyFunctionPtr) NPN_GetPropertyProc
	, (NPSetPropertyFunctionPtr) NPN_SetPropertyProc
	, (NPRemovePropertyFunctionPtr) NPN_RemovePropertyProc
	};
static NPObject __window = { &__gen_class, 1 };
static NPObject __location = { &__gen_class, 1};
static NPObject __top = { &__gen_class, 1 };
static NPObject __top_location = { &__gen_class, 1 };

static void traceObjectOnCall(const char *f, NPObject *o){
	if (o == &__top) printf("DOM object 'top': %s\n",f);
	else if (o == &__window) printf("DOM object 'window': %s\n",f);
	else if (o == &__location) printf("DOM object 'location': %s\n",f);
	else if (o == &__top_location) printf("DOM object 'top.location': %s\n",f);
}


NPError NPN_GetValueProc(NPP instance, NPNVariable variable, void *ret_value)
{
    DEBUG_FUNCTION_NAME


    switch (variable)
    {
	case NPNVSupportsXEmbedBool:
        cout << "\tNPNVSupportsXEmbedBool" << endl;
		*((int*)ret_value)= NP_TRUE;
		break;
	case NPNVToolkit:
        cout << "\tNPNVToolkit" << endl;
		*((int*)ret_value)= NPNVGtk2;
		break;
	case NPNVnetscapeWindow:
        cout << "\tNPNVnetscapeWindow" << endl;
		*((int*)ret_value)= NP_TRUE;
		break;
    case NPNVWindowNPObject:
        cout << "\tNPNVWindowNPObject" << endl;
        *(NPObject**)ret_value = &__window;
        NPN_RetainObjectProc(&__window);
        break;
	default:
        cout << "\tvariable=" << variable << endl;
		*((int*)ret_value)=NP_FALSE;
		break;
	};
    return NPERR_NO_ERROR;
}
NPError NPN_SetValueProc(NPP instance, NPPVariable variable, void *value)
{
    DEBUG_FUNCTION_NAME


	switch(variable) {
		case NPPVpluginWindowBool:
			printf( "NPPVpluginWindowBool - %p\n", value);
			break;
		default:
			printf( "SetValue %i\n", variable );
			break;
	}
	return NPERR_NO_ERROR;
}

NPError NPN_GetURLNotifyProc(NPP instance, const char* url, const char* target, void* notifyData)
{
    DEBUG_FUNCTION_NAME
    cout << "\turl=" << url << endl;
    cout << "\ttarget" << target << endl;
    cout << "\tnotifyData=" << notifyData << endl;

	if (target && strlen(target)==6 && memcmp("_blank",target,6)==0)
	{
		//system_launch_url(url);
		S_PluginFuncs->urlnotify(instance,url,NPRES_DONE,notifyData);
	}
	else
	if( memcmp(url,"javascript:",11) == 0 )
	{
		NPStream s;
		uint16_t stype;
		int success;
		memset(&s,0,sizeof(NPStream));
		s.url = strdup(url);
		success = (S_PluginFuncs->newstream(instance,"text/html",&s,0,&stype) == NPERR_NO_ERROR);

		if( success )
		{
			int pos = 0;
			int size;
			char buf[256];
			sprintf(buf,"%X__flashplugin_unique__",(int_val)instance);
			size = (int)strlen(buf);
			s.end = size;
			while( pos < size ) {
				int len = S_PluginFuncs->writeready(instance,&s);
				if( len <= 0 )
					break;
				if( len > size - pos )
					len = size - pos;
				len = S_PluginFuncs->write(instance,&s,pos,len,buf+pos);
				if( len <= 0 )
					break;
				pos += len;
			}
			success = (pos == size);
		}

		S_PluginFuncs->urlnotify(instance,url,success?NPRES_DONE:NPRES_NETWORK_ERR,notifyData);
		S_PluginFuncs->destroystream(instance,&s,NPRES_DONE);
		free((void*)s.url);
	}
	else
	{
        bool success = false;
		NPStream stream;
		uint16_t stype;

		memset(&stream,0,sizeof(NPStream));
		stream.url = strdup(url);
		stream.notifyData = notifyData;

        S_PluginFuncs->newstream(instance,MIMETYPE_SWF,&stream, 0, &stype);

        if (memcmp(url,"http://",7)==0|| memcmp(url,"ftp://",6)==0)
        {
            char filename[L_tmpnam]; tmpnam(filename);
            int l = strlen(url);
            char* call = new char[128+l];
            sprintf(call,"wget -O %s %s",filename,url);
            system(call);


            FILE* f = fopen(filename,"rb");
            if (f)
            {
                int len=0;
                char buffer[8192];
                while((len=fread(buffer, 1, sizeof(buffer), f)) != 0)
                {
                    S_PluginFuncs->writeready(instance, &stream);
                    S_PluginFuncs->write(instance, &stream, 0, len, buffer);
                }
                fclose(f);
                unlink(filename);
                success = true;
            }
        }

        // If the target is non-null, the browser calls NPP_URLNotify() after it has finished loading the URL.
        // If the target is null, the browser calls NPP_URLNotify() after closing the stream by calling NPN_DestroyStream().
        if (target==NULL)
        {
            S_PluginFuncs->destroystream(instance, &stream, success?NPRES_DONE:NPRES_NETWORK_ERR);
        }


        S_PluginFuncs->urlnotify(instance, url, success?NPRES_DONE:NPRES_NETWORK_ERR, notifyData);

        free((char*)stream.url);
	}

    return NPERR_NO_ERROR;
}
NPError NPN_PostURLNotifyProc(NPP instance, const char* url, const char* window, uint32_t len, const char* buf, NPBool file, void* notifyData)
{
    DEBUG_FUNCTION_NAME
    return NPERR_NO_ERROR;
}
NPError NPN_GetURLProc(NPP instance, const char* url, const char* window)
{
    DEBUG_FUNCTION_NAME
    return NPERR_NO_ERROR;
}
NPError NPN_PostURLProc(NPP instance, const char* url, const char* window, uint32_t len, const char* buf, NPBool file)
{
    DEBUG_FUNCTION_NAME
    return NPERR_NO_ERROR;
}
NPError NPN_RequestReadProc(NPStream* stream, NPByteRange* rangeList)
{
    DEBUG_FUNCTION_NAME
    return NPERR_NO_ERROR;
}
NPError NPN_NewStreamProc(NPP instance, NPMIMEType type, const char* window, NPStream** stream)
{
    DEBUG_FUNCTION_NAME
    return NPERR_NO_ERROR;
}
int32_t NPN_WriteProc(NPP instance, NPStream* stream, int32_t len, void* buffer)
{
    DEBUG_FUNCTION_NAME
    return len;
}
NPError NPN_DestroyStreamProc(NPP instance, NPStream* stream, NPReason reason)
{
    DEBUG_FUNCTION_NAME
    return NPERR_NO_ERROR;
}
void NPN_StatusProc(NPP instance, const char* message)
{
    DEBUG_FUNCTION_NAME
}
const char* NPN_UserAgentProc(NPP instance)
{
    DEBUG_FUNCTION_NAME

    return USERAGENT;
}
void* NPN_MemAllocProc(uint32_t size)
{
    DEBUG_FUNCTION_NAME
    cout << "\tsize=" << size << endl;
    return malloc(size);
}
void NPN_MemFreeProc(void* ptr)
{
    DEBUG_FUNCTION_NAME
    return free(ptr);
}
uint32_t NPN_MemFlushProc(uint32_t size)
{
    DEBUG_FUNCTION_NAME

    return size;
}
void NPN_ReloadPluginsProc(NPBool reloadPages)
{
    DEBUG_FUNCTION_NAME
}
void* NPN_GetJavaEnvProc(void)
{
    DEBUG_FUNCTION_NAME
    return NULL;
}
void* NPN_GetJavaPeerProc(NPP instance)
{
    DEBUG_FUNCTION_NAME
    return NULL;
}
void NPN_InvalidateRectProc(NPP instance, NPRect *rect)
{
    DEBUG_FUNCTION_NAME
}
void NPN_InvalidateRegionProc(NPP instance, NPRegion region)
{
    DEBUG_FUNCTION_NAME
}
void NPN_ForceRedrawProc(NPP instance)
{
    DEBUG_FUNCTION_NAME
}
NPIdentifier NPN_GetStringIdentifierProc(const NPUTF8* name)
{
    DEBUG_FUNCTION_NAME

    return addNPId(name);
}
void NPN_GetStringIdentifiersProc(const NPUTF8** names, int32_t nameCount, NPIdentifier* identifiers)
{
    DEBUG_FUNCTION_NAME
}
NPIdentifier NPN_GetIntIdentifierProc(int32_t intid)
{
    DEBUG_FUNCTION_NAME
    return NO_IDENTIFIER;
}
bool NPN_IdentifierIsStringProc(NPIdentifier identifier)
{
    DEBUG_FUNCTION_NAME
    return getNPId(identifier) != NULL;
}
NPUTF8* NPN_UTF8FromIdentifierProc(NPIdentifier identifier)
{
    DEBUG_FUNCTION_NAME
    const char *result = getNPId(identifier);
	return result ? strdup(result) : NULL;
}
int32_t NPN_IntFromIdentifierProc(NPIdentifier identifier)
{
    DEBUG_FUNCTION_NAME
    return 0;
}
NPObject* NPN_CreateObjectProc(NPP npp, NPClass *aClass)
{
    DEBUG_FUNCTION_NAME

	NPObject *o;
	if( aClass->allocate )
		o = aClass->allocate(npp,aClass);
	else
		o = (NPObject*)malloc(sizeof(NPObject));
	o->_class = aClass;
	o->referenceCount = 1;
	return o;
}
NPObject* NPN_RetainObjectProc(NPObject *obj)
{
    DEBUG_FUNCTION_NAME
	if( obj == NULL )
		return NULL;
	obj->referenceCount++;
	return obj;
}
void NPN_ReleaseObjectProc(NPObject *obj)
{
    DEBUG_FUNCTION_NAME
	if( obj == NULL )
		return;
	obj->referenceCount--;
	if( obj->referenceCount != 0 )
		return;
	if( obj->_class->deallocate ) {
		obj->_class->deallocate(obj);
		return;
	}
	if( obj->_class->invalidate )
		obj->_class->invalidate(obj);
	free(obj);
}
bool NPN_InvokeProc(NPP npp, NPObject* npobj, NPIdentifier npid, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    DEBUG_FUNCTION_NAME
    cout << "\tnpid=" << getNPId(npid) << endl;
    cout << "\targc=" << argCount << endl;

	traceObjectOnCall(__FUNCTION__,npobj);

   if (npobj == &__window)
   {
       if( matchNPId(npid,"location") )
       {
            result->type = NPVariantType_Object;
            result->value.objectValue = &__location;
            NPN_RetainObjectProc(&__location);
            return 1;
        }
        if( matchNPId(npid,"top") ) {
            result->type = NPVariantType_Object;
            result->value.objectValue = &__top;
            NPN_RetainObjectProc(&__top);
            //SCRIPT_TRACE("Returned 'top' class");
            return 1;
        }
    }
    else
    if (npobj == &__top)
    {
        if ( matchNPId(npid,"location") )
        {
            result->type = NPVariantType_Object;
            result->value.objectValue = &__top_location;
            NPN_RetainObjectProc(&__top_location);
            //SCRIPT_TRACE("Returned 'top.location' class");
            return 1;
        }
	}

	if( matchNPId(npid,FLASH_REQUEST) && argCount == 1 && args[0].type == NPVariantType_String ) {
		return 1;
	}
	if( matchNPId(npid,FLASH_REQUEST) && argCount == 3 &&
		args[0].type == NPVariantType_String &&
		args[1].type == NPVariantType_String &&
		args[2].type == NPVariantType_String
	) {
		return 1;
	}

	if( matchNPId(npid,"_DoFSCommand") && argCount == 2 && args[0].type == NPVariantType_String && args[1].type == NPVariantType_String ) {
		printf("[D] FSCOMMAND\n");
		return 1;
	}

	if( npobj == &__top_location )
	{
		if( matchNPId(npid,"toString") )
		{
			result->type = NPVariantType_String;
			// "chrome://global/content/console.xul" is what Firefox returns for 'top.location.toString()';
			//result->value.stringValue.UTF8Characters = strdup("chrome://global/content/console.xul");
			result->value.stringValue.UTF8Characters = strdup("localhost");
			result->value.stringValue.UTF8Length = (int)strlen(result->value.stringValue.UTF8Characters);
			printf("[D] Returned %s\n", result->value.stringValue.UTF8Characters);
		}
		return 1;
	}
	//On OSX, Flash retreives locations by injected functions:
	if( matchNPId(npid,"__flash_getWindowLocation") ) {
		// return the location object:
		result->type = NPVariantType_Object;
		result->value.objectValue = &__location;
		NPN_RetainObjectProc(&__location);
		return 1;
	}
	if( matchNPId(npid,"__flash_getTopLocation") ) {
		// return the top_location object:
		result->type = NPVariantType_Object;
		result->value.objectValue = &__top_location;
		NPN_RetainObjectProc(&__top_location);
		return 1;
	}

    return 0;
}
bool NPN_InvokeDefaultProc(NPP npp, NPObject* obj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    DEBUG_FUNCTION_NAME
    return false;
}
bool NPN_EvaluateProc(NPP npp, NPObject *obj, NPString *script, NPVariant *result)
{
    DEBUG_FUNCTION_NAME
    return false;
}
bool NPN_GetPropertyProc(NPP npp, NPObject *npobj, NPIdentifier npid, NPVariant *result)
{
    DEBUG_FUNCTION_NAME


	if (npobj == &__window) {
		if( matchNPId(npid,"location") ) {
			result->type = NPVariantType_Object;
			result->value.objectValue = &__location;
			NPN_RetainObjectProc(&__location);
			return 1;
		}
		if( matchNPId(npid,"top") ) {
			result->type = NPVariantType_Object;
			result->value.objectValue = &__top;
			NPN_RetainObjectProc(&__top);
			return 1;
		}
	} else if (npobj == &__top) {
		if ( matchNPId(npid,"location") ) {
			result->type = NPVariantType_Object;
			result->value.objectValue = &__top_location;
			NPN_RetainObjectProc(&__top_location);
			return 1;
		}
	}
	return 0;
}
bool NPN_SetPropertyProc(NPP npp, NPObject *obj, NPIdentifier propertyName, const NPVariant *value)
{
    DEBUG_FUNCTION_NAME
    return false;
}
bool NPN_RemovePropertyProc(NPP npp, NPObject *obj, NPIdentifier propertyName)
{
    DEBUG_FUNCTION_NAME
    return false;
}
bool NPN_HasPropertyProc(NPP npp, NPObject *obj, NPIdentifier propertyName)
{
    DEBUG_FUNCTION_NAME
    return false;
}
bool NPN_HasMethodProc(NPP npp, NPObject *obj, NPIdentifier propertyName)
{
    DEBUG_FUNCTION_NAME
    return false;
}
void NPN_ReleaseVariantValueProc(NPVariant *variant)
{
    DEBUG_FUNCTION_NAME

	switch( variant->type ) {
		case NPVariantType_Null:
		case NPVariantType_Void:
			break;
		case NPVariantType_String:
			free( (char*)variant->value.stringValue.UTF8Characters );
			variant->type = NPVariantType_Void;
			break;
		case NPVariantType_Object:
			NPN_ReleaseObjectProc(variant->value.objectValue);
			variant->type = NPVariantType_Void;
			break;
		default:
			break;
	}
}
void NPN_SetExceptionProc(NPObject *obj, const NPUTF8 *message)
{
    DEBUG_FUNCTION_NAME
}
void NPN_PushPopupsEnabledStateProc(NPP npp, NPBool enabled)
{
    DEBUG_FUNCTION_NAME
}
void NPN_PopPopupsEnabledStateProc(NPP npp)
{
    DEBUG_FUNCTION_NAME
}
bool NPN_EnumerateProc(NPP npp, NPObject *obj, NPIdentifier **identifier, uint32_t *count)
{
    DEBUG_FUNCTION_NAME
    return false;
}
void NPN_PluginThreadAsyncCallProc(NPP instance, void (*func)(void *), void *userData)
{
    DEBUG_FUNCTION_NAME
}
bool NPN_ConstructProc(NPP npp, NPObject* obj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    DEBUG_FUNCTION_NAME
    return false;
}
NPError NPN_GetValueForURL(NPP npp, NPNURLVariable variable, const char *url, char **value, uint32_t *len)
{
    DEBUG_FUNCTION_NAME
    return NPERR_GENERIC_ERROR;
}
NPError NPN_SetValueForURL(NPP npp, NPNURLVariable variable, const char *url, const char *value, uint32_t len)
{
    DEBUG_FUNCTION_NAME
    return NPERR_GENERIC_ERROR;
}
NPError NPN_GetAuthenticationInfo(NPP npp, const char *protocol, const char *host, int32_t port, const char *scheme, const char *realm, char **username, uint32_t *ulen, char **password, uint32_t *plen)
{
    DEBUG_FUNCTION_NAME
    return NPERR_GENERIC_ERROR;
}
uint32_t NPN_ScheduleTimer(NPP instance, uint32_t interval, NPBool repeat, void (*timerFunc)(NPP npp, uint32_t timerID))
{
    DEBUG_FUNCTION_NAME
    return 0;
}
void NPN_UnscheduleTimer(NPP instance, uint32_t timerID)
{
    DEBUG_FUNCTION_NAME
}
NPError NPN_PopUpContextMenu(NPP instance, NPMenu* menu)
{
    DEBUG_FUNCTION_NAME
    return NPERR_GENERIC_ERROR;
}
NPBool NPN_ConvertPoint(NPP instance, double sourceX, double sourceY, NPCoordinateSpace sourceSpace, double *destX, double *destY, NPCoordinateSpace destSpace)
{
    DEBUG_FUNCTION_NAME
    return NP_FALSE;
}
NPBool NPN_HandleEvent(NPP instance, void *event, NPBool handled)
{
    DEBUG_FUNCTION_NAME
    return NP_FALSE;
}
NPBool NPN_UnfocusInstance(NPP instance, NPFocusDirection direction)
{
    DEBUG_FUNCTION_NAME
    return NP_FALSE;
}
void NPN_URLRedirectResponse(NPP instance, void* notifyData, NPBool allow)
{
    DEBUG_FUNCTION_NAME
}
NPError NPN_InitAsyncSurface(NPP instance, NPSize *size, NPImageFormat format, void *initData, NPAsyncSurface *surface)
{
    DEBUG_FUNCTION_NAME
    return NPERR_GENERIC_ERROR;
}
NPError NPN_FinalizeAsyncSurface(NPP instance, NPAsyncSurface *surface)
{
    DEBUG_FUNCTION_NAME
    return NPERR_GENERIC_ERROR;
}
void NPN_SetCurrentAsyncSurface(NPP instance, NPAsyncSurface *surface, NPRect *changed)
{
    DEBUG_FUNCTION_NAME
}

/* -------- */


void InitializeNPN(NPNetscapeFuncs* funcs, NPPluginFuncs* plugin_funcs)
{
    memset(funcs,0,sizeof(NPNetscapeFuncs));
    funcs->size = sizeof(NPNetscapeFuncs);
    funcs->version = 20; //????

    funcs->geturl = NPN_GetURLProc;
    funcs->posturl = NPN_PostURLProc;
    funcs->requestread = NPN_RequestReadProc;
    funcs->newstream =  NPN_NewStreamProc;
    funcs->write =  NPN_WriteProc;
    funcs->destroystream = NPN_DestroyStreamProc;
    funcs->status = NPN_StatusProc;
    funcs->uagent = NPN_UserAgentProc;
    funcs->memalloc = NPN_MemAllocProc;
    funcs->memfree = NPN_MemFreeProc;
    funcs->memflush = NPN_MemFlushProc;
    funcs->reloadplugins = NPN_ReloadPluginsProc;
    funcs->getJavaEnv = NPN_GetJavaEnvProc;
    funcs->getJavaPeer = NPN_GetJavaPeerProc;
    funcs->geturlnotify = NPN_GetURLNotifyProc;
    funcs->posturlnotify = NPN_PostURLNotifyProc;
    funcs->getvalue = NPN_GetValueProc;
    funcs->setvalue = NPN_SetValueProc;
    funcs->invalidaterect = NPN_InvalidateRectProc;
    funcs->invalidateregion = NPN_InvalidateRegionProc;
    funcs->forceredraw = NPN_ForceRedrawProc;
    funcs->getstringidentifier = NPN_GetStringIdentifierProc;
    funcs->getstringidentifiers = NPN_GetStringIdentifiersProc;
    funcs->getintidentifier = NPN_GetIntIdentifierProc;
    funcs->identifierisstring = NPN_IdentifierIsStringProc;
    funcs->utf8fromidentifier = NPN_UTF8FromIdentifierProc;
    funcs->intfromidentifier = NPN_IntFromIdentifierProc;
    funcs->createobject = NPN_CreateObjectProc;
    funcs->retainobject = NPN_RetainObjectProc;
    funcs->releaseobject = NPN_ReleaseObjectProc;
    funcs->invoke = NPN_InvokeProc;
    funcs->invokeDefault = NPN_InvokeDefaultProc;
    funcs->evaluate = NPN_EvaluateProc;
    funcs->getproperty = NPN_GetPropertyProc;
    funcs->setproperty = NPN_SetPropertyProc;
    funcs->removeproperty = NPN_RemovePropertyProc;
    funcs->hasproperty = NPN_HasPropertyProc;
    funcs->hasmethod = NPN_HasMethodProc;
    funcs->releasevariantvalue = NPN_ReleaseVariantValueProc;
    funcs->setexception = NPN_SetExceptionProc;
    funcs->pushpopupsenabledstate = NPN_PushPopupsEnabledStateProc;
    funcs->poppopupsenabledstate = NPN_PopPopupsEnabledStateProc;
    funcs->enumerate = NPN_EnumerateProc;
    funcs->pluginthreadasynccall = NPN_PluginThreadAsyncCallProc;
    funcs->construct = NPN_ConstructProc;
    funcs->getvalueforurl = NPN_GetValueForURL;
    funcs->setvalueforurl = NPN_SetValueForURL;
    funcs->getauthenticationinfo = NPN_GetAuthenticationInfo;
    funcs->scheduletimer = NPN_ScheduleTimer;
    funcs->unscheduletimer = NPN_UnscheduleTimer;
    funcs->popupcontextmenu = NPN_PopUpContextMenu;
    funcs->convertpoint = NPN_ConvertPoint;
    funcs->handleevent = NPN_HandleEvent;
    funcs->unfocusinstance = NPN_UnfocusInstance;
    funcs->urlredirectresponse = NPN_URLRedirectResponse;
    funcs->initasyncsurface = NPN_InitAsyncSurface;
    funcs->finalizeasyncsurface = NPN_FinalizeAsyncSurface;
    funcs->setcurrentasyncsurface = NPN_SetCurrentAsyncSurface;

    S_PluginFuncs = plugin_funcs;
}
