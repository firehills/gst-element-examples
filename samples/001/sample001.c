#include <gst/gst.h>

int
main (int argc, char *argv[])
{
    GstElement *pipeline;
    GstElement *source;
    GstElement *transform;
    GstElement *sink;
    gboolean terminate = FALSE;
    
    GstBus *bus;
    GstMessage *msg;
    
    /* Initialize GStreamer */
    gst_init (&argc, &argv);


    /* Create the elements */
    source    = gst_element_factory_make ("mysrc", "custom_source_name");
    transform = gst_element_factory_make ("mytransform", "custom_transform_name");
    sink      = gst_element_factory_make ("mysink", "custom_sink_name");
    
    /* Build the pipeline */
    pipeline = gst_pipeline_new ("test-pipeline");
    if (!pipeline || !source || !transform || !sink)
    {
        g_printerr ("Not all elements could be created.\n");
        return -1;
    }   

    /* Build the pipeline */
    gst_bin_add_many (GST_BIN (pipeline), source, transform, sink, NULL);
    if (gst_element_link (source, transform) != TRUE) {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref (pipeline);
        return -1;
    }

    if (gst_element_link (transform, sink) != TRUE) {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref (pipeline);
        return -1;
    }

    /* Modify the source's properties */
    g_object_set (source, "num-buffers", 5, NULL);
    

    /* Start playing */
    gst_element_set_state (pipeline, GST_STATE_PLAYING);

    /* Wait until error or EOS */
    bus = gst_element_get_bus (pipeline);


    /* Capture messages whilst executing */
    do
    {
        msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
                                          GST_MESSAGE_STATE_CHANGED |
                                          GST_MESSAGE_ERROR |
                                          GST_MESSAGE_EOS);
    
        /* Parse message */
        if (msg != NULL) {
            GError *err;
            gchar *debug_info;
        
            switch (GST_MESSAGE_TYPE (msg)) {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error (msg, &err, &debug_info);
                g_printerr ("Error received from element %s: %s\n",
                            GST_OBJECT_NAME (msg->src), err->message);
                g_printerr ("Debugging information: %s\n",
                            debug_info ? debug_info : "none");
                g_clear_error (&err);
                g_free (debug_info);
                terminate = TRUE;
                break;
            case GST_MESSAGE_EOS:
                g_print ("End-Of-Stream reached.\n");
                terminate = TRUE;
                break;
            case GST_MESSAGE_STATE_CHANGED:

                // This line just debug... 
                g_print ("MSG SeqNum=%d From '%s' type=%d\n", msg->seqnum, msg->src->name, msg->type);

                /* We are only really interested in state-changed messages from the pipeline */
                if (GST_MESSAGE_SRC (msg) == GST_OBJECT (pipeline)) {
                    GstState old_state, new_state, pending_state;
                    gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
                    g_print ("Pipeline state changed from %s to %s:\n",
                             gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
                }
                break;
            default:
                /* We should not reach here because we only asked for ERRORs and EOS */
                g_printerr ("Unexpected message received.\n");
                break;
            }
            gst_message_unref (msg);
        }
    } while (!terminate);
    
    
    /* Free resources */
    gst_object_unref (bus);
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);
    return 0;
}
