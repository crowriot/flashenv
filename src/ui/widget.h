// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#ifndef WIDGET_H
#define WIDGET_H

#include <SDL.h>
#include <SDL/SDL_ttf.h>
#include <string>
#include "filelist.h"


/// simple widget class
class Widget
{
public:
    /// constructor
    Widget();
    /// destructor
    virtual ~Widget();

    /** picking **/

    /// pick the widget, returns true if mx and my are within the widget bounds
    bool Pick( int mx, int my );

    /** selection **/

    /// set selection
    void SetSelected( int select );
    /// get selection
    int GetSelected() const;

    /** text **/

    /// set the font to be used for drawing
    void SetFont( TTF_Font* font );
    /// set the font colors for normal and selected state
    void SetColors( const SDL_Color& normal, const SDL_Color& selected );

    /// override: return the text that should be drawn
    virtual std::string GetText() const = 0;

    /// updates the text surfaces using the given font.
    void UpdateSurfaces();

    /// returns the width of the normal text surface
    int GetTextWidth() const;
    /// returns the height of the normal text surface
    int GetTextHeight() const;

    /** drawing **/

    /// rectangle of the text to be drawn
    void SetRect( const SDL_Rect& rect );
    /// returns the current drawing rectangle
    SDL_Rect GetRect() const;

    /// blit text to target
    void BlitTo(SDL_Surface* target);

protected:
    /// free allocated surfaces
    void FreeSurfaces();

private:
    SDL_Color m_ColorNormal;
    SDL_Color m_ColorSelected;
    SDL_Rect m_TextRect;
    SDL_Surface* m_TextSurface;
    SDL_Surface* m_TextSelectedSurface;
    std::string m_CurrentText;
    int m_Selected;
    TTF_Font* m_Font;
};


/// widget where text can be set
class TextWidget : public Widget
{
public:
    /// default constructor
    TextWidget();
    /// initializes the drawn text with |text|
    TextWidget(const char* text);

    /// set text to |text|
    void SetText(const std::string& text);
    /// override. returns the defined text.
    std::string GetText() const;

private:
    std::string m_Text;
};


typedef std::vector<Widget*> WidgetList;


#endif
