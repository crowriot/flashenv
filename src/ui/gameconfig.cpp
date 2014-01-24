// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------


#include "gameconfig.h"
#include "defines.h"
#include "../player/defines.h"
#include <SDL/SDL_gfxPrimitives.h>
#include <iostream>
#include <algorithm>

EditWidget* CreateKeyEdit()
{
    return new KeyMappingEditWidget;
}

EditWidget* CreateWinWidthEdit()
{
    return new RangeEditWidget(0,SCREENWIDTH,SCREENWIDTH);
}

EditWidget* CreateWinHeightEdit()
{
    return new RangeEditWidget(0,SCREENHEIGHT,SCREENHEIGHT);
}

EditWidget* CreateFbWidthEdit()
{
    return new RangeEditWidget(0,SCREENWIDTH,0);
}

EditWidget* CreateFbHeightEdit()
{
    return new RangeEditWidget(0,SCREENHEIGHT,0);
}


EditWidget* CreateBoolEdit()
{
    SetEditWidget* set_widget = new SetEditWidget();
    set_widget->AddValue("yes",true);
    set_widget->AddValue("no");
    return set_widget;
}

EditWidget* CreateScaleEdit()
{
    SetEditWidget* set_widget = new SetEditWidget();
    set_widget->AddValue("showall",true);
    set_widget->AddValue("noborder");
    set_widget->AddValue("exactfit");
    set_widget->AddValue("noscale");
    return set_widget;
}

EditWidget* CreateQualityEdit()
{
    SetEditWidget* set_widget = new SetEditWidget();
    set_widget->AddValue("low");
    set_widget->AddValue("autolow",true);
    set_widget->AddValue("autohigh");
    set_widget->AddValue("medium");
    set_widget->AddValue("high");
    set_widget->AddValue("best");
    return set_widget;
}


static const GameConfigValue C_GameConfigs[] =
{
    { &CreateKeyEdit, "DPAD_Left"  , "DPad Left" },
    { &CreateKeyEdit, "DPAD_Right" , "DPad Right" },
    { &CreateKeyEdit, "DPAD_Up"    , "DPad Up" },
    { &CreateKeyEdit, "DPAD_Down"  , "DPad Down" },

    { &CreateKeyEdit, "DPAD_A" , "DPad A" },
    { &CreateKeyEdit, "DPAD_B" , "DPad B" },
    { &CreateKeyEdit, "DPAD_X" , "DPad X" },
    { &CreateKeyEdit, "DPAD_Y" , "DPad Y" },

    { &CreateKeyEdit, "Start"  , "Start" },
    { &CreateKeyEdit, "Select" , "Select" },

    { &CreateKeyEdit, "Trigger_Left"  , "Left Shoulder" },
    { &CreateKeyEdit, "Trigger_Right" , "Right Shoulder" },

    { &CreateWinWidthEdit, "WindowWidth"  , "Window Width" },
    { &CreateWinHeightEdit, "WindowHeight" , "Window Height" },

    { &CreateFbWidthEdit, "FramebufferWidth"  , "Framebuffer Width" },
    { &CreateFbHeightEdit, "FramebufferHeight" , "Framebuffer Height" },

    { &CreateScaleEdit  , "scale"   , "Flash Scale Mode" },
    { &CreateBoolEdit   , "menu"    , "Flash Menu" },
    { &CreateQualityEdit, "quality" , "Flash Quality", }
};

static const int C_GameConfigCount = sizeof(C_GameConfigs)/sizeof(*C_GameConfigs);

/* -------- */

template <class T, class B>
void AddClamp(T& val, T add, B min, B max)
{
    val += add;

    if (val<min) { val=min; }
    if (val>max) { val=max; }
}


template <class T, class B>
void AddWrap(T& val, T add, B size)
{
    val += add;
    if (val<0) val = size -1;
    else val %= size;
}


static bool IsAcceptKey(const SDL_Event& event)
{
    return event.key.keysym.sym==SDLK_RETURN
         || event.key.keysym.sym==SDLK_PANDORA_A
         || event.key.keysym.sym==SDLK_PANDORA_B
         || event.key.keysym.sym==SDLK_PANDORA_X
         || event.key.keysym.sym==SDLK_PANDORA_Y;
}

static bool IsCancelKey(const SDL_Event& event)
{
    return event.key.keysym.sym==SDLK_ESCAPE;
}

/* -------- */

void EditWidget::Load( dictionary* dict, char* inikey )
{

}

void EditWidget::Save( dictionary* dict, char* inikey )
{

}

bool EditWidget::OnKeyDown(const SDL_Event& event)
{
    if (GetSelected()==false)
    {
        if (IsAcceptKey(event))
        {
            SetSelected(true);
            return true;
        }
    }
    else
    {
        if (IsAcceptKey(event))
        {
            SetSelected(false);
        }
        else
        if (IsCancelKey(event))
        {
            SetSelected(false);
        }
        return true;
    }
    return false;
}

