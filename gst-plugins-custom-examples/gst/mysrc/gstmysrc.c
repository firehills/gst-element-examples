/* GStreamer
 * Copyright (C) 2022 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstmysrc
 *
 * The mysrc element generates a buffer, sets the first byte to an incrementing value and puts the bufffer on its src pad.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v mysrc ! fakesink
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasesrc.h>
#include "gstmysrc.h"

#include <unistd.h>



GST_DEBUG_CATEGORY_STATIC (gst_mysrc_debug_category);
#define GST_CAT_DEFAULT gst_mysrc_debug_category

/* prototypes */ 

static void gst_mysrc_set_property (GObject * object,
                                    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_mysrc_get_property (GObject * object,
                                    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_mysrc_dispose (GObject * object);
static void gst_mysrc_finalize (GObject * object);

static GstCaps *gst_mysrc_get_caps (GstBaseSrc * src, GstCaps * filter);
static gboolean gst_mysrc_negotiate (GstBaseSrc * src);
static GstCaps *gst_mysrc_fixate (GstBaseSrc * src, GstCaps * caps);
static gboolean gst_mysrc_set_caps (GstBaseSrc * src, GstCaps * caps);
static gboolean gst_mysrc_decide_allocation (GstBaseSrc * src,
                                             GstQuery * query);
static gboolean gst_mysrc_start (GstBaseSrc * src);
static gboolean gst_mysrc_stop (GstBaseSrc * src);
static void gst_mysrc_get_times (GstBaseSrc * src, GstBuffer * buffer,
                                 GstClockTime * start, GstClockTime * end);
static gboolean gst_mysrc_get_size (GstBaseSrc * src, guint64 * size);
static gboolean gst_mysrc_is_seekable (GstBaseSrc * src);
static gboolean gst_mysrc_prepare_seek_segment (GstBaseSrc * src,
                                                GstEvent * seek, GstSegment * segment);
static gboolean gst_mysrc_do_seek (GstBaseSrc * src, GstSegment * segment);
static gboolean gst_mysrc_unlock (GstBaseSrc * src);
static gboolean gst_mysrc_unlock_stop (GstBaseSrc * src);
static gboolean gst_mysrc_query (GstBaseSrc * src, GstQuery * query);
static gboolean gst_mysrc_event (GstBaseSrc * src, GstEvent * event);
static GstFlowReturn gst_mysrc_create (GstBaseSrc * src, guint64 offset,
                                       guint size, GstBuffer ** buf);
static GstFlowReturn gst_mysrc_alloc (GstBaseSrc * src, guint64 offset,
                                      guint size, GstBuffer ** buf);
static GstFlowReturn gst_mysrc_fill (GstBaseSrc * src, guint64 offset,
                                     guint size, GstBuffer * buf);


/* Some properties to see how get set methods work - actual prop values currently unused */
enum
    {
        ARG_PROP_0 = 1,
        ARG_PROP_1
    };
#define ARG_PROP_1_DEFAULT "SomeDefault"


/* pad templates */
static GstStaticPadTemplate gst_mysrc_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
                             GST_PAD_SRC,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("application/buffer")    // <- This is a NEW type specific to this example
                             );

#define gst_mysrc_parent_class parent_class // Note this define enables creation of the parent_class varaible via later macros



/* class initialization */
G_DEFINE_TYPE_WITH_CODE (GstMysrc, gst_mysrc, GST_TYPE_BASE_SRC,
                         GST_DEBUG_CATEGORY_INIT (gst_mysrc_debug_category, "mysrc", 0,
                                                  "debug category for mysrc element"));


/* class_init, run ONCE early on and sets up global state */
static void
gst_mysrc_class_init (GstMysrcClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstBaseSrcClass *base_src_class = GST_BASE_SRC_CLASS (klass);
    GstMysrc *mysrc = GST_MYSRC (klass);

    GST_DEBUG_OBJECT (mysrc, "gst_mysrc_class_init");
   
    /* Setting up pads and setting metadata should be moved to
       base_class_init if you intend to subclass this class. */
    gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass), &gst_mysrc_src_template);

    gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
                                           "My Test Source",                      // Long Name
                                           "Generic",                             // Classification Type see docs/additional/design/draft-klass.txt
                                           "Output an integer",                   // Description
                                           "Philip Hill <some_email@gmail.com>"); //Author


    // In most plugins, only a few of these are implemented, leaving the base class to provide the implementation. 
  
    gobject_class->set_property = gst_mysrc_set_property;
    gobject_class->get_property = gst_mysrc_get_property;
    gobject_class->dispose = gst_mysrc_dispose;
    gobject_class->finalize = gst_mysrc_finalize;
    base_src_class->get_caps = GST_DEBUG_FUNCPTR (gst_mysrc_get_caps);
    base_src_class->negotiate = GST_DEBUG_FUNCPTR (gst_mysrc_negotiate);
    base_src_class->fixate = GST_DEBUG_FUNCPTR (gst_mysrc_fixate);
    base_src_class->set_caps = GST_DEBUG_FUNCPTR (gst_mysrc_set_caps);
    base_src_class->decide_allocation = GST_DEBUG_FUNCPTR (gst_mysrc_decide_allocation);
    base_src_class->start = GST_DEBUG_FUNCPTR (gst_mysrc_start);
    base_src_class->stop = GST_DEBUG_FUNCPTR (gst_mysrc_stop);
    base_src_class->get_times = GST_DEBUG_FUNCPTR (gst_mysrc_get_times);
    base_src_class->get_size = GST_DEBUG_FUNCPTR (gst_mysrc_get_size);
    base_src_class->is_seekable = GST_DEBUG_FUNCPTR (gst_mysrc_is_seekable);
    base_src_class->prepare_seek_segment = GST_DEBUG_FUNCPTR (gst_mysrc_prepare_seek_segment);
    base_src_class->do_seek = GST_DEBUG_FUNCPTR (gst_mysrc_do_seek);
    base_src_class->unlock = GST_DEBUG_FUNCPTR (gst_mysrc_unlock);
    base_src_class->unlock_stop = GST_DEBUG_FUNCPTR (gst_mysrc_unlock_stop);
    base_src_class->query = GST_DEBUG_FUNCPTR (gst_mysrc_query);
    base_src_class->event = GST_DEBUG_FUNCPTR (gst_mysrc_event);
    base_src_class->create = GST_DEBUG_FUNCPTR (gst_mysrc_create);
    base_src_class->alloc = GST_DEBUG_FUNCPTR (gst_mysrc_alloc);
    base_src_class->fill = GST_DEBUG_FUNCPTR (gst_mysrc_fill);



    /* Two params an int and string defined here, purely to see how they work. Actual values currently unused */
    g_object_class_install_property(gobject_class,
                                    ARG_PROP_0,
                                    g_param_spec_int
                                    (
                                     "param0-name",     // Name
                                     "param0-nickname", //Nickname
                                     "param0-blurb",    // Blurb (details)
                                     0,  //min
                                     10, //max
                                     5,  //default
                                     (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT) //See https://www.freedesktop.org/software/gstreamer-sdk/data/docs/latest/gobject/gobject-GParamSpec.html#GParamFlags
                                     )
                                    );
  

    g_object_class_install_property(gobject_class,
                                    ARG_PROP_1,
                                    g_param_spec_string
                                    (
                                     "param1-name",     // Name
                                     "param1-nickname", //Nickname
                                     "param1-blurb",    // Blurb (details)
                                     ARG_PROP_1_DEFAULT,                 //Default Val
                                     (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT) //See https://www.freedesktop.org/software/gstreamer-sdk/data/docs/latest/gobject/gobject-GParamSpec.html#GParamFlags
                                     )
                                    );

  
}


