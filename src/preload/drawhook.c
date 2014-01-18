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


/// function for external frame buffer enabling
void EnableFramebufferDraw(int enable)
{
    fb_draw_enabled = enable;
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
    if ( FB_PAGES>1 )
    {
        fb_curr ++;
        fb_curr %= FB_PAGES;

        image->mem = fb_mem[fb_curr];
    }
}