KeyMappingEditWidget::KeyMappingEditWidget()
{

}

void KeyMappingEditWidget::Load(dictionary* dict, char* key)
{
    m_Keys.clear();

    char* value = iniparser_getstring(dict, key, "");
    if (value && value[0])
    {
        char* mapping = strtok(value, " ");
        if (!mapping)
            mapping = value;

        do
        {
            m_Keys.push_back(std::string(mapping,strlen(mapping)));
        }
        while(mapping=strtok(NULL," "));
    }

    m_Reset = m_Keys;
}

void KeyMappingEditWidget::Save( dictionary* dict, char* inikey )
{
    if (m_Keys.size())
    {
        std::string val = GetText();
        iniparser_set(dict, const_cast<char*>(inikey), const_cast<char*>(val.c_str()));
    }
    else
    {
        iniparser_unset(dict, const_cast<char*>(inikey));
    }
}


std::string KeyMappingEditWidget::GetText() const
{
    if (m_Keys.size())
    {
        std::string keys;
        for (size_t i=0; i<m_Keys.size(); ++i)
        {
            if (i) keys += " ";
            keys += m_Keys[i];
        }
        return keys;
    }
    return "<unassigned>";
}

bool KeyMappingEditWidget::OnKeyDown(const SDL_Event& event)
{
    if (GetSelected())
    {
        if (event.key.keysym.sym==SDLK_BACKSPACE)
        {
            if (m_Keys.size()) m_Keys.erase(m_Keys.begin()+m_Keys.size()-1);
        }
        else
        if (IsAcceptKey(event))
        {
            SetSelected(false);
            m_Reset = m_Keys;
        }
        else
        if (IsCancelKey(event))
        {
            SetSelected(false);
            m_Keys = m_Reset;
        }
        else
        if (event.key.keysym.sym<=127)
        {
            const char* keyname = SDL_GetKeyName(event.key.keysym.sym);
            if (keyname)
                m_Keys.push_back(keyname);
        }

        return true;
    }

    return EditWidget::OnKeyDown(event);
}


/* -------- */

RangeEditWidget::RangeEditWidget( int min, int max, int def )
    : m_Min(min)
    , m_Max(max)
    , m_Current(def)
    , m_Default(def)
    , m_Reset(def)
    , m_TextInput(false)
{

}

void RangeEditWidget::Load(dictionary* dict, char* key)
{
    int value = iniparser_getint(dict,key,m_Default);
    AddClamp(value,0,m_Min,m_Max);
    m_Reset = value;
}

void RangeEditWidget::Save(dictionary* dict, char* key)
{
    if (m_Current!=m_Default)
    {
        char tmp[256]; sprintf(tmp,"%d",m_Current);
        iniparser_set(dict, key, tmp);
    }
    else
    {
        iniparser_unset(dict,key);
    }
}

std::string RangeEditWidget::GetText() const
{
    if (m_TextInput)
        return m_InputText+"_";

    char tmp[64];
    sprintf(tmp,"%d",m_Current);
    return tmp;
}

bool RangeEditWidget::OnKeyDown(const SDL_Event& event)
{
    if (GetSelected())
    {
        int scale = 10;
        if (event.key.keysym.mod & KMOD_SHIFT) scale = 1;
        if (event.key.keysym.mod & KMOD_CTRL) scale = 100;

        if (event.key.keysym.sym==SDLK_DOWN)
        {
            AddClamp(m_Current,-1*scale,m_Min,m_Max);
            return true;
        }
        else
        if (event.key.keysym.sym==SDLK_UP)
        {
            AddClamp(m_Current,+1*scale,m_Min,m_Max);
            return true;
        }
        else
        if (event.key.keysym.sym>='0' && event.key.keysym.sym<='9')
        {
            m_TextInput = true;
            m_InputText += (char)event.key.keysym.sym;
        }
        else
        if (m_TextInput)
        {
            if (event.key.keysym.sym==SDLK_BACKSPACE)
            {
                if (m_InputText.size()) m_InputText.erase(m_InputText.size()-1);
                return true;
            }
            else
            if (IsAcceptKey(event))
            {
                m_Current = atoi(m_InputText.c_str());
                AddClamp(m_Current,0,m_Min,m_Max);
                m_Reset = m_Current;
                m_InputText.clear();
                m_TextInput = false;
            }
            else
            if (IsCancelKey(event))
            {
                m_TextInput = false;
                m_InputText.clear();
                return true;
            }
        }
        else
        if (event.key.keysym.sym==SDLK_BACKSPACE)
        {
            m_InputText = GetText();
            m_TextInput = true;
            return true;
        }
        else
        if (IsCancelKey(event))
        {
            m_Current = m_Reset;
        }
        else
        if (IsAcceptKey(event))
        {
            m_Reset = m_Current;
        }
    }

    return EditWidget::OnKeyDown(event);
}

