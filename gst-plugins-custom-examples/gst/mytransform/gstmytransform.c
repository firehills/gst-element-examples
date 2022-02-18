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
 * SECTION:element-gstmytransform
 *
 * The mytransform element just clears MSB nibble from 1st element in the buffer
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! mytransform ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gstmytransform.h"

GST_DEBUG_CATEGORY_STATIC (gst_mytransform_debug_category);
#define GST_CAT_DEFAULT gst_mytransform_debug_category

/* prototypes */


static void gst_mytransform_set_property (GObject * object,
                                          guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_mytransform_get_property (GObject * object,
                                          guint property_id, GValue * value, GParamSpec * pspec);
static void gst_mytransform_dispose (GObject * object);
static void gst_mytransform_finalize (GObject * object);

static GstCaps *gst_mytransform_transform_caps (GstBaseTransform * trans,
                                                GstPadDirection direction, GstCaps * caps, GstCaps * filter);
static GstCaps *gst_mytransform_fixate_caps (GstBaseTransform * trans,
                                             GstPadDirection direction, GstCaps * caps, GstCaps * othercaps);
static gboolean gst_mytransform_accept_caps (GstBaseTransform * trans,
                                             GstPadDirection direction, GstCaps * caps);
static gboolean gst_mytransform_set_caps (GstBaseTransform * trans,
                                          GstCaps * incaps, GstCaps * outcaps);
static gboolean gst_mytransform_query (GstBaseTransform * trans,
                                       GstPadDirection direction, GstQuery * query);
static gboolean gst_mytransform_decide_allocation (GstBaseTransform * trans,
                                                   GstQuery * query);
static gboolean gst_mytransform_filter_meta (GstBaseTransform * trans,
                                             GstQuery * query, GType api, const GstStructure * params);
static gboolean gst_mytransform_propose_allocation (GstBaseTransform * trans,
                                                    GstQuery * decide_query, GstQuery * query);
static gboolean gst_mytransform_transform_size (GstBaseTransform * trans,
                                                GstPadDirection direction, GstCaps * caps, gsize size, GstCaps * othercaps,
                                                gsize * othersize);
static gboolean gst_mytransform_get_unit_size (GstBaseTransform * trans,
                                               GstCaps * caps, gsize * size);
static gboolean gst_mytransform_start (GstBaseTransform * trans);
static gboolean gst_mytransform_stop (GstBaseTransform * trans);
static gboolean gst_mytransform_sink_event (GstBaseTransform * trans,
                                            GstEvent * event);
static gboolean gst_mytransform_src_event (GstBaseTransform * trans,
                                           GstEvent * event);
static GstFlowReturn gst_mytransform_prepare_output_buffer (GstBaseTransform *
                                                            trans, GstBuffer * input, GstBuffer ** outbuf);
static gboolean gst_mytransform_copy_metadata (GstBaseTransform * trans,
                                               GstBuffer * input, GstBuffer * outbuf);
static gboolean gst_mytransform_transform_meta (GstBaseTransform * trans,
                                                GstBuffer * outbuf, GstMeta * meta, GstBuffer * inbuf);
static void gst_mytransform_before_transform (GstBaseTransform * trans,
                                              GstBuffer * buffer);
static GstFlowReturn gst_mytransform_transform (GstBaseTransform * trans,
                                                GstBuffer * inbuf, GstBuffer * outbuf);
static GstFlowReturn gst_mytransform_transform_ip (GstBaseTransform * trans,
                                                   GstBuffer * buf);

enum
    {
        PROP_0
    };

/* pad templates */

static GstStaticPadTemplate gst_mytransform_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
                             GST_PAD_SRC,
                             GST_PAD_ALWAYS, 
                             GST_STATIC_CAPS ("application/buffer") // Custom types or this example
                             );

static GstStaticPadTemplate gst_mytransform_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
                             GST_PAD_SINK,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS ("application/buffer") // Custom types or this example
                             );


/* class initialization */
#define gst_mytransform_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstMytransform, gst_mytransform, GST_TYPE_BASE_TRANSFORM,
                         GST_DEBUG_CATEGORY_INIT (gst_mytransform_debug_category, "mytransform", 0,
                                                  "debug category for mytransform element"));




static GstFlowReturn gst_mytransform_chain (GstPad    *pad,
                                            GstObject *parent,
                                            GstBuffer *buf)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (parent);
    GstMapInfo info;
    
    if (gst_buffer_map(buf, &info,  GST_MAP_WRITE))
    {
        GST_DEBUG_OBJECT (mytransform, "Info BUF size %ld", info.size);
        if (info.size > 0)
        {
            GST_INFO("Transforming (Strip top nibble) -> 0x%02X to 0x%02X",  info.data[0], info.data[0] & 0x0F);
            info.data[0] &= 0x0F;
        }
        gst_buffer_unmap(buf, &info); 
    }      

    // push out on src to next element
    return gst_pad_push (mytransform->srcpad, buf);
}



