// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------

#ifndef FLASHWINDOW_H
#define FLASHWINDOW_H

#include "keymap.h"
#include <gdk/gdkevents.h>
#include <vector>

struct _NPWindow;
struct _GtkWidget;
struct _GdkEventKey;
struct _GdkCursor;
struct _GtkBindingSet;

class FlashPlayer;

class FlashWindow
{
public:
    FlashWindow();
    ~FlashWindow();

    /// set key map for key redirection
    void SetKeyMap( const KeyMapGdk& key_map );

    /// initializes an NPWindow instance to be used for NPN_SetWindow
    bool InitializeNPWindow( int width, int height );

    /// set the player that owns the window
    void SetPlayer( FlashPlayer* );

    /// show the window
    void Show();

    /// modal run loop
    void RunLoop();

    /// returns the _NPWindow instance
    inline _NPWindow* GetNPWindow() { return m_NPWindow; }

    /// key event handler
    bool OnKey(_GtkWidget* widget, _GdkEventKey* event);

    /// key snooper event handler
    bool OnKeySnooper(_GtkWidget* widget, _GdkEventKey* event);


private:
    _GtkWidget* m_MainWindow;
    _GtkWidget* m_ScrollWindow;
    _NPWindow* m_NPWindow;
    _GdkCursor* m_BlankCursor;
    _GdkCursor* m_PrevCursor;
    _GtkBindingSet* m_KeyBindings;
    _GtkWidget* m_Socket;
    KeyMapGdk m_KeyMap;
    FlashPlayer* m_Player;
};


#endif