/* -------- */

SetEditWidget::SetEditWidget()
    : m_CurrentValue(0)
    , m_ResetValue(0)
    , m_DefaultValue(0)
{

}

void SetEditWidget::AddValue(const char* value, bool is_default)
{
    m_Set.push_back(value);

    if (is_default)
    {
        m_DefaultValue = m_Set.size()-1;
        m_ResetValue = m_DefaultValue;
        m_CurrentValue = m_DefaultValue;
    }
}

void SetEditWidget::Load( dictionary* dict, char* key )
{
    m_CurrentValue = m_DefaultValue;
    m_ResetValue = m_DefaultValue;

    std::string defvalue = m_Set[m_DefaultValue];
    char* value = iniparser_getstring(dict,key,const_cast<char*>(defvalue.c_str()));
    if (value && value[0])
    {
        std::vector<std::string>::iterator it = std::find(m_Set.begin(),m_Set.end(), std::string(value));
        if (it!=m_Set.end())
        {
            m_CurrentValue = it - m_Set.begin();
            m_ResetValue = m_CurrentValue;
        }
    }
}
void SetEditWidget::Save( dictionary* dict, char* key )
{
    std::string value = m_Set[m_CurrentValue];
    if (m_CurrentValue!=m_DefaultValue)
        iniparser_set(dict,key,const_cast<char*>(value.c_str()));
    else
        iniparser_unset(dict,key);
}

std::string SetEditWidget::GetText() const
{
    return m_Set[m_CurrentValue];
}

bool SetEditWidget::OnKeyDown(const SDL_Event& event)
{
    if (GetSelected())
    {
        if (event.key.keysym.sym==SDLK_DOWN)
        {
            AddWrap(m_CurrentValue,-1,m_Set.size());
        }
        else
        if (event.key.keysym.sym==SDLK_UP)
        {
            AddWrap(m_CurrentValue,+1,m_Set.size());
        }
        else
        if (IsCancelKey(event))
        {
            m_CurrentValue = m_ResetValue;
            SetSelected(false);
        }
        else
        if (IsAcceptKey(event))
        {
            SetSelected(false);
            m_ResetValue = m_CurrentValue;
        }
        else
        {
            return EditWidget::OnKeyDown(event);
        }

        return true;
    }

    return EditWidget::OnKeyDown(event);
}

/* -------- */

GameConfigWidget::GameConfigWidget()
    : m_Active(false)
    , m_EditWidget(NULL)
{
}

GameConfigWidget::~GameConfigWidget()
{
}

void GameConfigWidget::Init(TTF_Font* font, int x, int y, const GameConfigValue& value)
{
    m_LabelWidget.SetFont(font);
    m_LabelWidget.SetText(value.display_name);

    SDL_Rect lrect = {x, y,CONFIGWIDTH/2,WIDGETHEIGHT};
    m_LabelWidget.SetRect(lrect);

    m_EditWidget = (*value.CreateFN)();
    m_EditWidget->SetFont(font);
    //m_EditWidget->SetText("<value>");

    SDL_Rect vrect = {x+CONFIGEDITOFFSET, y,CONFIGWIDTH-CONFIGEDITOFFSET,WIDGETHEIGHT};
    m_EditWidget->SetRect(vrect);
}

void GameConfigWidget::SetSelected(bool sel)
{
    m_LabelWidget.SetSelected(sel);
}

bool GameConfigWidget::GetSelected() const
{
    return m_LabelWidget.GetSelected();
}

bool GameConfigWidget::IsEditing() const
{
    return m_EditWidget->GetSelected();
}

bool GameConfigWidget::OnKeyDown(const SDL_Event& key_event)
{
    return m_EditWidget->OnKeyDown(key_event);
}

void GameConfigWidget::BlitTo(SDL_Surface* screen)
{
    m_LabelWidget.BlitTo(screen);
    m_EditWidget->BlitTo(screen);
}

/* -------- */

GameConfigWindow::GameConfigWindow( TTF_Font* font )
    : m_Font(font)
    , m_Visible(false)
    , m_CurrentConfig(0)
    , m_Dict(0)
{
    m_DrawRect.x = (SCREENWIDTH-CONFIGWIDTH)/2;
    m_DrawRect.y = CONFIGOFFSETY;
    m_DrawRect.w = CONFIGWIDTH;
    m_DrawRect.h = CONFIGHEIGHT;

    for (int i=0;i<C_GameConfigCount;++i)
    {
        GameConfigWidget widget;
        widget.Init(font,m_DrawRect.x+CONFIGBORDER,m_DrawRect.y+i*CONFIGLINEHEIGHT+CONFIGBORDER,C_GameConfigs[i]);

        m_ConfigWidgets.push_back(widget);
    }
}


