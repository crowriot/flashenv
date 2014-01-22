// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------


#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include "widget.h"
#include "filelist.h"

extern "C" {
#include "../third_party/iniparser-3.0/src/iniparser.h"
}


/// base class for value edit widgets
class EditWidget : public Widget
{
public:
    /// load value from ini dictionary
    virtual void Load( dictionary* dict, char* inikey );
    /// save value to ini dictionary
    virtual void Save( dictionary* dict, char* inikey );
    /// key handling
    virtual bool OnKeyDown(const SDL_Event& key_event);
};

/// key mapping widget
class KeyMappingEditWidget : public EditWidget
{
public:
    KeyMappingEditWidget();

    /** overrides */
    virtual void Load( dictionary* dict, char* key );
    virtual void Save( dictionary* dict, char* key );
    virtual bool OnKeyDown(const SDL_Event& key_event);
    virtual std::string GetText() const;

private:
    std::vector<std::string> m_Keys;
};


/// range edit: edit value between a specified range
class RangeEditWidget : public EditWidget
{
public:
    RangeEditWidget( int min, int max, int def );

    /** overrides */
    virtual void Load( dictionary* dict, const char* key );
    virtual std::string GetText() const;
    virtual bool OnKeyDown(const SDL_Event& key_event);

private:
    int m_Min;
    int m_Max;
    int m_Reset;
    int m_Current;
    std::string m_InputText;
    bool m_TextInput;
};


/// SetEdit: allows selection of a predefined string value
class SetEditWidget : public EditWidget
{
public:
    /// constructor
    SetEditWidget();
    /// add a value to the set
    void AddValue(const char* value, bool is_default=false);


    /** overrides */
    virtual void Load( dictionary* dict, const char* inikey );
    virtual std::string GetText() const;
    virtual bool OnKeyDown(const SDL_Event& key_event);

private:
    std::vector<std::string> m_Set;
    int m_CurrentValue;
    int m_DefaultValue;
    int m_ResetValue;
};


/// game config entry class
class GameConfigValue
{
public:
    EditWidget* (*CreateFN)();
    const char* ini_name;
    const char* display_name;
};


/// single value config widget
class GameConfigWidget
{
public:
    GameConfigWidget();
    ~GameConfigWidget();

    void Init(TTF_Font* font, int x, int y, const GameConfigValue& value);
    EditWidget* GetEdit() { return m_EditWidget; }
    void SetSelected(bool);
    bool GetSelected() const;
    bool OnKeyDown(const SDL_Event& keyevent);
    void BlitTo(SDL_Surface* screen);

private:
    TextWidget m_LabelWidget;
    EditWidget* m_EditWidget;
    bool m_Active;
};


/// game config widget
class GameConfigWindow
{
public:
    /// constructor
    GameConfigWindow( TTF_Font* font );
    /// destructor
    ~GameConfigWindow();

    /// loads the gameconfig.ini if available
    void LoadINI();
    /// save changes to gameconfig.ini
    void SaveINI();

    /// set the file to edit the config for
    void ConfigFile( const FileStat& file );


    /// show the configg
    void Show();
    void Hide();
    bool IsShown() const;

    /** sdl related */

    /// key event handler
    bool OnKeyDown(const SDL_Event& event);

    /// draw it...
    void BlitTo(SDL_Surface* screen);

protected:
    void UpdateCurrentConfig(int dir);
    GameConfigWidget& GetCurrentConfig();


private:
    TTF_Font* m_Font;
    bool m_Visible;
    FileStat m_Swf;
    dictionary* m_Dict;
    SDL_Rect m_DrawRect;
    std::vector<GameConfigWidget> m_ConfigWidgets;
    int m_CurrentConfig;
};



#endif

