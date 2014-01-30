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
struct _GdkEventMotion;
struct _GdkEventButton;
struct _GtkDrawingArea;
struct _GdkGC;
union _GdkEvent;

class FlashPlayer;

class FlashWindow
{
public:
    FlashWindow();
    ~FlashWindow();

    /// set key map for key redirection
    void SetKeyMap( const KeyMapGdk& key_map );

    /// set the dimensionf of the flash window.
    /// usually src_width and trg_width are set to SCREENWIDTH
    /// and trg_width and trg_height to SCREENHEIGHT
    /// if one of the width or height values differ, fraembuffer drawing gets enabled
    /// set trg_width or trg_height to 0 to automatically caculate the dimensions to stretch
    /// it to pandora screen size while keeping aspect in tact.
    void SetDimensions(int src_width, int src_height, int trg_width, int trg_height);

    /// initializes an NPWindow instance to be used for NPN_SetWindow
    bool InitializeNPWindow( );

    /// set the player that owns the window
    void SetPlayer( FlashPlayer* );

    /// show the window
    void Show();

    /// single process message
    void Step();

    /// modal run loop
    void RunLoop();

    /// returns the _NPWindow instance
    inline _NPWindow* GetNPWindow() { return m_NPWindow; }

    /// returns the XID of the top level widget
    unsigned long GetMainWindowXID() const;

    /* event handlers */

    /// key event handler
    bool OnKey(_GtkWidget* widget, _GdkEventKey* event);

    /// key snooper event handler
    bool OnKeySnooper(_GtkWidget* widget, _GdkEventKey* event);

    /// any event
    void OnEvent(_GdkEvent* event);


protected:
    /// setup framebuffer rendering
    void SetupFramebuffer();
    /// restore framebuffer to default settings
    void RestoreFramebuffer();
    /// center mouse cursor
    void CenterMouseCursor();
    /// send left click
    void SimulateLeftClick(int x, int y);

private:
    int m_SrcWidth, m_SrcHeight;
    int m_TrgWidth, m_TrgHeight;
    int m_OffsetX, m_OffsetY;
    bool m_FramebufferEnabled;
    _GtkWidget* m_MainWindow;
    _GtkWidget* m_ScrollWindow;
    _NPWindow* m_NPWindow;
    _GdkCursor* m_BlankCursor;
    _GdkCursor* m_PrevCursor;
    _GtkBindingSet* m_KeyBindings;
    _GtkWidget* m_Socket;
    _GdkGC* m_BlackGC;
    KeyMapGdk m_KeyMap;
    FlashPlayer* m_Player;
    bool m_LeftClickOnExpose;
};


#endif

