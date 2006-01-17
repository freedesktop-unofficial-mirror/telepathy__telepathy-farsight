/*
 * tp-media-stream-handler.c - Source for TpMediaStreamHandler
 * Copyright (C) 2005 Collabora Ltd.
 * Copyright (C) 2005 Nokia Corporation
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

#include <dbus/dbus-glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "tp-media-stream-handler.h"
#include "tp-media-stream-handler-signals-marshal.h"

#include "tp-media-stream-handler-glue.h"

G_DEFINE_TYPE(TpMediaStreamHandler, tp_media_stream_handler, G_TYPE_OBJECT)

/* signal enum */
enum
{
    ADD_REMOTE_CANDIDATE,
    REMOVE_REMOTE_CANDIDATE,
    SET_ACTIVE_CANDIDATE_PAIR,
    SET_REMOTE_CANDIDATE_LIST,
    SET_REMOTE_CODECS,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0};

/* private structure */
typedef struct _TpMediaStreamHandlerPrivate TpMediaStreamHandlerPrivate;

struct _TpMediaStreamHandlerPrivate
{
  gboolean dispose_has_run;
};

#define TP_MEDIA_STREAM_HANDLER_GET_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), TP_TYPE_MEDIA_STREAM_HANDLER, TpMediaStreamHandlerPrivate))

static void
tp_media_stream_handler_init (TpMediaStreamHandler *obj)
{
  TpMediaStreamHandlerPrivate *priv = TP_MEDIA_STREAM_HANDLER_GET_PRIVATE (obj);

  /* allocate any data required by the object here */
  /* this line allows the plain autogenerated code to compile with -Werror
     remove it!*/
  g_assert(priv);
}

static void tp_media_stream_handler_dispose (GObject *object);
static void tp_media_stream_handler_finalize (GObject *object);

