// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------


#include "flashwindow.h"
#include "flashplayer.h"
#include "defines.h"
#include "../third_party/npapi/npfunctions.h"
#include <string.h>
#include <iostream>
#include <stdlib.h>

#ifndef GSEAL
#define GSEAL(x) x
#endif

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkscreen.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>


using namespace std;

#ifdef _DEBUG
#define DEBUG_KEY_HANDLERS
#endif

static gboolean KeyHandler(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
#ifdef DEBUG_KEY_HANDLERS
    DEBUG_FUNCTION_NAME
    cout << "\tkeyval=" << event->keyval << " modifier=" << event->state << " type=" << event->type << endl;
#endif

    return reinterpret_cast<FlashWindow*>(user_data)->OnKey(widget, event);
}

static gboolean KeySnooperHandler(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
{
#ifdef DEBUG_KEY_HANDLERS
    DEBUG_FUNCTION_NAME
    cout << "\tkeyval=" << event->keyval << " modifier=" << event->state << " type=" << event->type << endl;
#endif

    return reinterpret_cast<FlashWindow*>(user_data)->OnKeySnooper(widget, event);
}

static void DestroyHandler( GtkWidget *widget, gpointer data )
{
    gtk_main_quit();
}

FlashWindow::FlashWindow()
    : m_MainWindow(0)
    , m_NPWindow(0)
    , m_BlankCursor(0)
    , m_PrevCursor(0)
    , m_KeyBindings(0)
    , m_Socket(0)
{
}

FlashWindow::~FlashWindow()
{
 //   gtk_widget_destroy(m_MainWindow);

    delete m_NPWindow;
}


void FlashWindow::SetKeyMap(const KeyMapGdk& key_map)
{
    m_KeyMap = key_map;
}


void FlashWindow::SetPlayer(FlashPlayer* player)
{
    m_Player = player;
}

bool FlashWindow::InitializeNPWindow(int width, int height)
{
// gtk part
    m_MainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	gtk_key_snooper_install(KeySnooperHandler, this);

// main window is ALWAYS fullscreen
    gtk_widget_set_usize(m_MainWindow,SCREENWIDTH,SCREENHEIGHT);

    GdkColor black_color; memset(&black_color,0,sizeof(GdkColor));

	g_signal_connect(G_OBJECT(m_MainWindow), "destroy", G_CALLBACK(DestroyHandler), NULL);

	gtk_widget_realize(m_MainWindow);


	m_Socket = gtk_socket_new();

    if (width>SCREENWIDTH || height>SCREENHEIGHT)
    {
        m_ScrollWindow = gtk_scrolled_window_new (NULL, NULL);
        gtk_container_add (GTK_CONTAINER(m_MainWindow), m_ScrollWindow);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_ScrollWindow),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(m_ScrollWindow), m_Socket);
    }
    else
    {
        gtk_container_add(GTK_CONTAINER(m_MainWindow),m_Socket);
    }


	gtk_widget_realize(m_Socket);


    g_signal_connect(G_OBJECT(m_MainWindow), "key-press-event", G_CALLBACK(KeyHandler), this);
	g_signal_connect(G_OBJECT(m_MainWindow), "key-release-event", G_CALLBACK(KeyHandler), this);
	g_signal_connect(m_Socket, "destroy", G_CALLBACK(gtk_widget_destroyed), &m_Socket);

	GdkNativeWindow ww = gtk_socket_get_id(GTK_SOCKET(m_Socket));
	GdkWindow *w = gdk_window_lookup(ww);

	m_NPWindow = new NPWindow;
	memset(m_NPWindow,0,sizeof(NPWindow));

    m_NPWindow->window = (void*)(unsigned long)ww;
    m_NPWindow->x = 0;
    m_NPWindow->y = 0;
    m_NPWindow->width = width;
	m_NPWindow->height = height;

	NPSetWindowCallbackStruct* ws_info = new NPSetWindowCallbackStruct;
	ws_info->type = NP_SETWINDOW;
	ws_info->display = GDK_WINDOW_XDISPLAY(w);
	ws_info->colormap = GDK_COLORMAP_XCOLORMAP(gdk_drawable_get_colormap(w));

	GdkVisual* gdkVisual = gdk_drawable_get_visual(w);
    ws_info->visual = GDK_VISUAL_XVISUAL(gdkVisual);
	ws_info->depth = gdkVisual->bits_per_rgb;

	m_NPWindow->ws_info = ws_info;
	m_NPWindow->type = NPWindowTypeWindow;


    return true;
}