/* init -> run every INSTANCE of the class */
static void
gst_mysrc_init (GstMysrc *mysrc)
{
    GST_DEBUG_OBJECT (mysrc, "init");

    // any properties to default?
    mysrc->prop1 = NULL;
    mysrc->counter = 0;
}

void
gst_mysrc_set_property (GObject * object, guint property_id,
                        const GValue * value, GParamSpec * pspec)
{
    GstMysrc *mysrc = GST_MYSRC (object);
    const gchar *val;
    GST_DEBUG_OBJECT (mysrc, "set_property");

    switch (property_id) {

    case ARG_PROP_0:
        mysrc->prop0 = g_value_get_int(value);
        break;

    case ARG_PROP_1:
        g_free(mysrc->prop1);
    
        if((val = g_value_get_string(value)))
            mysrc->prop1 = g_strdup(val);
        else
            mysrc->prop1 = g_strdup(ARG_PROP_1_DEFAULT);
        break;
           
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

void
gst_mysrc_get_property (GObject * object, guint property_id,
                        GValue * value, GParamSpec * pspec)
{
    GstMysrc *mysrc = GST_MYSRC (object);

    //GST_DEBUG_OBJECT (mysrc, "get_property");

    switch (property_id) {

    case ARG_PROP_0:
        g_value_set_int(value, mysrc->prop0);
        break;
      
    case ARG_PROP_1:
        g_value_set_string(value, mysrc->prop1);
        break;
      
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

void
gst_mysrc_dispose (GObject * object)
{
    GstMysrc *mysrc = GST_MYSRC (object);

    GST_DEBUG_OBJECT (mysrc, "dispose");

    /* clean up as possible.  may be called multiple times */

    G_OBJECT_CLASS (gst_mysrc_parent_class)->dispose (object);
}

void
gst_mysrc_finalize (GObject * object)
{
    GstMysrc *mysrc = GST_MYSRC (object);

    GST_DEBUG_OBJECT (mysrc, "finalize");

    /* clean up object here */
    g_free(mysrc->prop1);
  
    mysrc->prop1 = NULL;
  
    G_OBJECT_CLASS (gst_mysrc_parent_class)->finalize (object);
}

/* get caps from subclass */
static GstCaps *
gst_mysrc_get_caps (GstBaseSrc * src, GstCaps * filter)
{
    GstCaps *caps;
    GstMysrc *mysrc = GST_MYSRC (src);
    
    caps = gst_caps_new_simple ("application/buffer", NULL, NULL);
    GST_DEBUG_OBJECT (mysrc, "get_caps");
    GST_DEBUG_OBJECT (mysrc, "here are the caps: %" GST_PTR_FORMAT, caps);
    
    return caps;
}

/* decide on caps */
static gboolean
gst_mysrc_negotiate (GstBaseSrc * src)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "negotiate");

    return TRUE;
}

/* called if, in negotiation, caps need fixating */
static GstCaps *
gst_mysrc_fixate (GstBaseSrc * src, GstCaps * caps)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "fixate");

    return NULL;
}

