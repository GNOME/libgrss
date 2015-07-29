/* grss-autodiscovery.c
 *
 * Copyright (C) 2015 Igor Gnatenko <ignatenko@src.gnome.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "grss-autodiscovery.h"

#include <libxml/HTMLparser.h>
#include <libsoup/soup.h>

#define XML_TO_CHAR(s)  ((char *) (s))
#define CHAR_TO_XML(s)  ((unsigned char *) (s))

/**
 * SECTION: autodiscovery
 * @short_description: autodiscovery
 *
 * #GrssAutodiscovery rappresents RSS autodiscovery feature.
 */

struct _GrssAutodiscovery
{
  GObject parent_instance;

  gchar *url;
  htmlDocPtr doc;
  GInputStream *stream;
};

G_DEFINE_TYPE (GrssAutodiscovery, grss_autodiscovery, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_URL,
  LAST_PROP
};

static GParamSpec *gParamSpecs [LAST_PROP];

static int
grss_autodiscovery_io_read_cb (void *context,
                               char *buffer,
                               int   len)
{
  GInputStream *stream = (GInputStream *)context;
  g_return_val_if_fail (G_IS_INPUT_STREAM (stream), -1);
  return g_input_stream_read (stream, buffer, len, NULL, NULL);
}

static int
grss_autodiscovery_io_close_cb (void *context)
{
  GInputStream *stream = (GInputStream *)context;
  g_return_val_if_fail (G_IS_INPUT_STREAM (stream), -1);
  return g_input_stream_close (stream, NULL, NULL) ? 0 : -1;
}

gboolean
grss_autodiscovery_load_from_stream (GrssAutodiscovery *self)
{
  g_return_val_if_fail (GRSS_IS_AUTODISCOVERY (self), FALSE);

  self->doc = htmlReadIO (grss_autodiscovery_io_read_cb,
                          grss_autodiscovery_io_close_cb,
                          self->stream,
                          self->url,
                          "utf8", /* FIXME */
                          0);

  /* TODO: also have a error setting if fails */
  if (!self->doc)
    return FALSE;

  return TRUE;
}

/**
 * grss_autodiscovery_new:
 * @url: The URL.
 *
 * Allocates a new #GrssAutodiscovery.
 *
 * Returns: (transfer full): a #GrssAutodiscovery.
 */
GrssAutodiscovery *
grss_autodiscovery_new (const gchar *url)
{
  return g_object_new (GRSS_TYPE_AUTODISCOVERY,
                       "url", url,
                       NULL);
}

static void
grss_autodiscovery_finalize (GObject *object)
{
  GrssAutodiscovery *self = (GrssAutodiscovery *)object;

  g_clear_pointer (&self->url, g_free);
  g_object_unref (self->stream);
  xmlFreeDoc (self->doc);

  G_OBJECT_CLASS (grss_autodiscovery_parent_class)->finalize (object);
}

