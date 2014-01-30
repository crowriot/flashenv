// --------------------------------------------------------------------
// crow_riot, 2014
// --------------------------------------------------------------------

#define _GNU_SOURCE
#include <dlfcn.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <X11/extensions/Xfixes.h>
#include "drawhook.h"

/// gdk_image_new hooking

#define SCREENWIDTH 800
#define SCREENHEIGHT 480


/// function pointer declaration
typedef GdkImage* (*image_new)(GdkImageType  type, GdkVisual *visual, gint width, gint height);

/// original gdk_image_new function pointer
static image_new image_new_orig = 0;

/// frame buffer draw enable flag
static int fb_draw_enabled = 0;

/// current frame buffer index
static int fb_curr = 0;
/// frame buffer pointer
static void* fb_mem[FB_PAGES] = {0};
/// full memory size of framebuffer
static int fb_mem_size = 0;
/// width of framebuffer in pixels
static int fb_width = 0;
/// height of framebuffer in pixels
static int fb_height = 0;


/// cursor image drawing
typedef XFixesCursorImage* (*XFIXESGETCURSORIMAGEFN)(Display* display);
/// current cursor image
static XFIXESGETCURSORIMAGEFN XFixesGetCursorImageProc = 0;
/// default display
Display* x11_display = 0;



/// function for external frame buffer enabling
void EnableFramebufferDraw(int enable)
{
    fb_draw_enabled = enable;

    if (fb_draw_enabled==0 && fb_mem[0]!=0)
    {
        munmap( fb_mem[0], fb_mem_size );
        memset(fb_mem,0,sizeof(fb_mem));
    }
    else
    if (fb_draw_enabled)
    {
        XFixesGetCursorImageProc = (XFIXESGETCURSORIMAGEFN)dlsym(RTLD_NEXT,"XFixesGetCursorImage");
        x11_display = XOpenDisplay(0);
        printf("hooked XFixesGetCursorImageProc: %p\n", XFixesGetCursorImageProc);
    }
}

#define R32(argb) ((argb&0x00FF0000)>>16)
#define G32(argb) ((argb&0x0000FF00)>>8)
#define B32(argb) ((argb&0x000000FF))

#define R16(argb) ((R32(argb) >> (3)) << 11)
#define B16(argb) ((G32(argb) >> (2)) << 5)
#define G16(argb) ((B32(argb) >> (3)) )

static void draw_cursor( unsigned short* data )
{
    if (!XFixesGetCursorImageProc)
        return;

    XFixesCursorImage* img = (*XFixesGetCursorImageProc)(x11_display);
    if (!img)
        return;

    int screen = DefaultScreen(x11_display);
    Window window = RootWindow(x11_display,screen);
    Window root_window, child_window;
    int root_x, root_y, child_x, child_y;
    unsigned int mask;

    XQueryPointer(x11_display,window,&root_window,&child_window, &root_x, &root_y, &child_x, &child_y, &mask);

    int y,x;
    for (y=0; y<img->height; ++y)
    {
        int ty = root_y + y;

        if (ty<0 || ty>=fb_height)
            continue;

        for (x=0; x<img->width; ++x)
        {
            int tx = root_x + x;

            if(tx>=0 && tx<fb_width)
            {
                unsigned long src_pixel = img->pixels[y*img->width+x];
                unsigned short src_pixel_16 = R16(src_pixel)|G16(src_pixel)|B16(src_pixel);

                if (src_pixel)
                    data[(root_y+y)*fb_width+x+root_x] = src_pixel_16;
            }
        }
    }
}

/// hooked gdk_image_new
GdkImage* gdk_image_new(GdkImageType type, GdkVisual *visual, gint width, gint height)
{
    if (image_new_orig==0)
    {
        image_new_orig = (image_new)dlsym(RTLD_NEXT,"gdk_image_new");
    }

    printf("hooked gdk_image_new: type=%d w=%d h=%d\n", type, width, height);


    GdkImage* image = (*image_new_orig)(GDK_IMAGE_FASTEST, visual, width, height);

    if (fb_draw_enabled)
    {
        int fb = open("/dev/fb1",O_RDWR);
        if (fb>=0)
        {
            int page_mem_size = width*height*2;
            int mem_size = page_mem_size*FB_PAGES;
            fb_width = width;
            fb_height = height;

            fb_mem[0] = mmap(0, mem_size, PROT_READ|PROT_WRITE, MAP_SHARED, fb, 0);
            if (fb_mem[0]==MAP_FAILED)
            {
                fb_mem[0] = 0;
                printf("hooked gdk_image_new: failed to open framebuffer: %s\n",strerror(errno));
            }
            else
            {
                int i;

                image->mem = fb_mem[0];
                fb_curr = 0;

                memset(fb_mem[0],0,mem_size);

                for (i=1;i<FB_PAGES;++i)
                    fb_mem[i] = fb_mem[i-1] + page_mem_size;

                printf("hooked gdk_image_new: drawing to framebuffer: %p\n",fb_mem[0]);
            }

            close(fb);
        }
    }
    else
    {
        printf("hooked gdk_image_new: not using direct framebuffer\n");
    }


    return image;
}


/// function pointer declaration
typedef void (*draw_image)(GdkDrawable *drawable, GdkGC *gc, GdkImage *image, gint xsrc, gint ysrc, gint xdest, gint ydest, gint width, gint height);

///original gdk_draw_image function pointer
static draw_image draw_image_orig = 0;


void gdk_draw_image(GdkDrawable *drawable, GdkGC *gc, GdkImage *image, gint xsrc, gint ysrc, gint xdest, gint ydest, gint width, gint height)
{
    if (draw_image_orig==0)
    {
        draw_image_orig = (draw_image)dlsym(RTLD_NEXT,"gdk_draw_image");
    }
    if ( fb_mem[0]==0 || image->mem!=fb_mem[fb_curr] )
    {
        // printf("hooked gdk_draw_image w=%d h=%d\n", width, height);

        (*draw_image_orig)(drawable, gc, image, xsrc, ysrc, xdest, ydest, width, height);
    }
    else
    {
        //draw_cursor((unsigned short*)image->mem);

#if FB_PAGES>0
        fb_curr ++;
        fb_curr %= FB_PAGES;
        image->mem = fb_mem[fb_curr];
#endif
    }
}

