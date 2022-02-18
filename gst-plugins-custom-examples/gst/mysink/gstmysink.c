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
 * SECTION:element-gstmysink
 *
 * The mysink element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! mysink ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasesink.h>
#include "gstmysink.h"

GST_DEBUG_CATEGORY_STATIC (gst_mysink_debug_category);
#define GST_CAT_DEFAULT gst_mysink_debug_category

/* prototypes */


static void gst_mysink_set_property (GObject * object,
                                     guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_mysink_get_property (GObject * object,
                                     guint property_id, GValue * value, GParamSpec * pspec);
static void gst_mysink_dispose (GObject * object);
static void gst_mysink_finalize (GObject * object);

static GstCaps *gst_mysink_get_caps (GstBaseSink * sink, GstCaps * filter);
static gboolean gst_mysink_set_caps (GstBaseSink * sink, GstCaps * caps);
static GstCaps *gst_mysink_fixate (GstBaseSink * sink, GstCaps * caps);
static gboolean gst_mysink_activate_pull (GstBaseSink * sink, gboolean active);
static void gst_mysink_get_times (GstBaseSink * sink, GstBuffer * buffer,
                                  GstClockTime * start, GstClockTime * end);
static gboolean gst_mysink_propose_allocation (GstBaseSink * sink,
                                               GstQuery * query);
static gboolean gst_mysink_start (GstBaseSink * sink);
static gboolean gst_mysink_stop (GstBaseSink * sink);
static gboolean gst_mysink_unlock (GstBaseSink * sink);
static gboolean gst_mysink_unlock_stop (GstBaseSink * sink);
static gboolean gst_mysink_query (GstBaseSink * sink, GstQuery * query);
static gboolean gst_mysink_event (GstBaseSink * sink, GstEvent * event);
static GstFlowReturn gst_mysink_wait_event (GstBaseSink * sink,
                                            GstEvent * event);
static GstFlowReturn gst_mysink_prepare (GstBaseSink * sink,
                                         GstBuffer * buffer);
static GstFlowReturn gst_mysink_prepare_list (GstBaseSink * sink,
                                              GstBufferList * buffer_list);
static GstFlowReturn gst_mysink_preroll (GstBaseSink * sink,
                                         GstBuffer * buffer);
static GstFlowReturn gst_mysink_render (GstBaseSink * sink,
                                        GstBuffer * buffer);
static GstFlowReturn gst_mysink_render_list (GstBaseSink * sink,
                                             GstBufferList * buffer_list);

enum
    {
        PROP_0
    };

/* pad templates */

static GstStaticPadTemplate gst_mysink_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
                             GST_PAD_SINK,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("application/buffer")
                             );


#define gst_mysink_parent_class parent_class


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstMysink, gst_mysink, GST_TYPE_BASE_SINK,
                         GST_DEBUG_CATEGORY_INIT (gst_mysink_debug_category, "mysink", 0,
                                                  "debug category for mysink element"));






static void
gst_mysink_class_init (GstMysinkClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstBaseSinkClass *base_sink_class = GST_BASE_SINK_CLASS (klass);

    /* Setting up pads and setting metadata should be moved to
       base_class_init if you intend to subclass this class. */
    gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
                                               &gst_mysink_sink_template);

    gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
                                           "mysink -> Long name ", "Generic", "A mysink long description",
                                           "Philip Hill <email@gmail.com>");

    gobject_class->set_property = gst_mysink_set_property;
    gobject_class->get_property = gst_mysink_get_property;
    gobject_class->dispose = gst_mysink_dispose;
    gobject_class->finalize = gst_mysink_finalize;
    base_sink_class->get_caps = GST_DEBUG_FUNCPTR (gst_mysink_get_caps);
    base_sink_class->set_caps = GST_DEBUG_FUNCPTR (gst_mysink_set_caps);
    base_sink_class->fixate = GST_DEBUG_FUNCPTR (gst_mysink_fixate);
    base_sink_class->activate_pull = GST_DEBUG_FUNCPTR (gst_mysink_activate_pull);
    base_sink_class->get_times = GST_DEBUG_FUNCPTR (gst_mysink_get_times);
    base_sink_class->propose_allocation = GST_DEBUG_FUNCPTR (gst_mysink_propose_allocation);
    base_sink_class->start = GST_DEBUG_FUNCPTR (gst_mysink_start);
    base_sink_class->stop = GST_DEBUG_FUNCPTR (gst_mysink_stop);
    base_sink_class->unlock = GST_DEBUG_FUNCPTR (gst_mysink_unlock);
    base_sink_class->unlock_stop = GST_DEBUG_FUNCPTR (gst_mysink_unlock_stop);
    base_sink_class->query = GST_DEBUG_FUNCPTR (gst_mysink_query);
    base_sink_class->event = GST_DEBUG_FUNCPTR (gst_mysink_event);
    base_sink_class->wait_event = GST_DEBUG_FUNCPTR (gst_mysink_wait_event);
    base_sink_class->prepare = GST_DEBUG_FUNCPTR (gst_mysink_prepare);
    base_sink_class->prepare_list = GST_DEBUG_FUNCPTR (gst_mysink_prepare_list);
    base_sink_class->preroll = GST_DEBUG_FUNCPTR (gst_mysink_preroll);
    base_sink_class->render = GST_DEBUG_FUNCPTR (gst_mysink_render);
    base_sink_class->render_list = GST_DEBUG_FUNCPTR (gst_mysink_render_list);

}

