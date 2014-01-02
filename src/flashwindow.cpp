// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------


#include "flashwindow.h"
#include "npapi/npapi.h"
#include <string.h>

#define GSEAL(x) x
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

static gboolean KeyHandler(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    return reinterpret_cast<FlashWindow*>(user_data)->OnKey(widget, event);
}

static void DestroyHandler( GtkWidget *widget, gpointer data )
{
    gtk_main_quit();
}

FlashWindow::FlashWindow()
    : m_GtkWindow(0)
    , m_NPWindow(0)
    , m_BlankCursor(0)
    , m_PrevCursor(0)
    , m_KeyBindings(0)
{
}

FlashWindow::~FlashWindow()
{
 //   gtk_widget_destroy(m_GtkWindow);

    delete m_NPWindow;
}


void FlashWindow::SetKeyMap(const KeyMap& key_map)
{
    m_KeyMap = key_map;
}

bool FlashWindow::InitializeNPWindow(int width, int height)
{
// gtk part
    m_GtkWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_widget_set_usize(m_GtkWindow,width,height);

	g_signal_connect(G_OBJECT(m_GtkWindow), "destroy", G_CALLBACK(DestroyHandler), NULL);
	g_signal_connect(G_OBJECT(m_GtkWindow), "key_press_event", G_CALLBACK(KeyHandler), this);
	g_signal_connect(G_OBJECT(m_GtkWindow), "key_release_event", G_CALLBACK(KeyHandler), this);

	gtk_widget_realize(m_GtkWindow);

	GdkWindow* parent_win = m_GtkWindow->window;

	GtkWidget* socketWidget = gtk_socket_new();
	gtk_widget_set_parent_window(socketWidget, parent_win);

	g_signal_connect(socketWidget, "destroy", G_CALLBACK(gtk_widget_destroyed), &socketWidget);

	gpointer user_data = NULL;
	gdk_window_get_user_data(parent_win, &user_data);

	GtkContainer* container = GTK_CONTAINER(user_data);
	gtk_container_add(container, socketWidget);
	gtk_widget_realize(socketWidget);

	GtkAllocation new_allocation;
	new_allocation.x = 0;
	new_allocation.y = 0;
	new_allocation.width = width;
	new_allocation.height = height;
	gtk_widget_size_allocate(socketWidget, &new_allocation);

	gtk_widget_show(socketWidget);
	gdk_flush();

	GdkNativeWindow ww = gtk_socket_get_id(GTK_SOCKET(socketWidget));
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
    gtk_widget_show_all(m_GtkWindow);

    gtk_window_fullscreen((GtkWindow*)m_GtkWindow);
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

    if ((event->type==GDK_KEY_PRESS || event->type==GDK_KEY_RELEASE) && IsPandoraKey(event->keyval))
    {
        KeyMap::iterator it = m_KeyMap.find((PandoraKeys)event->keyval);
        if (it!=m_KeyMap.end())
        {
            GdkDisplay* display = gdk_display_get_default ();
            GdkScreen* screen = gdk_display_get_default_screen (display);

            gint x=0,y=0;
            gdk_display_get_pointer (display, NULL, &x, &y, NULL);

            const SimpleKey& simkey = it->second;
            gdk_test_simulate_key(widget->window,x,y,simkey.keyval,simkey.modifier,event->type);//==GDK_KEY_PRESS ? GDK_KEY_PRESS : GDK_KEY_RELEASE);
            return 1;
        }
    }

    return 0;
}

