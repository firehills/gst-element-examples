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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_MYSRC_H_
#define _GST_MYSRC_H_

#include <gst/base/gstbasesrc.h>

G_BEGIN_DECLS

#define GST_TYPE_MYSRC   (gst_mysrc_get_type())
#define GST_MYSRC(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MYSRC,GstMysrc))
#define GST_MYSRC_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_MYSRC,GstMysrcClass))
#define GST_IS_MYSRC(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_MYSRC))
#define GST_IS_MYSRC_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_MYSRC))

typedef struct _GstMysrc GstMysrc;
typedef struct _GstMysrcClass GstMysrcClass;

struct _GstMysrc
{
  GstBaseSrc base_mysrc;

    // props
    gint prop0;
    gchar* prop1;

    //internals
    pthread_t recv_thread;
    GstPad* srcpad;
    guint8 counter;
};

struct _GstMysrcClass
{
  GstBaseSrcClass base_mysrc_class;
};

GType gst_mysrc_get_type (void);

G_END_DECLS

#endif
