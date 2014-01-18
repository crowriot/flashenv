// --------------------------------------------------------------------
// crow_riot, 2013
// --------------------------------------------------------------------


#include "flashwindow.h"
#include "flashplayer.h"
#include "defines.h"
#include "../preload/drawhook.h"
#include "../preload/inputhook.h"
#include "../third_party/npapi/npfunctions.h"
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <dlfcn.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <X11/Xlib.h>

#ifndef GSEAL
#define GSEAL(x) x
#endif

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkscreen.h>
#include <gdk/gdkx.h>


using namespace std;

#ifdef _DEBUG
#define DEBUG_KEY_HANDLERS
//#define DEBUG_MOTION_HANDLERS
#define DEBUG_BUTTON_HANDLERS
#endif

static gboolean KeyHandler(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
#ifdef _DEBUG
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

static void EventHandler(GdkEvent* event, gpointer data)
{
    reinterpret_cast<FlashWindow*>(data)->OnEvent(event);

    gtk_main_do_event(event);
}


EnableFramebufferDrawFN G_enable_framebuffer_draw = 0;

FlashWindow::FlashWindow()
    : m_MainWindow(0)
    , m_NPWindow(0)
    , m_BlankCursor(0)
    , m_PrevCursor(0)
    , m_KeyBindings(0)
    , m_Socket(0)
    , m_SrcWidth(SCREENWIDTH)
    , m_SrcHeight(SCREENHEIGHT)
    , m_TrgWidth(SCREENWIDTH)
    , m_TrgHeight(SCREENHEIGHT)
    , m_OffsetX(0)
    , m_OffsetY(0)
    , m_BlackGC(0)
{

}

FlashWindow::~FlashWindow()
{
    delete m_NPWindow;
}

void FlashWindow::SetDimensions(int src_width, int src_height, int trg_width, int trg_height)
{
    m_SrcWidth = src_width; m_SrcWidth += m_SrcWidth%8; //width must be divisible by 8 for fb draw to work!
    m_SrcHeight = src_height;
    m_TrgWidth = trg_width;
    m_TrgHeight = trg_height;

    if (m_TrgWidth==0 || m_TrgWidth==0)
    {
        float scale_factor = MIN(float(SCREENWIDTH)/m_SrcWidth, float(SCREENHEIGHT)/m_SrcHeight);
        m_TrgWidth = (int)(m_SrcWidth * scale_factor);
        m_TrgHeight = (int)(m_SrcHeight * scale_factor);
    }
    if (m_TrgWidth>SCREENWIDTH) m_TrgWidth = SCREENWIDTH;
    if (m_TrgHeight>SCREENHEIGHT) m_TrgHeight = SCREENHEIGHT;

    m_OffsetX = SCREENWIDTH/2 - m_TrgWidth/2;
    m_OffsetY = SCREENHEIGHT/2 - m_TrgHeight/2;
}

void FlashWindow::SetKeyMap(const KeyMapGdk& key_map)
{
    m_KeyMap = key_map;
}


void FlashWindow::SetPlayer(FlashPlayer* player)
{
    m_Player = player;
}

bool FlashWindow::InitializeNPWindow()
{
    m_MainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gdk_event_handler_set(EventHandler,this,NULL);

	gtk_key_snooper_install(KeySnooperHandler, this);

    gtk_widget_set_usize(m_MainWindow,SCREENWIDTH,SCREENHEIGHT);

	gtk_widget_realize(m_MainWindow);

    gtk_window_fullscreen((GtkWindow*)m_MainWindow);

	m_BlackGC = m_MainWindow->style->black_gc;

	m_Socket = gtk_socket_new();

	gtk_widget_set_usize(m_Socket, m_SrcWidth, m_SrcHeight);

    // m_ScrollWindow = gtk_scrolled_window_new (NULL, NULL);
    // gtk_container_add (GTK_CONTAINER(m_MainWindow), m_ScrollWindow);
    // gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_ScrollWindow),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    // gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(m_ScrollWindow), m_Socket);

    gtk_container_add(GTK_CONTAINER(m_MainWindow),m_Socket);

	gtk_widget_realize(m_Socket);

    g_signal_connect(G_OBJECT(m_MainWindow), "key-press-event", G_CALLBACK(KeyHandler), this);
	g_signal_connect(G_OBJECT(m_MainWindow), "key-release-event", G_CALLBACK(KeyHandler), this);
	g_signal_connect(G_OBJECT(m_MainWindow), "destroy", G_CALLBACK(DestroyHandler), NULL);

	g_signal_connect(m_Socket, "destroy", G_CALLBACK(gtk_widget_destroyed), &m_Socket);

	GdkNativeWindow ww = gtk_socket_get_id(GTK_SOCKET(m_Socket));
	GdkWindow *w = gdk_window_lookup(ww);

	m_NPWindow = new NPWindow;
	memset(m_NPWindow,0,sizeof(NPWindow));

    m_NPWindow->window = (void*)(unsigned long)ww;
    m_NPWindow->x = 0;
    m_NPWindow->y = 0;
    m_NPWindow->width = m_SrcWidth;
	m_NPWindow->height = m_SrcHeight;

	NPSetWindowCallbackStruct* ws_info = new NPSetWindowCallbackStruct;
	ws_info->type = NP_SETWINDOW;
	ws_info->display = GDK_WINDOW_XDISPLAY(w);
	ws_info->colormap = GDK_COLORMAP_XCOLORMAP(gdk_drawable_get_colormap(w));

	GdkVisual* gdkVisual = gdk_drawable_get_visual(w);
    ws_info->visual = GDK_VISUAL_XVISUAL(gdkVisual);
	ws_info->depth = gdkVisual->bits_per_rgb;

	m_NPWindow->ws_info = ws_info;
	m_NPWindow->type = NPWindowTypeWindow;

	GdkColor black = {0,0,0,0};
	for (int state=0; state<=GTK_STATE_INSENSITIVE; ++state)
	{
        gtk_widget_modify_bg(m_MainWindow, (GtkStateType)state, &black);
        gtk_widget_modify_bg(m_Socket, (GtkStateType)state, &black);
	}
	gtk_widget_set_visible(m_Socket,false);

    SetupFramebuffer();

    return true;
}


void FlashWindow::Show()
{

    gtk_widget_show_all(m_MainWindow);
}

void FlashWindow::Step()
{
    gtk_main_iteration_do(false);
}

void FlashWindow::RunLoop()
{
    gtk_main();

    RestoreFramebuffer();
}

unsigned long FlashWindow::GetMainWindowXID() const
{
    return GDK_WINDOW_XWINDOW(m_MainWindow->window);
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
gboolean CreateKeyEvent(GdkWindow *window,
                        int x,
                        int y,
                        guint keyval,
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


void FlashWindow::OnEvent(_GdkEvent* event)
{
    if (event->type==GDK_BUTTON_PRESS || event->type==GDK_BUTTON_RELEASE)
    {
        event->button.x -= m_OffsetX; event->button.x *= m_SrcWidth; event->button.x /= m_TrgWidth;
        event->button.y -= m_OffsetY; event->button.y *= m_SrcHeight; event->button.y /= m_TrgHeight;
    }
    else
    if (event->type==GDK_MOTION_NOTIFY)
    {
        event->motion.x -= m_OffsetX; event->motion.x *= m_SrcWidth; event->motion.x /= m_TrgWidth;
        event->motion.y -= m_OffsetY; event->motion.y *= m_SrcHeight; event->motion.y /= m_TrgHeight;
    }
    else
    if (event->type==GDK_EXPOSE)
    {
        gdk_draw_rectangle(event->expose.window,
                           m_BlackGC,
                           true,
                           event->expose.area.x,
                           event->expose.area.y,
                           event->expose.area.width,
                           event->expose.area.height);
    }
}

void FlashWindow::SetupFramebuffer()
{
    G_enable_framebuffer_draw =
            (EnableFramebufferDrawFN)(dlsym(dlopen(DRAWHOOK_LIBRARY_NAME,RTLD_LAZY),ENABLEFRAMEBUFFERDRAW_FUNCTION_NAME));

    SetPointerChangeFN set_pointer_change =
        (SetPointerChangeFN)(dlsym(dlopen(INPUTHOOK_LIBRARY_NAME,RTLD_LAZY),SETPOINTERCHANGE_FUNCTION_NAME));

	if (m_SrcWidth!=m_TrgWidth || m_SrcHeight!=m_TrgHeight)
    {
        if (G_enable_framebuffer_draw) (*G_enable_framebuffer_draw)(1);

        //# ofbset -fb /dev/fb1 -pos 0 0 -size 800 480 -mem 192000 -en 1
        //# fbset -fb /dev/fb1 -g 400 240 400 240 16

        int pages = FB_PAGES;
        int bpp = 16;
        int mem = pages*m_SrcWidth*m_SrcHeight*bpp/8;

        char ofbset[PATH_MAX];
        sprintf(ofbset,"ofbset -fb /dev/fb1 -pos %d %d -size %d %d -mem %d -en 1", m_OffsetX, m_OffsetY, m_TrgWidth, m_TrgHeight, mem);
        char fbset[PATH_MAX];
        sprintf(fbset,"fbset -fb /dev/fb1 -g %d %d %d %d %d", m_SrcWidth, m_SrcHeight, m_SrcWidth, m_SrcHeight*pages, bpp);

        system(ofbset);
        system(fbset);

        PointerChange change;
        change.off_x = m_OffsetX;
        change.mul_x = m_SrcWidth;
        change.div_x = m_TrgWidth;

        change.off_y = m_OffsetY;
        change.mul_y = m_SrcHeight;
        change.div_y = m_TrgHeight;

        if (set_pointer_change) (*set_pointer_change)(change);
    }
    else
    {
        if (G_enable_framebuffer_draw) (*G_enable_framebuffer_draw)(0);
    }
}

void FlashWindow::RestoreFramebuffer()
{
    if (m_SrcWidth!=m_TrgWidth || m_SrcHeight!=m_TrgHeight)
    {
        if (G_enable_framebuffer_draw) (*G_enable_framebuffer_draw)(0);

        system("ofbset -fb /dev/fb1 -pos 0 0 -size 0 0 -mem 0 -en 0");
    }
}
