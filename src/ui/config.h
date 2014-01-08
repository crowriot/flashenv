// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------


#ifndef CONFIG_H
#define CONFIG_H

class FileStat;

/// save config file
bool SaveConfig( const FileStat& swf );

/// load config file
bool LoadConfig( FileStat* file_stat );

#endif

