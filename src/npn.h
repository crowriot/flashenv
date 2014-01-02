// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------

#ifndef NPN_H
#define NPN_H


#include "npapi/npfunctions.h"


/// initializes the function pointers of the given NPNetscapeFuncs
/// instance and sets the global plugin function struct
void InitializeNPN( NPNetscapeFuncs* functions, NPPluginFuncs* plugin_functions );



#endif