static void
tp_media_stream_handler_class_init (TpMediaStreamHandlerClass *tp_media_stream_handler_class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (tp_media_stream_handler_class);

  g_type_class_add_private (tp_media_stream_handler_class, sizeof (TpMediaStreamHandlerPrivate));

  object_class->dispose = tp_media_stream_handler_dispose;
  object_class->finalize = tp_media_stream_handler_finalize;

  signals[ADD_REMOTE_CANDIDATE] =
    g_signal_new ("add-remote-candidate",
                  G_OBJECT_CLASS_TYPE (tp_media_stream_handler_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  tp_media_stream_handler_marshal_VOID__STRING_BOXED,
                  G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_BOXED);

  signals[REMOVE_REMOTE_CANDIDATE] =
    g_signal_new ("remove-remote-candidate",
                  G_OBJECT_CLASS_TYPE (tp_media_stream_handler_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  tp_media_stream_handler_marshal_VOID__STRING,
                  G_TYPE_NONE, 1, G_TYPE_STRING);

  signals[SET_ACTIVE_CANDIDATE_PAIR] =
    g_signal_new ("set-active-candidate-pair",
                  G_OBJECT_CLASS_TYPE (tp_media_stream_handler_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  tp_media_stream_handler_marshal_VOID__STRING_STRING,
                  G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

  signals[SET_REMOTE_CANDIDATE_LIST] =
    g_signal_new ("set-remote-candidate-list",
                  G_OBJECT_CLASS_TYPE (tp_media_stream_handler_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  tp_media_stream_handler_marshal_VOID__BOXED,
                  G_TYPE_NONE, 1, G_TYPE_BOXED);

  signals[SET_REMOTE_CODECS] =
    g_signal_new ("set-remote-codecs",
                  G_OBJECT_CLASS_TYPE (tp_media_stream_handler_class),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  0,
                  NULL, NULL,
                  tp_media_stream_handler_marshal_VOID__BOXED,
                  G_TYPE_NONE, 1, G_TYPE_BOXED);

  dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (tp_media_stream_handler_class), &dbus_glib_tp_media_stream_handler_object_info);
}

void
tp_media_stream_handler_dispose (GObject *object)
{
  TpMediaStreamHandler *self = TP_MEDIA_STREAM_HANDLER (object);
  TpMediaStreamHandlerPrivate *priv = TP_MEDIA_STREAM_HANDLER_GET_PRIVATE (self);

  if (priv->dispose_has_run)
    return;

  priv->dispose_has_run = TRUE;

  /* release any references held by the object here */

  if (G_OBJECT_CLASS (tp_media_stream_handler_parent_class)->dispose)
    G_OBJECT_CLASS (tp_media_stream_handler_parent_class)->dispose (object);
}

void
tp_media_stream_handler_finalize (GObject *object)
{
  TpMediaStreamHandler *self = TP_MEDIA_STREAM_HANDLER (object);
  TpMediaStreamHandlerPrivate *priv = TP_MEDIA_STREAM_HANDLER_GET_PRIVATE (self);

  /* free any data held directly by the object here */
  /* this line allows the plain autogenerated code to compile with -Werror
     remove it!*/
  g_assert(priv);

  G_OBJECT_CLASS (tp_media_stream_handler_parent_class)->finalize (object);
}



/**
 * tp_media_stream_handler_codec_choice
 *
 * Implements DBus method CodecChoice
 * on interface org.freedesktop.Telepathy.Media.StreamHandler
 *
 * @error: Used to return a pointer to a GError detailing any error
 *         that occured, DBus will throw the error only if this
 *         function returns false.
 *
 * Returns: TRUE if successful, FALSE if an error was thrown.
 */
gboolean tp_media_stream_handler_codec_choice (TpMediaStreamHandler *obj, gint codec_id, GError **error)
{
  return TRUE;
}


/**
 * tp_media_stream_handler_error
 *
 * Implements DBus method Error
 * on interface org.freedesktop.Telepathy.Media.StreamHandler
 *
 * @error: Used to return a pointer to a GError detailing any error
 *         that occured, DBus will throw the error only if this
 *         function returns false.
 *
 * Returns: TRUE if successful, FALSE if an error was thrown.
 */
gboolean tp_media_stream_handler_error (TpMediaStreamHandler *obj, gint errno, const gchar * message, GError **error)
{
  return TRUE;
}


/**
 * tp_media_stream_handler_introspect
 *
 * Implements DBus method Introspect
 * on interface org.freedesktop.DBus.Introspectable
 *
 * @error: Used to return a pointer to a GError detailing any error
 *         that occured, DBus will throw the error only if this
 *         function returns false.
 *
 * Returns: TRUE if successful, FALSE if an error was thrown.
 */
gboolean tp_media_stream_handler_introspect (TpMediaStreamHandler *obj, gchar ** ret, GError **error)
{
  return TRUE;
}


/**
 * tp_media_stream_handler_native_candidates_prepared
 *
 * Implements DBus method NativeCandidatesPrepared
 * on interface org.freedesktop.Telepathy.Media.StreamHandler
 *
 * @error: Used to return a pointer to a GError detailing any error
 *         that occured, DBus will throw the error only if this
 *         function returns false.
 *
 * Returns: TRUE if successful, FALSE if an error was thrown.
 */
gboolean tp_media_stream_handler_native_candidates_prepared (TpMediaStreamHandler *obj, GError **error)
{
  return TRUE;
}


/**
 * tp_media_stream_handler_new_active_candidate_pair
 *
 * Implements DBus method NewActiveCandidatePair
 * on interface org.freedesktop.Telepathy.Media.StreamHandler
 *
 * @error: Used to return a pointer to a GError detailing any error
 *         that occured, DBus will throw the error only if this
 *         function returns false.
 *
 * Returns: TRUE if successful, FALSE if an error was thrown.
 */
gboolean tp_media_stream_handler_new_active_candidate_pair (TpMediaStreamHandler *obj, const gchar * native_candidate_id, const gchar * remote_candidate_id, GError **error)
{
  return TRUE;
}


/**
 * tp_media_stream_handler_new_native_candidate
 *
 * Implements DBus method NewNativeCandidate
 * on interface org.freedesktop.Telepathy.Media.StreamHandler
 *
 * @error: Used to return a pointer to a GError detailing any error
 *         that occured, DBus will throw the error only if this
 *         function returns false.
 *
 * Returns: TRUE if successful, FALSE if an error was thrown.
 */
gboolean tp_media_stream_handler_new_native_candidate (TpMediaStreamHandler *obj, const gchar * candidate_id, const GArray * transports, GError **error)
{
  return TRUE;
}


/**
 * tp_media_stream_handler_ready
 *
 * Implements DBus method Ready
 * on interface org.freedesktop.Telepathy.Media.StreamHandler
 *
 * @error: Used to return a pointer to a GError detailing any error
 *         that occured, DBus will throw the error only if this
 *         function returns false.
 *
 * Returns: TRUE if successful, FALSE if an error was thrown.
 */
gboolean tp_media_stream_handler_ready (TpMediaStreamHandler *obj, GError **error)
{
  return TRUE;
}


/**
 * tp_media_stream_handler_supported_codecs
 *
 * Implements DBus method SupportedCodecs
 * on interface org.freedesktop.Telepathy.Media.StreamHandler
 *
 * @error: Used to return a pointer to a GError detailing any error
 *         that occured, DBus will throw the error only if this
 *         function returns false.
 *
 * Returns: TRUE if successful, FALSE if an error was thrown.
 */
gboolean tp_media_stream_handler_supported_codecs (TpMediaStreamHandler *obj, const GArray * codecs, GError **error)
{
  return TRUE;
}
