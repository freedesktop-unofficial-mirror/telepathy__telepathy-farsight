/*
 * session.c - Source for TfSession
 * Copyright (C) 2006-2007 Collabora Ltd.
 * Copyright (C) 2006-2007 Nokia Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <telepathy-glib/dbus.h>
#include <telepathy-glib/errors.h>
#include <telepathy-glib/interfaces.h>

#include <gst/farsight/fs-conference-iface.h>

#include "session-priv.h"
#include "tf-signals-marshal.h"

G_DEFINE_TYPE (TfSession, _tf_session, G_TYPE_OBJECT);

struct _TfSessionPrivate
{
  GError *construction_error;

  gchar *session_type;
  FsConference *fs_conference;
  FsParticipant *fs_participant;

  TpMediaSessionHandler *session_handler_proxy;
};

enum
{
  PROP_PROXY = 1,
  PROP_SESSION_TYPE,
  PROP_FARSIGHT_CONFERENCE,
  PROP_FARSIGHT_PARTICIPANT,
};

enum
{
  NEW_STREAM,
  INVALIDATED,
  SIGNAL_COUNT
};

static guint signals[SIGNAL_COUNT] = { 0 };

static void
_tf_session_init (TfSession *self)
{
  TfSessionPrivate *priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
      TF_TYPE_SESSION, TfSessionPrivate);

  self->priv = priv;
}

static void
_tf_session_get_property (GObject    *object,
    guint       property_id,
    GValue     *value,
    GParamSpec *pspec)
{
  TfSession *self = TF_SESSION (object);

  switch (property_id)
    {
    case PROP_SESSION_TYPE:
      g_value_set_string (value, self->priv->session_type);
      break;
    case PROP_FARSIGHT_CONFERENCE:
      g_value_set_object (value, self->priv->fs_conference);
      break;
    case PROP_FARSIGHT_PARTICIPANT:
      g_value_set_object (value, self->priv->fs_participant);
      break;
    case PROP_PROXY:
      g_value_set_object (value, self->priv->session_handler_proxy);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
_tf_session_set_property (GObject      *object,
    guint         property_id,
    const GValue *value,
    GParamSpec   *pspec)
{
  TfSession *self = TF_SESSION (object);

  switch (property_id)
    {
    case PROP_SESSION_TYPE:
      self->priv->session_type = g_value_dup_string (value);
      break;
    case PROP_PROXY:
      self->priv->session_handler_proxy =
          TP_MEDIA_SESSION_HANDLER (g_value_dup_object (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void new_media_stream_handler (TpMediaSessionHandler *proxy,
    const gchar *stream_handler_path, guint id, guint media_type,
    guint direction, gpointer user_data, GObject *object);

static void dummy_callback (TpMediaSessionHandler *proxy, const GError *error,
    gpointer user_data, GObject *object);

static void invalidated_cb (TpMediaSessionHandler *proxy, guint domain,
    gint code, gchar *message, gpointer user_data);

static GObject *
_tf_session_constructor (GType type,
    guint n_props,
    GObjectConstructParam *props)
{
  GObject *obj;
  TfSession *self;
  gchar *conftype;
  GstElement *conf;

  obj = G_OBJECT_CLASS (_tf_session_parent_class)->
           constructor (type, n_props, props);
  self = (TfSession *) obj;

  conftype = g_strdup_printf ("fs%sconference", self->priv->session_type);
  conf = gst_element_factory_make (conftype, NULL);
  g_free (conftype);

  if (!conf)
    {
      self->priv->construction_error = g_error_new (FS_ERROR,
          FS_ERROR_CONSTRUCTION, "Invalid session type");
      return obj;
    }

  self->priv->fs_conference = FS_CONFERENCE (gst_object_ref (conf));

  self->priv->fs_participant =
      fs_conference_new_participant (self->priv->fs_conference,
          "whatever-cname@1.2.3.4",
          &self->priv->construction_error);

  if (!self->priv->fs_participant)
    return obj;

  g_signal_connect (self->priv->session_handler_proxy, "invalidated",
      G_CALLBACK (invalidated_cb), obj);


  tp_cli_media_session_handler_connect_to_new_stream_handler
      (self->priv->session_handler_proxy, new_media_stream_handler, NULL, NULL,
       obj, NULL);

  g_debug ("calling MediaSessionHandler::Ready");

  tp_cli_media_session_handler_call_ready (self->priv->session_handler_proxy,
      -1, dummy_callback, "Media.SessionHandler::Ready", NULL, NULL);

  return obj;
}

static void
_tf_session_dispose (GObject *object)
{
  TfSession *self = TF_SESSION (object);

  g_debug (G_STRFUNC);

  if (self->priv->session_handler_proxy)
    {
      TpMediaSessionHandler *tmp;

      g_signal_handlers_disconnect_by_func (
          self->priv->session_handler_proxy, invalidated_cb, self);

      tmp = self->priv->session_handler_proxy;
      self->priv->session_handler_proxy = NULL;
      g_object_unref (tmp);
    }

  if (self->priv->fs_participant)
    {
      g_object_unref (self->priv->fs_participant);
      self->priv->fs_participant = NULL;
    }

  if (self->priv->fs_conference)
    {

      gst_object_unref (self->priv->fs_conference);
      self->priv->fs_conference = NULL;
    }

  g_free (self->priv->session_type);
  self->priv->session_type = NULL;

  if (G_OBJECT_CLASS (_tf_session_parent_class)->dispose)
    G_OBJECT_CLASS (_tf_session_parent_class)->dispose (object);
}

static void
_tf_session_class_init (TfSessionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (TfSessionPrivate));

  object_class->set_property = _tf_session_set_property;
  object_class->get_property = _tf_session_get_property;

  object_class->constructor = _tf_session_constructor;

  object_class->dispose = _tf_session_dispose;

  g_object_class_install_property (object_class, PROP_SESSION_TYPE,
      g_param_spec_string ("conference-type",
          "Farsight conference type",
          "Name of the Farsight conference type this "
          "session will create (rtp, msn, etc).",
          NULL,
          G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_FARSIGHT_CONFERENCE,
      g_param_spec_object ("farsight-conference",
          "Farsight conference",
          "The Farsight conference to add to the pipeline",
          FS_TYPE_CONFERENCE,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_FARSIGHT_PARTICIPANT,
      g_param_spec_object ("farsight-participant",
          "Farsight participant",
          "The Farsight participant for this session",
          FS_TYPE_PARTICIPANT,
          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_PROXY,
      g_param_spec_object ("proxy", "TpMediaSessionHandler proxy",
          "The session handler proxy which this session interacts with.",
          TP_TYPE_MEDIA_SESSION_HANDLER,
          G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * TfSession::new-stream:
   * @session: the #TfSession which has a new stream
   * @object_path: The object-path of the new stream
   * @stream_id: The stream id of the new strema
   * @media_type: The media type of the new stream
   * @direction: The direction of the new stream
   *
   * This is emitted when a new stream is created and should only be used
   * by the #TfChannel.
   * One should connect to the #TfChannel::channel-new-stream signal
   * instead.
   */

  signals[NEW_STREAM] =
      g_signal_new ("new-stream",
          G_OBJECT_CLASS_TYPE (klass),
          G_SIGNAL_RUN_LAST,
          0,
          NULL, NULL,
          _tf_marshal_VOID__BOXED_UINT_UINT_UINT,
          G_TYPE_NONE, 4,
          DBUS_TYPE_G_OBJECT_PATH, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT);
  signals[INVALIDATED] =
      g_signal_new ("invalidated",
          G_OBJECT_CLASS_TYPE (klass),
          G_SIGNAL_RUN_LAST,
          0,
          NULL, NULL,
          g_cclosure_marshal_VOID__VOID,
          G_TYPE_NONE, 0);
}

