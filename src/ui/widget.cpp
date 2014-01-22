// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#include "widget.h"
#include "defines.h"


static const SDL_Color S_ColorNormal = {FONTCOLORNORMAL};
static const SDL_Color S_ColorSelected = {FONTCOLORSELECTED};


Widget::Widget()
    : m_ColorNormal(S_ColorNormal)
    , m_ColorSelected(S_ColorSelected)
    , m_TextSurface(NULL)
    , m_TextSelectedSurface(NULL)
    , m_Selected(0)
    , m_Font(NULL)
{
    memset(&m_TextRect,0,sizeof(m_TextRect));
}

Widget::~Widget()
{
    FreeSurfaces();
}

bool Widget::Pick( int mx, int my )
{
    return mx>=m_TextRect.x && mx<=m_TextRect.x+m_TextRect.w
         && my>=m_TextRect.y && my<=m_TextRect.y+m_TextRect.h;
}

void Widget::SetSelected( int select )
{
    m_Selected = select;
}

int Widget::GetSelected() const
{
    return m_Selected;
}

void Widget::SetFont(TTF_Font* font)
{
    m_Font = font;
}

void Widget::UpdateSurfaces()
{
    if (m_Font==0)
        return;

    std::string text = GetText();

    if ( m_CurrentText!=text )
    {
        m_CurrentText = text;

        FreeSurfaces();

        m_TextSurface = TTF_RenderText_Blended(m_Font,text.c_str(),m_ColorNormal);

        int old_style = TTF_GetFontStyle(m_Font);
        TTF_SetFontStyle(m_Font,TTF_STYLE_BOLD);
        m_TextSelectedSurface = TTF_RenderText_Blended(m_Font,text.c_str(),m_ColorSelected);
        TTF_SetFontStyle(m_Font,old_style);
    }
}

int Widget::GetTextWidth() const
{
    return m_TextSurface->w;
}

int Widget::GetTextHeight() const
{
    return m_TextSurface->h;
}

void Widget::SetRect( const SDL_Rect& rect )
{
    m_TextRect = rect;
}
SDL_Rect Widget::GetRect() const
{
    return m_TextRect;
}

void Widget::BlitTo(SDL_Surface* target)
{
    SDL_Rect target_rect = m_TextRect;

    UpdateSurfaces();

    if (m_Selected && m_TextSelectedSurface) {
        SDL_BlitSurface(m_TextSelectedSurface,NULL,target,&target_rect);
    }
    else
    if (m_TextSurface) {
        SDL_BlitSurface(m_TextSurface,NULL,target,&target_rect);
    }
}

void Widget::FreeSurfaces()
{
    if (m_TextSurface) SDL_FreeSurface(m_TextSurface);
    if (m_TextSelectedSurface) SDL_FreeSurface(m_TextSelectedSurface);
}

// --


TextWidget::TextWidget()
{

}

TextWidget::TextWidget(const char* text)
    : m_Text(text)
{

}

void TextWidget::SetText(const std::string& text)
{
    m_Text = text;
}

std::string TextWidget::GetText() const
{
    return m_Text;
}


