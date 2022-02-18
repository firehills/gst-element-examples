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

#ifndef _GST_MYTRANSFORM_H_
#define _GST_MYTRANSFORM_H_

#include <gst/base/gstbasetransform.h>

G_BEGIN_DECLS

#define GST_TYPE_MYTRANSFORM   (gst_mytransform_get_type())
#define GST_MYTRANSFORM(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MYTRANSFORM,GstMytransform))
#define GST_MYTRANSFORM_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_MYTRANSFORM,GstMytransformClass))
#define GST_IS_MYTRANSFORM(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_MYTRANSFORM))
#define GST_IS_MYTRANSFORM_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_MYTRANSFORM))

typedef struct _GstMytransform GstMytransform;
typedef struct _GstMytransformClass GstMytransformClass;

struct _GstMytransform
{
    GstBaseTransform base_mytransform;
    GstPad * sinkpad;
    GstPad * srcpad;
};

struct _GstMytransformClass
{
  GstBaseTransformClass base_mytransform_class;
};

GType gst_mytransform_get_type (void);

G_END_DECLS

#endif