static void
gst_mytransform_class_init (GstMytransformClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);

   
  
    gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
                                           "FIXME mytransform Long name", "Generic", "FIXME mytransform Description",
                                           "Philip Hill email@gmail.com>");

    gobject_class->set_property = gst_mytransform_set_property;
    gobject_class->get_property = gst_mytransform_get_property;
    gobject_class->dispose = gst_mytransform_dispose;
    gobject_class->finalize = gst_mytransform_finalize;
    base_transform_class->transform_caps = GST_DEBUG_FUNCPTR (gst_mytransform_transform_caps);
    base_transform_class->fixate_caps = GST_DEBUG_FUNCPTR (gst_mytransform_fixate_caps);
    base_transform_class->accept_caps = GST_DEBUG_FUNCPTR (gst_mytransform_accept_caps);
    base_transform_class->set_caps = GST_DEBUG_FUNCPTR (gst_mytransform_set_caps);
    base_transform_class->query = GST_DEBUG_FUNCPTR (gst_mytransform_query);
    base_transform_class->decide_allocation = GST_DEBUG_FUNCPTR (gst_mytransform_decide_allocation);
    base_transform_class->filter_meta = GST_DEBUG_FUNCPTR (gst_mytransform_filter_meta);
    base_transform_class->propose_allocation = GST_DEBUG_FUNCPTR (gst_mytransform_propose_allocation);
    base_transform_class->transform_size = GST_DEBUG_FUNCPTR (gst_mytransform_transform_size);
    base_transform_class->get_unit_size = GST_DEBUG_FUNCPTR (gst_mytransform_get_unit_size);
    base_transform_class->start = GST_DEBUG_FUNCPTR (gst_mytransform_start);
    base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_mytransform_stop);
    base_transform_class->sink_event = GST_DEBUG_FUNCPTR (gst_mytransform_sink_event);
    base_transform_class->src_event = GST_DEBUG_FUNCPTR (gst_mytransform_src_event);
    base_transform_class->prepare_output_buffer = GST_DEBUG_FUNCPTR (gst_mytransform_prepare_output_buffer);
    base_transform_class->copy_metadata = GST_DEBUG_FUNCPTR (gst_mytransform_copy_metadata);
    base_transform_class->transform_meta = GST_DEBUG_FUNCPTR (gst_mytransform_transform_meta);
    base_transform_class->before_transform = GST_DEBUG_FUNCPTR (gst_mytransform_before_transform);
    base_transform_class->transform = GST_DEBUG_FUNCPTR (gst_mytransform_transform);
    base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_mytransform_transform_ip);

}

static void
gst_mytransform_init (GstMytransform *mytransform)
{


    // init called per INSTANCE
    
    
    /* pad through which data comes in to the element */
    mytransform->sinkpad = gst_pad_new_from_static_template (&gst_mytransform_sink_template, "sink");

    /* pads are configured here with gst_pad_set_*_function () */

    gst_element_add_pad (GST_ELEMENT (mytransform), mytransform->sinkpad);




    /* pad through which data exits the element */
    mytransform->srcpad = gst_pad_new_from_static_template (&gst_mytransform_src_template, "src");

    /* pads are configured here with gst_pad_set_*_function () */

    gst_element_add_pad (GST_ELEMENT (mytransform), mytransform->srcpad);
    


    
    // chain is where the ACTUAL transformational work happens
    gst_pad_set_chain_function (mytransform->sinkpad,
                                gst_mytransform_chain);

   
}

void
gst_mytransform_set_property (GObject * object, guint property_id,
                              const GValue * value, GParamSpec * pspec)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (object);

    GST_DEBUG_OBJECT (mytransform, "set_property");

    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

void
gst_mytransform_get_property (GObject * object, guint property_id,
                              GValue * value, GParamSpec * pspec)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (object);

    GST_DEBUG_OBJECT (mytransform, "get_property");

    switch (property_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

void
gst_mytransform_dispose (GObject * object)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (object);

    GST_DEBUG_OBJECT (mytransform, "dispose");

    /* clean up as possible.  may be called multiple times */

    G_OBJECT_CLASS (gst_mytransform_parent_class)->dispose (object);
}

void
gst_mytransform_finalize (GObject * object)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (object);

    GST_DEBUG_OBJECT (mytransform, "finalize");

    /* clean up object here */

    G_OBJECT_CLASS (gst_mytransform_parent_class)->finalize (object);
}

static GstCaps *
gst_mytransform_transform_caps (GstBaseTransform * trans, GstPadDirection direction,
                                GstCaps * caps, GstCaps * filter)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);
    GstCaps *othercaps;

    GST_DEBUG_OBJECT (mytransform, "transform_caps");

    othercaps = gst_caps_copy (caps);

    /* Copy other caps and modify as appropriate */
    /* This works for the simplest cases, where the transform modifies one
     * or more fields in the caps structure.  It does not work correctly
     * if passthrough caps are preferred. */
    if (direction == GST_PAD_SRC) {
        /* transform caps going upstream */
    } else {
        /* transform caps going downstream */
    }

    if (filter) {
        GstCaps *intersect;

        intersect = gst_caps_intersect (othercaps, filter);
        gst_caps_unref (othercaps);

        return intersect;
    } else {
        return othercaps;
    }
}