void FlashWindow::Show()
{
    gtk_window_fullscreen((GtkWindow*)m_MainWindow);

    gtk_widget_show_all(m_MainWindow);
}

void FlashWindow::RunLoop()
{

    gtk_main();
}

bool FlashWindow::OnKey(_GtkWidget* widget, _GdkEventKey* event)
{
    switch(event->keyval)
    {
    case GDK_q:
    case GDK_Q:
        if (event->type==GDK_KEY_PRESS && event->state & GDK_CONTROL_MASK)
        {
            gtk_main_quit();
            return 1;
        }
        break;
    case GDK_m:
    case GDK_M:
        if (event->type==GDK_KEY_PRESS && event->state & GDK_CONTROL_MASK)
        {
            if (m_BlankCursor==NULL)
                m_BlankCursor = gdk_cursor_new(GDK_BLANK_CURSOR);

            if (gdk_window_get_cursor(widget->window)!=m_BlankCursor)
            {
                m_PrevCursor = gdk_window_get_cursor(widget->window);
                gdk_window_set_cursor(widget->window,m_BlankCursor);
            }
            else
                gdk_window_set_cursor(widget->window,m_PrevCursor);

            return 1;
        }
        break;

    }
    return 0;
}


/// modified code from
/// https://git.gnome.org/browse/gtk+/plain/gdk/x11/gdktestutils-x11.c
/// _gdk_x11_window_simulate_key
gboolean CreateKeyEvent(GdkWindow      *window,
                        int x,
                        int y,
                        guint           keyval,
                        GdkModifierType modifiers,
                        GdkEventType    key_pressrelease,
                        XKeyEvent& xev)
{

    GdkScreen *screen;
    GdkKeymapKey *keys = NULL;
    gboolean success;
    gint n_keys = 0;


    screen = gdk_screen_get_default();

    xev.serial = 0;
    xev.send_event = 1;
    xev.type = key_pressrelease == GDK_KEY_PRESS ? KeyPress : KeyRelease;
    xev.display = GDK_WINDOW_XDISPLAY (window);
    xev.window = GDK_WINDOW_XID (window);
    xev.root = RootWindow (xev.display, GDK_SCREEN_XNUMBER(screen));
    xev.subwindow = 0;
    xev.time = 0;
    xev.x = MAX (x, 0);
    xev.y = MAX (y, 0);
    xev.x_root = 0;
    xev.y_root = 0;
    xev.state = modifiers;
    xev.keycode = 0;
    xev.same_screen = 1;
    success = gdk_keymap_get_entries_for_keyval( gdk_keymap_get_for_display(gdk_display_get_default()) , keyval, &keys, &n_keys);
    success &= n_keys > 0;
    if (success)
    {
        gint i;
        for (i = 0; i < n_keys; i++)
        {
            if (keys[i].group == 0 && (keys[i].level == 0 || keys[i].level == 1))
            {
                xev.keycode = keys[i].keycode;
                if (keys[i].level == 1)
                {
                    /* Assume shift takes us to level 1 */
                    xev.state |= GDK_SHIFT_MASK;
                }
                break;
            }
        }
        if (i >= n_keys) /* no match for group==0 and level==0 or 1 */
            xev.keycode = keys[0].keycode;
    }

    g_free (keys);

    return success;
}


bool FlashWindow::OnKeySnooper(_GtkWidget* widget, _GdkEventKey* event)
{
    if (widget == m_MainWindow && (event->type==GDK_KEY_PRESS || event->type==GDK_KEY_RELEASE) && IsPandoraKey(event->keyval))
    {
        KeyMapGdk::iterator it = m_KeyMap.find(event->keyval);
        if (it!=m_KeyMap.end())
        {

            GdkDisplay* display = gdk_display_get_default ();
            gint x=0,y=0;
            gdk_display_get_pointer (display, NULL, &x, &y, NULL);

            const vector<SimpleKey>& target_keys = it->second;
            for (size_t k=0; k<target_keys.size();  ++k)
            {
                const SimpleKey& sim_key = target_keys[k];

                XKeyEvent key_event;
                CreateKeyEvent(widget->window, x, y, sim_key.keyval, sim_key.modifier, event->type, key_event);
                m_Player->SendEvent(&key_event);

            }
            return 1;
        }
    }

    return 0;
}