static void
grss_autodiscovery_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  GrssAutodiscovery *self = GRSS_AUTODISCOVERY (object);

  switch (prop_id)
    {
    case PROP_URL:
      g_value_set_string (value, self->url);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grss_autodiscovery_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  GrssAutodiscovery *self = GRSS_AUTODISCOVERY (object);

  switch (prop_id)
    {
    case PROP_URL:
      self->url = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grss_autodiscovery_class_init (GrssAutodiscoveryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = grss_autodiscovery_finalize;
  object_class->get_property = grss_autodiscovery_get_property;
  object_class->set_property = grss_autodiscovery_set_property;

  gParamSpecs [PROP_URL] =
    g_param_spec_string ("url",
                         "Url",
                         "Url of site for RSS discovery",
                         NULL,
                         (G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, gParamSpecs);
}

static void
grss_autodiscovery_init (GrssAutodiscovery *self)
{
}

/**
 * grss_autodiscovery_fetch:
 * @self: a #GrssAutodiscovery.
 * @error: a #GError.
 *
 * Returns: %TRUE if fetch and parsing was ok, %FALSE otherwise.
 */
gboolean
grss_autodiscovery_fetch (GrssAutodiscovery  *self,
                          GError            **error)
{
  g_return_val_if_fail (GRSS_IS_AUTODISCOVERY (self), FALSE);

  SoupSession *session = soup_session_new ();
  SoupMessage *message = soup_message_new ("GET", self->url);
  self->stream = soup_session_send (session, message, NULL, error);
  if (error)
    return FALSE;

  grss_autodiscovery_load_from_stream (self);

  return TRUE;
}

static gboolean
grss_autodiscovery_validate_link_node (xmlNode  *link_node,
                                       gchar   **type,
                                       gchar   **href)
{
  xmlAttr *props = link_node->properties;

  gchar *allowed_mime_types[] = {
    "application/atom+xml",
    "application/rss+xml",
    NULL
  };

  gboolean href_ok = FALSE;
  gboolean rel_ok = FALSE;
  gboolean type_ok = FALSE;

  for (; props; props = props->next) {
    /*
     * The href attribute MUST be the feed's URL. This can be a relative URL in
     * pages that include a base element in the header.
     *
     * <head>
     *   <title>RSS Advisory Board</title>
     *   <base href="http://www.rssboard.org/">
     *   <link rel="alternate" type="application/rss+xml" href="rss-feed">
     * </head>
     *
     * Because some software might not check for a base URL in relation to
     * autodiscovery links, publishers SHOULD identify feeds with full URLs.
     * When an autodiscovery link is relative and no base URL has been
     * provided, clients should treat the web page's URL as the base.
     */
    if (g_strcmp0 (XML_TO_CHAR (props->name), "href") == 0)
      /* TODO: add some more checks, for example base element */
      if (props->children->type == XML_TEXT_NODE) {
        *href = XML_TO_CHAR (props->children->content);
        href_ok = TRUE;
      }

    /*
     * The rel attribute MUST have a value of "alternate", a keyword that
     * indicates the link is an alternate version of the site's main content.
     *
     * Although for purposes other than autodiscovery this attribute may
     * contain multiple keywords separated by spaces, in an autodiscovery link,
     * the value MUST NOT contain keywords other than "alternate".
     *
     * Additionally, though rel keywords are case-insensitive elsewhere,
     * "alternate" MUST be lowercase.
     */
    if (g_strcmp0 (XML_TO_CHAR (props->name), "rel") == 0)
      if (props->children->type == XML_TEXT_NODE &&
          g_strcmp0 (XML_TO_CHAR (props->children->content), "alternate") == 0)
        rel_ok = TRUE;

    /*
     * The type attribute MUST contain the feed's MIME type, which is
     * "application/rss+xml" for RSS 1.0 or RSS 2.0 feeds.
     *
     * Although type values are case-insensitive for other HTML and XHTML
     * links, the value must be lowercase for autodiscovery.
     */
    if (g_strcmp0 (XML_TO_CHAR (props->name), "type") == 0)
      if (props->children->type == XML_TEXT_NODE)
        for (guint i = 0; allowed_mime_types[i] != NULL; i++)
          if (g_strcmp0 (XML_TO_CHAR (props->children->content),
                         allowed_mime_types [i]) == 0) {
            type_ok = TRUE;
            break;
          }
  }

  return (href_ok && rel_ok && type_ok);
}

xmlNode *
grss_autodiscovery_get_head_node (xmlNode *html_node)
{
  xmlNode *cur_node = NULL;

  for (cur_node = html_node->children; cur_node; cur_node = cur_node->next)
    if (cur_node->type == XML_ELEMENT_NODE &&
        g_strcmp0 (XML_TO_CHAR (cur_node->name), "head") == 0)
      return cur_node;

  return NULL;
}

/**
 * grss_autodiscovery_discover:
 * @self: a #GrssAutodiscovery.
 *
 * We will trturn all possible auto-discovered links, but you'd probably want
 * to use first of list.
 *
 * Returns: a #GList.
 */
GList *
grss_autodiscovery_discover (GrssAutodiscovery *self)
{
  g_return_val_if_fail (GRSS_IS_AUTODISCOVERY (self), NULL);
  g_return_val_if_fail (self->doc, NULL);

  GList *ret = NULL;
  xmlNode *tmp = NULL;
  xmlNode *root_element = xmlDocGetRootElement (self->doc);
  gchar *href = NULL;

  tmp = grss_autodiscovery_get_head_node (root_element);
  if (!tmp)
    return NULL;

  for (tmp = tmp->children; tmp; tmp = tmp->next)
    if (tmp->type == XML_ELEMENT_NODE)
      if (g_strcmp0 (XML_TO_CHAR (tmp->name), "link") == 0)
        if (grss_autodiscovery_validate_link_node (tmp, NULL, &href)) {
          ret = g_list_append (ret, href);
          g_debug ("RSS discovered link: %s\n", href);
        }

  return ret;
}