static GstCaps *
gst_mytransform_fixate_caps (GstBaseTransform * trans, GstPadDirection direction,
                             GstCaps * caps, GstCaps * othercaps)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "fixate_caps");

    return NULL;
}

static gboolean
gst_mytransform_accept_caps (GstBaseTransform * trans, GstPadDirection direction,
                             GstCaps * caps)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "accept_caps");

    return TRUE;
}

static gboolean
gst_mytransform_set_caps (GstBaseTransform * trans, GstCaps * incaps,
                          GstCaps * outcaps)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "set_caps");

    return TRUE;
}

static gboolean
gst_mytransform_query (GstBaseTransform * trans, GstPadDirection direction,
                       GstQuery * query)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);
    gboolean ret = TRUE;
    GST_DEBUG_OBJECT (mytransform, "query");


    //TODO check direction?
  
    switch(GST_QUERY_TYPE(query)) {


    default:
        GST_DEBUG_OBJECT (mytransform, "query %d", __LINE__);

        /* just call the default handler */
        ret = GST_BASE_TRANSFORM_CLASS (parent_class)->query (trans, direction, query);
        break;
    }


  
    return ret;
}

/* decide allocation query for output buffers */
static gboolean
gst_mytransform_decide_allocation (GstBaseTransform * trans, GstQuery * query)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "decide_allocation");

    return TRUE;
}

static gboolean
gst_mytransform_filter_meta (GstBaseTransform * trans, GstQuery * query, GType api,
                             const GstStructure * params)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "filter_meta");

    return TRUE;
}

/* propose allocation query parameters for input buffers */
static gboolean
gst_mytransform_propose_allocation (GstBaseTransform * trans,
                                    GstQuery * decide_query, GstQuery * query)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "propose_allocation");

    return TRUE;
}

/* transform size */
static gboolean
gst_mytransform_transform_size (GstBaseTransform * trans, GstPadDirection direction,
                                GstCaps * caps, gsize size, GstCaps * othercaps, gsize * othersize)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "transform_size");

    return TRUE;
}

static gboolean
gst_mytransform_get_unit_size (GstBaseTransform * trans, GstCaps * caps,
                               gsize * size)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "get_unit_size");

    return TRUE;
}

/* states */
static gboolean
gst_mytransform_start (GstBaseTransform * trans)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "start");

    return TRUE;
}

static gboolean
gst_mytransform_stop (GstBaseTransform * trans)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "stop");

    return TRUE;
}

/* sink and src pad event handlers */
static gboolean
gst_mytransform_sink_event (GstBaseTransform * trans, GstEvent * event)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "sink_event");

    return GST_BASE_TRANSFORM_CLASS (gst_mytransform_parent_class)->sink_event (
                                                                                trans, event);
}

static gboolean
gst_mytransform_src_event (GstBaseTransform * trans, GstEvent * event)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "src_event");

    return GST_BASE_TRANSFORM_CLASS (gst_mytransform_parent_class)->src_event (
                                                                               trans, event);
}

static GstFlowReturn
gst_mytransform_prepare_output_buffer (GstBaseTransform * trans, GstBuffer * input,
                                       GstBuffer ** outbuf)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "prepare_output_buffer");

    return GST_FLOW_OK;
}

/* metadata */
static gboolean
gst_mytransform_copy_metadata (GstBaseTransform * trans, GstBuffer * input,
                               GstBuffer * outbuf)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "copy_metadata");

    return TRUE;
}

static gboolean
gst_mytransform_transform_meta (GstBaseTransform * trans, GstBuffer * outbuf,
                                GstMeta * meta, GstBuffer * inbuf)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "transform_meta");

    return TRUE;
}

static void
gst_mytransform_before_transform (GstBaseTransform * trans, GstBuffer * buffer)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "before_transform");

}

/* transform */
static GstFlowReturn
gst_mytransform_transform (GstBaseTransform * trans, GstBuffer * inbuf,
                           GstBuffer * outbuf)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "transform");

    return GST_FLOW_OK;
}

static GstFlowReturn
gst_mytransform_transform_ip (GstBaseTransform * trans, GstBuffer * buf)
{
    GstMytransform *mytransform = GST_MYTRANSFORM (trans);

    GST_DEBUG_OBJECT (mytransform, "transform_ip");

    return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

    /* FIXME Remember to set the rank if it's an element that is meant
       to be autoplugged by decodebin. */
    return gst_element_register (plugin, "mytransform", GST_RANK_NONE,
                                 GST_TYPE_MYTRANSFORM);
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
                   mytransform,
                   "mytransform description",
                   plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