/* notify the subclass of new caps */
static gboolean
gst_mysrc_set_caps (GstBaseSrc * src, GstCaps * caps)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "set_caps");

    return TRUE;
}

/* setup allocation query */
static gboolean
gst_mysrc_decide_allocation (GstBaseSrc * src, GstQuery * query)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "decide_allocation");

    return TRUE;
}

/* start and stop processing, ideal for opening/closing the resource */
static gboolean
gst_mysrc_start (GstBaseSrc * src)
{
    GstMysrc *mysrc = GST_MYSRC (src);
    
    GST_DEBUG_OBJECT (mysrc, "start");

    return TRUE;
}

static gboolean
gst_mysrc_stop (GstBaseSrc * src)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "stop");

    return TRUE;
}

/* given a buffer, return start and stop time when it should be pushed
 * out. The base class will sync on the clock using these times. */
static void
gst_mysrc_get_times (GstBaseSrc * src, GstBuffer * buffer,
                     GstClockTime * start, GstClockTime * end)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "get_times");

}

/* get the total size of the resource in bytes */
static gboolean
gst_mysrc_get_size (GstBaseSrc * src, guint64 * size)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "get_size");

    return TRUE;
}

/* check if the resource is seekable */
static gboolean
gst_mysrc_is_seekable (GstBaseSrc * src)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "is_seekable");

    return TRUE;
}

/* Prepare the segment on which to perform do_seek(), converting to the
 * current basesrc format. */
static gboolean
gst_mysrc_prepare_seek_segment (GstBaseSrc * src, GstEvent * seek,
                                GstSegment * segment)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "prepare_seek_segment");

    return TRUE;
}

/* notify subclasses of a seek */
static gboolean
gst_mysrc_do_seek (GstBaseSrc * src, GstSegment * segment)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "do_seek");

    return TRUE;
}

/* unlock any pending access to the resource. subclasses should unlock
 * any function ASAP. */
static gboolean
gst_mysrc_unlock (GstBaseSrc * src)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "unlock");

    return TRUE;
}

/* Clear any pending unlock request, as we succeeded in unlocking */
static gboolean
gst_mysrc_unlock_stop (GstBaseSrc * src)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "unlock_stop");

    return TRUE;
}