/* dummy callback handler for async calling calls with no return values */
static void
dummy_callback (TpMediaSessionHandler *proxy G_GNUC_UNUSED,
    const GError *error,
    gpointer user_data,
    GObject *weak_object G_GNUC_UNUSED)
{
  if (error != NULL)
    {
      g_warning ("Error calling %s: %s", (gchar *) user_data, error->message);
    }
}

static void
invalidated_cb (TpMediaSessionHandler *proxy G_GNUC_UNUSED,
    guint domain G_GNUC_UNUSED,
    gint code G_GNUC_UNUSED,
    gchar *message G_GNUC_UNUSED,
    gpointer user_data)
{
  TfSession *self = TF_SESSION (user_data);

  if (self->priv->session_handler_proxy)
    {
      TpMediaSessionHandler *tmp;

      tmp = self->priv->session_handler_proxy;
      self->priv->session_handler_proxy = NULL;
      g_object_unref (tmp);
    }

  g_signal_emit (self, signals[INVALIDATED], 0);
}

static void
new_media_stream_handler (TpMediaSessionHandler *proxy G_GNUC_UNUSED,
    const gchar *object_path,
    guint stream_id,
    guint media_type,
    guint direction,
    gpointer user_data G_GNUC_UNUSED,
    GObject *object)
{
  TfSession *self = TF_SESSION (object);

  g_debug ("New stream, stream_id=%d, media_type=%d, direction=%d",
      stream_id, media_type, direction);

  g_signal_emit (self, signals[NEW_STREAM], 0, object_path, stream_id,
      media_type, direction);
}

