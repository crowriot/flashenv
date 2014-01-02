// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------

#ifndef FLASHWINDOW_H
#define FLASHWINDOW_H

#include "keymap.h"

struct _NPWindow;
struct _GtkWidget;
struct _GdkEventKey;
struct _GdkCursor;
struct _GtkBindingSet;

class FlashWindow
{
public:
    FlashWindow();
    ~FlashWindow();

    /// set key map for key redirection
    void SetKeyMap( const KeyMap& key_map );

    /// initializes an NPWindow instance to be used for NPN_SetWindow
    bool InitializeNPWindow( int width, int height );

    /// show the window
    void Show();

    /// modal run loop
    void RunLoop();

    /// returns the _NPWindow instance
    inline _NPWindow* GetNPWindow() { return m_NPWindow; }

    /// key event handler
    bool OnKey(_GtkWidget* widget, _GdkEventKey* event);


private:
    _GtkWidget* m_GtkWindow;
    _NPWindow* m_NPWindow;
    _GdkCursor* m_BlankCursor;
    _GdkCursor* m_PrevCursor;
    _GtkBindingSet* m_KeyBindings;
    KeyMap m_KeyMap;
};


#endif