GameConfigWindow::~GameConfigWindow()
{
    if (m_Dict) iniparser_freedict(m_Dict);
}


void GameConfigWindow::LoadINI()
{
    m_Dict = iniparser_load(GAMECONFIG_INI_FILE);
    if (m_Dict==0) m_Dict = dictionary_new(1);
}

void GameConfigWindow::SaveINI()
{
    if (m_Dict)
    {
        FILE* f = fopen(GAMECONFIG_INI_FILE,"w");
        iniparser_dump_ini(m_Dict,f);
        fclose(f);
    }
}

void GameConfigWindow::ConfigFile(const FileStat& file)
{
    if (file.IsFlashFile())
    {
        m_Swf = file;
        Show();
    }
}

void GameConfigWindow::Show()
{
    bool is_visible = m_Visible;
    m_Visible = true;
    m_CurrentConfig = 0;
    UpdateCurrentConfig(0);

    if (is_visible!=m_Visible)
    {
        for (size_t i=0; i<m_ConfigWidgets.size();++i)
        {
            const GameConfigValue& cfg_value = C_GameConfigs[i];
            char tmp[1024]; sprintf(tmp,"%s:%s",m_Swf.GetName().c_str(), cfg_value.ini_name);
            m_ConfigWidgets[i].GetEdit()->Load(m_Dict,tmp);
        }
    }
}

void GameConfigWindow::Hide()
{
    m_Visible = false;

    if (!m_Swf.IsFlashFile())
        return;

    if (!m_Dict) m_Dict = dictionary_new(0);

    std::string section = m_Swf.GetName();
    iniparser_set(m_Dict,const_cast<char*>(section.c_str()),NULL);

    for (size_t i=0; i<m_ConfigWidgets.size();++i)
    {
        const GameConfigValue& cfg_value = C_GameConfigs[i];
        char tmp[1024]; sprintf(tmp,"%s:%s",m_Swf.GetName().c_str(), cfg_value.ini_name);
        m_ConfigWidgets[i].GetEdit()->Save(m_Dict,tmp);
    }
}

bool GameConfigWindow::IsShown() const
{
    return m_Visible;
}



bool GameConfigWindow::OnKeyDown(const SDL_Event& event)
{
    bool eaten = GetCurrentConfig().OnKeyDown(event);
    if (eaten)
    {
        return true;
    }
    else
    if (event.key.keysym.sym==SDLK_ESCAPE)
    {
        Hide();
        SaveINI();
        return true;
    }
    else
    if (event.key.keysym.sym==SDLK_UP)
    {
        UpdateCurrentConfig(-1);
        return true;
    }
    else
    if (event.key.keysym.sym==SDLK_DOWN)
    {
        UpdateCurrentConfig(+1);
        return true;
    }

    return false;
}

void GameConfigWindow::BlitTo(SDL_Surface* screen)
{
    if (!m_Visible)
        return;

// shadow
    SDL_Rect shadow_target = m_DrawRect;
    shadow_target.x += CONFIGSHADOWOFFSETX;
    shadow_target.y += CONFIGSHADOWOFFSETY;
    Uint32 sh_color = SDL_MapRGBA(screen->format,SHADOWCOLOR);
    SDL_FillRect(screen,&shadow_target,sh_color);

// background
    SDL_Rect target =  m_DrawRect;
    Uint32 bg_color = SDL_MapRGBA(screen->format,BACKGROUNDCOLOR);
    SDL_FillRect(screen,&target,bg_color);

// background border
    rectangleRGBA(screen, target.x,target.y, target.x+target.w,target.y+target.h,SELECTIONCOLOR);

    for (size_t i=0; i<m_ConfigWidgets.size(); ++i)
    {
        m_ConfigWidgets[i].BlitTo(screen);
    }
}

void GameConfigWindow::UpdateCurrentConfig(int dir)
{
    m_CurrentConfig += dir;
    if (m_CurrentConfig<0) m_CurrentConfig = m_ConfigWidgets.size()-1;
    else
    if (m_CurrentConfig>=m_ConfigWidgets.size()) m_CurrentConfig = 0;

    for (size_t i=0; i<m_ConfigWidgets.size(); ++i)
    {
        m_ConfigWidgets[i].SetSelected(i==m_CurrentConfig);
    }
}

GameConfigWidget& GameConfigWindow::GetCurrentConfig()
{
    return m_ConfigWidgets[m_CurrentConfig];
}