static void
gst_mysink_init (GstMysink *mysink)
{
}

void
gst_mysink_set_property (GObject * object, guint property_id,
                         const GValue * value, GParamSpec * pspec)
{
    GstMysink *mysink = GST_MYSINK (object);

    GST_DEBUG_OBJECT (mysink, "set_property");

    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

void
gst_mysink_get_property (GObject * object, guint property_id,
                         GValue * value, GParamSpec * pspec)
{
    GstMysink *mysink = GST_MYSINK (object);

    GST_DEBUG_OBJECT (mysink, "get_property");

    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

void
gst_mysink_dispose (GObject * object)
{
    GstMysink *mysink = GST_MYSINK (object);

    GST_DEBUG_OBJECT (mysink, "dispose");

    /* clean up as possible.  may be called multiple times */

    G_OBJECT_CLASS (gst_mysink_parent_class)->dispose (object);
}

void
gst_mysink_finalize (GObject * object)
{
    GstMysink *mysink = GST_MYSINK (object);

    GST_DEBUG_OBJECT (mysink, "finalize");

    /* clean up object here */

    G_OBJECT_CLASS (gst_mysink_parent_class)->finalize (object);
}

static GstCaps *
gst_mysink_get_caps (GstBaseSink * sink, GstCaps * filter)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "get_caps");

    return NULL;
}

/* notify subclass of new caps */
static gboolean
gst_mysink_set_caps (GstBaseSink * sink, GstCaps * caps)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "set_caps");

    return TRUE;
}

/* fixate sink caps during pull-mode negotiation */
static GstCaps *
gst_mysink_fixate (GstBaseSink * sink, GstCaps * caps)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "fixate");

    return NULL;
}

/* start or stop a pulling thread */
static gboolean
gst_mysink_activate_pull (GstBaseSink * sink, gboolean active)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "activate_pull");

    return TRUE;
}

/* get the start and end times for syncing on this buffer */
static void
gst_mysink_get_times (GstBaseSink * sink, GstBuffer * buffer,
                      GstClockTime * start, GstClockTime * end)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "get_times");

}

/* propose allocation parameters for upstream */
static gboolean
gst_mysink_propose_allocation (GstBaseSink * sink, GstQuery * query)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "propose_allocation");

    return TRUE;
}

/* start and stop processing, ideal for opening/closing the resource */
static gboolean
gst_mysink_start (GstBaseSink * sink)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "start");

    return TRUE;
}

static gboolean
gst_mysink_stop (GstBaseSink * sink)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "stop");

    return TRUE;
}

/* unlock any pending access to the resource. subclasses should unlock
 * any function ASAP. */
static gboolean
gst_mysink_unlock (GstBaseSink * sink)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "unlock");

    return TRUE;
}

/* Clear a previously indicated unlock request not that unlocking is
 * complete. Sub-classes should clear any command queue or indicator they
 * set during unlock */
static gboolean
gst_mysink_unlock_stop (GstBaseSink * sink)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "unlock_stop");

    return TRUE;
}