/* notify subclasses of a query */
static gboolean
gst_mysrc_query (GstBaseSrc * src, GstQuery * query)
{
    GstMysrc *mysrc = GST_MYSRC (src);
    gboolean ret = TRUE;
  
    GST_DEBUG_OBJECT (mysrc, "query %d", GST_QUERY_TYPE(query));


    switch(GST_QUERY_TYPE(query)) {
      
#if 0 // usually the parent class does the work here
    case GST_QUERY_UNKNOWN:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break;
    case GST_QUERY_POSITION :
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break;
    case GST_QUERY_DURATION:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break;
    case GST_QUERY_LATENCY:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break;
    case GST_QUERY_JITTER :
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break;
    case GST_QUERY_RATE:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break;
    case GST_QUERY_SEEKING:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break;
    case GST_QUERY_SEGMENT:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break; 
    case GST_QUERY_CONVERT:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break; 
    case GST_QUERY_FORMATS:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break;
    case GST_QUERY_BUFFERING:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break; 
    case GST_QUERY_CUSTOM:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break; 
    case GST_QUERY_URI :
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break;  
    case GST_QUERY_ALLOCATION :
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break;
    case GST_QUERY_SCHEDULING:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break; 
    case GST_QUERY_ACCEPT_CAPS:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break;
    case GST_QUERY_CAPS  :
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);    
        break; 
    case GST_QUERY_DRAIN:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break; 
    case GST_QUERY_CONTEXT :
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break;
    case GST_QUERY_BITRATE:
        GST_DEBUG_OBJECT (mysrc, "query %d", __LINE__);
        break;
#endif

    default:
        GST_DEBUG_OBJECT (mysrc, "query (HiT Default) %d", __LINE__);

        /* just call the default handler */
        ret = GST_BASE_SRC_CLASS (parent_class)->query (src, query);
        break;
    }
  
    return ret;
}

/* notify subclasses of an event */
static gboolean
gst_mysrc_event (GstBaseSrc * src, GstEvent * event)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "event");

    return TRUE;
}

/* ask the subclass to create a buffer with offset and size, the default
 * implementation will call alloc and fill. */
static GstFlowReturn
gst_mysrc_create (GstBaseSrc * src, guint64 offset, guint size,
                  GstBuffer ** buf)
{
    GstMysrc *mysrc = GST_MYSRC (src);
    GstBuffer *thisbuf;
    gpointer data;
  
    GST_DEBUG_OBJECT (mysrc, "create");
    GST_DEBUG_OBJECT (mysrc, "buffer req size %d, offset %ld", size, offset);


    // Allocate a new buffer representation, this is passed between elements, note - no actual usable buffer memory yet....
    thisbuf = gst_buffer_new();
    if (size != 0)
    {
        guint8 *ptr;

        // And now you add the memory block 
        data = g_malloc (size);
        if (NULL != data)
        {
            gst_buffer_append_memory (thisbuf,
                                      gst_memory_new_wrapped (0, data, size, 0, size, data, g_free));
          
            // fake data - set first byte incrementing counter
            ptr = data;
            *ptr = mysrc->counter++;
            GST_DEBUG_OBJECT (mysrc, "buffer[0] = %d", *ptr);
            GST_INFO("buffer[0] = 0x%02X", *ptr);
        }
        else
        {
            // TODO Some real error handling
            GST_INFO("ERROR Failed to malloc buffer");

            // bail !!
            return GST_FLOW_ERROR;
        }
    }
  
    GST_BUFFER_OFFSET (thisbuf) = offset;
    *buf = thisbuf;

    return GST_FLOW_OK;
}

/* ask the subclass to allocate an output buffer. The default implementation
 * will use the negotiated allocator. */
static GstFlowReturn
gst_mysrc_alloc (GstBaseSrc * src, guint64 offset, guint size,
                 GstBuffer ** buf)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "alloc");

    return GST_FLOW_OK;
}

/* ask the subclass to fill the buffer with data from offset and size */
static GstFlowReturn
gst_mysrc_fill (GstBaseSrc * src, guint64 offset, guint size, GstBuffer * buf)
{
    GstMysrc *mysrc = GST_MYSRC (src);

    GST_DEBUG_OBJECT (mysrc, "fill");

    return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

    /* FIXME Remember to set the rank if it's an element that is meant
       to be autoplugged by decodebin. */
    return gst_element_register (plugin, "mysrc", GST_RANK_NONE,
                                 GST_TYPE_MYSRC);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.0.FIXME"
#endif
#ifndef PACKAGE
#define PACKAGE "FIXME_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "FIXME_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
                   GST_VERSION_MINOR,
                   mysrc,
                   "FIXME mysrc description",
                   plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