TfSession *
_tf_session_new (TpMediaSessionHandler *proxy,
    const gchar *conference_type,
    GError **error)
{
  TfSession *self;

  g_return_val_if_fail (proxy != NULL, NULL);
  g_return_val_if_fail (conference_type != NULL, NULL);

  self = g_object_new (TF_TYPE_SESSION,
      "proxy", proxy,
      "conference-type", conference_type,
      NULL);

  if (self->priv->construction_error)
    {
      g_propagate_error (error, self->priv->construction_error);
      g_object_unref (self);
      return NULL;
    }

  return self;
}

/**
 * _tf_session_bus_message:
 * @session: A #TfSession
 * @message: A #GstMessage received from the bus
 *
 * You must call this function on call messages received on the async bus.
 * #GstMessages are not modified.
 *
 * Returns: %TRUE if the message has been handled, %FALSE otherwise
 */

gboolean
_tf_session_bus_message (TfSession *session,
    GstMessage *message)
{
  GError *error = NULL;
  gchar *debug = NULL;

  if (GST_MESSAGE_SRC (message) !=
      GST_OBJECT_CAST (session->priv->fs_conference))
    return FALSE;

  switch (GST_MESSAGE_TYPE (message))
    {
    case GST_MESSAGE_WARNING:
      gst_message_parse_warning (message, &error, &debug);

      g_warning ("session: %s (%s)", error->message, debug);

      g_error_free (error);
      g_free (debug);
      return TRUE;
    case GST_MESSAGE_ERROR:
      gst_message_parse_error (message, &error, &debug);

      g_warning ("session ERROR: %s (%s)", error->message, debug);

      tp_cli_media_session_handler_call_error (
          session->priv->session_handler_proxy,
          -1, 0, /* Not errors defined ??? */
          error->message, NULL, /* Do I need a callback ? */
          NULL, NULL, NULL);

      g_error_free (error);
      g_free (debug);
      return TRUE;
    case GST_MESSAGE_ELEMENT:
      {
        const GstStructure *s = gst_message_get_structure (message);

        if (gst_structure_has_name (s, "farsight-error"))
          {
            GObject *object;
            const GValue *value = NULL;

            value = gst_structure_get_value (s, "src-object");
            object = g_value_get_object (value);

            if (object == G_OBJECT (session->priv->fs_participant))
              {
                const gchar *msg, *debugmsg;
                FsError errorno;
                GEnumClass *enumclass;
                GEnumValue *enumvalue;

                value = gst_structure_get_value (s, "error-no");
                errorno = g_value_get_enum (value);
                msg = gst_structure_get_string (s, "error-msg");
                debugmsg = gst_structure_get_string (s, "debug-msg");


                enumclass = g_type_class_ref (FS_TYPE_ERROR);
                enumvalue = g_enum_get_value (enumclass, errorno);
                g_warning ("participant error (%s (%d)): %s : %s",
                    enumvalue->value_nick, errorno, msg, debugmsg);
                g_type_class_unref (enumclass);

                tp_cli_media_session_handler_call_error (
                    session->priv->session_handler_proxy,
                    -1, 0, msg, NULL, NULL, NULL, NULL);
                return TRUE;
              }
          }
      }
    default:
      return FALSE;
    }
}