/* notify subclass of query */
static gboolean
gst_mysink_query (GstBaseSink * sink, GstQuery * query)
{
    GstMysink *mysink = GST_MYSINK (sink);
    gboolean ret = TRUE;
    GST_DEBUG_OBJECT (mysink, "query");


    switch(GST_QUERY_TYPE(query)) {
#if 0
    case GST_QUERY_UNKNOWN:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break;
    case GST_QUERY_POSITION :
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break;
    case GST_QUERY_DURATION:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break;
    case GST_QUERY_LATENCY:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break;
    case GST_QUERY_JITTER :
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break;
    case GST_QUERY_RATE:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break;
    case GST_QUERY_SEEKING:{
      
      
        GstFormat fmt;
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        /* we don't supporting seeking */
        gst_query_parse_seeking (query, &fmt, NULL, NULL, NULL);
        gst_query_set_seeking (query, fmt, FALSE, 0, -1);
        ret = TRUE;
      
        break;
    }
    case GST_QUERY_SEGMENT:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break; 
    case GST_QUERY_CONVERT:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break; 
    case GST_QUERY_FORMATS:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break;
    case GST_QUERY_BUFFERING:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break; 
    case GST_QUERY_CUSTOM:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break; 
    case GST_QUERY_URI :
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break;  
    case GST_QUERY_ALLOCATION :
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break;
    case GST_QUERY_SCHEDULING:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break; 
    case GST_QUERY_ACCEPT_CAPS:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break;

        /*
          case GST_QUERY_CAPS  :
          GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
          GST_DEBUG_OBJECT (mysink, "GST_QUERY_CAPS");

      
          break;
        */ 
    case GST_QUERY_DRAIN:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break; 
    case GST_QUERY_CONTEXT :
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break;
    case GST_QUERY_BITRATE:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
        break;
#endif

    default:
        GST_DEBUG_OBJECT (mysink, "query %d", __LINE__);
      
        /* just call the default handler */
        ret = GST_BASE_SINK_CLASS (parent_class)->query (sink, query);
        break;
    }
  
    return ret;
}

/* notify subclass of event */
static gboolean
gst_mysink_event (GstBaseSink * sink, GstEvent * event)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "event");

    return TRUE;
}

/* wait for eos or gap, subclasses should chain up to parent first */
static GstFlowReturn
gst_mysink_wait_event (GstBaseSink * sink, GstEvent * event)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "wait_event");

    return GST_FLOW_OK;
}

/* notify subclass of buffer or list before doing sync */
static GstFlowReturn
gst_mysink_prepare (GstBaseSink * sink, GstBuffer * buffer)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "prepare");

    return GST_FLOW_OK;
}

static GstFlowReturn
gst_mysink_prepare_list (GstBaseSink * sink, GstBufferList * buffer_list)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "prepare_list");

    return GST_FLOW_OK;
}

/* notify subclass of preroll buffer or real buffer */
static GstFlowReturn
gst_mysink_preroll (GstBaseSink * sink, GstBuffer * buffer)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "preroll");

    return GST_FLOW_OK;
}

static GstFlowReturn
gst_mysink_render (GstBaseSink * sink, GstBuffer * buffer)
{
    GstMysink *mysink = GST_MYSINK (sink);
    GstMemory* mem;
    guint num_blocks;
    GstMapInfo info;
    GST_DEBUG_OBJECT (mysink, "render");


    // GstBuffer can have a memory block, and/or an array of memory blocks
  
    // An Array? how many memory blocks?
    num_blocks = gst_buffer_n_memory(buffer);
    GST_DEBUG_OBJECT (mysink, "render has %d mem block(s)", num_blocks);
  
    if (num_blocks > 1) // multiple buffers otherwise if 1, use buffer_map
    {
        gsize size;
        gsize offset;
        gsize maxsize;
      
        // peek at value of 1st byte in 1st block
        mem = gst_buffer_peek_memory(buffer, 0);
        size = gst_memory_get_sizes(mem, &offset, &maxsize);
        GST_DEBUG_OBJECT (mysink, "render Buf[0] size %ld, offset %ld, maxsize %ld", size, offset, maxsize);
     
        if(gst_memory_map(mem, &info, GST_MAP_READ))
        {
            GST_DEBUG_OBJECT (mysink, "Info  [0] size %ld", info.size);
            GST_DEBUG_OBJECT (mysink, "render[0] Value = 0x%x", info.data[0]);
            GST_INFO("buffer[0] = 0x%02X", info.data[0]);
            gst_memory_unmap(mem, &info);
        }      
    }

#if 1
    // Or just a buffer
    if (gst_buffer_map(buffer, &info,  GST_MAP_READ))
    {
        GST_DEBUG_OBJECT (mysink, "Info BUF size %ld", info.size);
        if (info.size > 0)
        {
            GST_DEBUG_OBJECT (mysink, "render BUF Value = 0x%x", info.data[0]);
            GST_INFO("buffer[0] = 0x%02X", info.data[0]);
        }
        gst_buffer_unmap(buffer, &info); 
    }      
#endif
  
    return GST_FLOW_OK;
}

/* Render a BufferList */
static GstFlowReturn
gst_mysink_render_list (GstBaseSink * sink, GstBufferList * buffer_list)
{
    GstMysink *mysink = GST_MYSINK (sink);

    GST_DEBUG_OBJECT (mysink, "render_list");

    return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

    /* FIXME Remember to set the rank if it's an element that is meant
       to be autoplugged by decodebin. */
    return gst_element_register (plugin, "mysink", GST_RANK_NONE,
                                 GST_TYPE_MYSINK);
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
                   mysink,
                   "FIXME plugin description",
                   plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

