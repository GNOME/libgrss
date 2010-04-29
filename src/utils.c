/*
 * Copyright (C) 2009, Roberto Guido <rguido@src.gnome.org>
 *                     Michele Tameni <michele@amdplanet.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

/*
 * Original code is from Liferea:
 *
 * xml.c  XML helper methods for Liferea
 * Copyright (C) 2003-2009  Lars Lindner <lars.lindner@gmail.com>
 * Copyright (C) 2004-2006  Nathan J. Conrad <t98502@users.sourceforge.net>
 */

#include "utils.h"

typedef struct {
	gchar	*data;
	gint	length;
} ResultBuffer;

static void
unhtmlizeHandleCharacters (void *user_data, const xmlChar *string, int length)
{
	ResultBuffer *buffer = (ResultBuffer*) user_data;
	gint old_length;

	old_length = buffer->length;
	buffer->length += length;
	buffer->data = g_renew (gchar, buffer->data, buffer->length + 1);
        strncpy (buffer->data + old_length, (gchar *)string, length);
	buffer->data[buffer->length] = 0;

}

static void
_unhtmlize (gchar *string, ResultBuffer *buffer)
{
	htmlParserCtxtPtr ctxt;
	htmlSAXHandlerPtr sax_p;

	sax_p = g_new0 (htmlSAXHandler, 1);
 	sax_p->characters = unhtmlizeHandleCharacters;
	ctxt = htmlCreatePushParserCtxt (sax_p, buffer, string, strlen (string), "", XML_CHAR_ENCODING_UTF8);
	htmlParseChunk (ctxt, string, 0, 1);
	htmlFreeParserCtxt (ctxt);
 	g_free (sax_p);
}

static void
_unxmlize (gchar *string, ResultBuffer *buffer)
{
	xmlParserCtxtPtr	ctxt;
	xmlSAXHandler		*sax_p;

	sax_p = g_new0 (xmlSAXHandler, 1);
 	sax_p->characters = unhtmlizeHandleCharacters;
	ctxt = xmlCreatePushParserCtxt (sax_p, buffer, string, strlen (string), "");
	xmlParseChunk (ctxt, string, 0, 1);
	xmlFreeParserCtxt (ctxt);
 	g_free(sax_p);
}

static gchar*
unmarkupize (gchar *string, void(*parse)(gchar *string, ResultBuffer *buffer))
{
	gchar *result;
	ResultBuffer *buffer;

	if (!string)
		return NULL;

	if (NULL == (strpbrk (string, "&<>")))
		return string;

	buffer = g_new0 (ResultBuffer, 1);
	parse (string, buffer);
	result = buffer->data;
	g_free (buffer);

 	if (result == NULL || !g_utf8_strlen (result, -1)) {
		g_free (result);
 		return string;
 	}
	else {
 		g_free (string);
 		return result;
 	}
}

gchar*
unhtmlize (gchar *string)
{
	return unmarkupize (string, _unhtmlize);
}

gchar*
unxmlize (gchar * string)
{
	return unmarkupize (string, _unxmlize);
}

static xmlNodePtr
xhtml_find_body (xmlDocPtr doc)
{
	xmlXPathContextPtr xpathCtxt = NULL;
	xmlXPathObjectPtr xpathObj = NULL;
	xmlNodePtr node = NULL;

	xpathCtxt = xmlXPathNewContext (doc);
	if (!xpathCtxt)
		goto error;

	xpathObj = xmlXPathEvalExpression (BAD_CAST"/html/body", xpathCtxt);
	if (!xpathObj)
		goto error;
	if (!xpathObj->nodesetval->nodeMax)
		goto error;

	node = xpathObj->nodesetval->nodeTab[0];

error:
	if (xpathObj)
		xmlXPathFreeObject (xpathObj);
	if (xpathCtxt)
		xmlXPathFreeContext (xpathCtxt);

	return node;
}

static xmlDocPtr
xhtml_parse (const gchar *html, gint len)
{
	xmlDocPtr out = NULL;

	g_assert (html != NULL);
	g_assert (len >= 0);

	/* Note: NONET is not implemented so it will return an error
	   because it doesn't know how to handle NONET. But, it might
	   learn in the future. */
	out = htmlReadMemory (html, len, NULL, "utf-8", HTML_PARSE_RECOVER | HTML_PARSE_NONET | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
	return out;
}

gchar*
xhtml_extract (xmlNodePtr xml, gint xhtmlMode, const gchar *defaultBase)
{
	xmlBufferPtr buf;
	xmlChar *xml_base = NULL;
	gchar *result = NULL;
	xmlNs *ns;

	/* Create the new document and add the div tag*/
	xmlDocPtr newDoc = xmlNewDoc (BAD_CAST "1.0" );
	xmlNodePtr divNode = xmlNewNode (NULL, BAD_CAST "div");
	xmlDocSetRootElement (newDoc, divNode);
	xmlNewNs (divNode, BAD_CAST "http://www.w3.org/1999/xhtml", NULL);

	/* Set the xml:base  of the div tag */
	xml_base = xmlNodeGetBase (xml->doc, xml);
	if (xml_base) {
		xmlNodeSetBase (divNode, xml_base );
		xmlFree (xml_base);
	}
	else if (defaultBase)
		xmlNodeSetBase (divNode, BAD_CAST defaultBase);

	if (xhtmlMode == 0) { /* Read escaped HTML and convert to XHTML, placing in a div tag */
		xmlDocPtr oldDoc;
		xmlNodePtr copiedNodes = NULL;
		xmlChar *escapedhtml;

		/* Parse the HTML into oldDoc*/
		escapedhtml = xmlNodeListGetString (xml->doc, xml->xmlChildrenNode, 1);
		if (escapedhtml) {
			escapedhtml = BAD_CAST g_strstrip ((gchar*) escapedhtml);	/* stripping whitespaces to make empty string detection easier */
			if (*escapedhtml) {						/* never process empty content, xmlDocCopy() doesn't like it... */
				xmlNodePtr body;
				oldDoc = xhtml_parse ((gchar*) escapedhtml, strlen ((gchar*) escapedhtml));
				body = xhtml_find_body (oldDoc);

				/* Copy namespace from original documents root node. This is
				   ro determine additional namespaces for item content. For
				   example to handle RSS 2.0 feeds as provided by LiveJournal:

				   <rss version='2.0' xmlns:lj='http://www.livejournal.org/rss/lj/1.0/'>
				   <channel>
				      ...
				      <item>
	        			 ...
  	        			 <description>... &lt;span class=&apos;ljuser&apos; lj:user=&apos;someone&apos; style=&apos;white-space: nowrap;&apos;&gt;&lt;a href=&apos;http://community.livejournal.com/someone/profile&apos;&gt;&lt;img src=&apos;http://stat.livejournal.com/img/community.gif&apos; alt=&apos;[info]&apos; width=&apos;16&apos; height=&apos;16&apos; style=&apos;vertical-align: bottom; border: 0; padding-right: 2px;&apos; /&gt;&lt;/a&gt;&lt;a href=&apos;http://community.livejournal.com/someone/&apos;&gt;&lt;b&gt;someone&lt;/b&gt;&lt;/a&gt;&lt;/span&gt; ...</description>
					 ...
				      </item>
				      ...
				   </channel>

				   Then we will want to extract <description> and need to
				   honour the xmlns:lj definition...
				*/
				ns = (xmlDocGetRootElement (xml->doc))->nsDef;
				while (ns) {
					xmlNewNs (divNode, ns->href, ns->prefix);
					ns = ns->next;
				}

				if (body) {
					/* Copy in the html tags */
					copiedNodes = xmlDocCopyNodeList (newDoc, body->xmlChildrenNode);
					// FIXME: is the above correct? Why only operate on the first child node?
					// It might be unproblematic because all content is wrapped in a <div>...
					xmlAddChildList (divNode, copiedNodes);
				}
				xmlFreeDoc (oldDoc);
				xmlFree (escapedhtml);
			}
		}
	}
	else if (xhtmlMode == 1 || xhtmlMode == 2) { /* Read multiple XHTML tags and embed in div tag */
		xmlNodePtr copiedNodes = xmlDocCopyNodeList (newDoc, xml->xmlChildrenNode);
		xmlAddChildList (divNode, copiedNodes);
	}

	buf = xmlBufferCreate ();
	xmlNodeDump (buf, newDoc, xmlDocGetRootElement (newDoc), 0, 0 );

	if (xmlBufferLength (buf) > 0)
		result = (gchar*) xmlCharStrdup ((gchar*) xmlBufferContent (buf));

	xmlBufferFree (buf);
	xmlFreeDoc (newDoc);
	return result;
}

static xmlEntityPtr
xml_process_entities (void *ctxt, const xmlChar *name)
{
	gchar *path;
	xmlEntityPtr entity;
	xmlEntityPtr found;
	xmlChar *tmp;
	static xmlDocPtr entities = NULL;

	entity = xmlGetPredefinedEntity (name);

	if (!entity) {
		if (!entities) {
			/* loading HTML entities from external DTD file */
			entities = xmlNewDoc (BAD_CAST "1.0");
			path = g_build_filename (g_get_user_data_dir (), PACKAGE, "/dtd/html.ent", NULL);
			xmlCreateIntSubset (entities, BAD_CAST "HTML entities", NULL, BAD_CAST path);
			g_free (path);
			entities->extSubset = xmlParseDTD (entities->intSubset->ExternalID, entities->intSubset->SystemID);
		}

		if (NULL != (found = xmlGetDocEntity (entities, name))) {
			/* returning as faked predefined entity... */
			tmp = xmlStrdup (found->content);
			tmp = BAD_CAST unhtmlize ((gchar*) tmp);	/* arghh ... slow... */
			entity = (xmlEntityPtr) g_new0 (xmlEntity, 1);
			entity->type = XML_ENTITY_DECL;
			entity->name = name;
			entity->orig = NULL;
			entity->content = tmp;
			entity->length = g_utf8_strlen ((gchar*) tmp, -1);
			entity->etype = XML_INTERNAL_PREDEFINED_ENTITY;
		}
	}

	return entity;
}

xmlDocPtr
content_to_xml (const gchar *contents, gsize size)
{
	xmlParserCtxtPtr ctxt;
	xmlDocPtr doc;

	ctxt = xmlNewParserCtxt ();
	ctxt->sax->getEntity = xml_process_entities;
	doc = xmlSAXParseMemory (ctxt->sax, contents, size, 0);
	xmlFreeParserCtxt (ctxt);

	return doc;
}

xmlDocPtr
file_to_xml (const gchar *path)
{
	xmlParserCtxtPtr ctxt;
	xmlDocPtr doc;

	ctxt = xmlNewParserCtxt ();
	ctxt->sax->getEntity = xml_process_entities;
	doc = xmlSAXParseFile (ctxt->sax, path, 0);
	xmlFreeParserCtxt (ctxt);

	return doc;
}

/* in theory, we'd need only the RFC822 timezones here
   in practice, feeds also use other timezones...        */
static struct {
	const char *name;
	int offset;
} tz_offsets [] = {
	{ "IDLW", -1200 },
	{ "HAST", -1000 },
	{ "AKST", -900 },
	{ "AKDT", -800 },
	{ "WESZ", 100 },
	{ "WEST", 100 },
	{ "WEDT", 100 },
	{ "MEST", 200 },
	{ "MESZ", 200 },
	{ "CEST", 200 },
	{ "CEDT", 200 },
	{ "EEST", 300 },
	{ "EEDT", 300 },
	{ "IRST", 430 },
	{ "CNST", 800 },
	{ "ACST", 930 },
	{ "ACDT", 1030 },
	{ "AEST", 1000 },
	{ "AEDT", 1100 },
	{ "IDLE", 1200 },
	{ "NZST", 1200 },
	{ "NZDT", 1300 },
	{ "GMT", 0 },
	{ "EST", -500 },
	{ "EDT", -400 },
	{ "CST", -600 },
	{ "CDT", -500 },
	{ "MST", -700 },
	{ "MDT", -600 },
	{ "PST", -800 },
	{ "PDT", -700 },
	{ "HDT", -900 },
	{ "YST", -900 },
	{ "YDT", -800 },
	{ "AST", -400 },
	{ "ADT", -300 },
	{ "VST", -430 },
	{ "NST", -330 },
	{ "NDT", -230 },
	{ "WET", 0 },
	{ "WEZ", 0 },
	{ "IST", 100 },
	{ "CET", 100 },
	{ "MEZ", 100 },
	{ "EET", 200 },
	{ "MSK", 300 },
	{ "MSD", 400 },
	{ "IRT", 330 },
	{ "IST", 530 },
	{ "ICT", 700 },
	{ "JST", 900 },
	{ "NFT", 1130 },
	{ "UT", 0 },
	{ "PT", -800 },
	{ "BT", 300 },
	{ "Z", 0 },
	{ "A", -100 },
	{ "M", -1200 },
	{ "N", 100 },
	{ "Y", 1200 }
};

static time_t
date_parse_rfc822_tz (char *token)
{
	int offset = 0;
	const char *inptr = token;
	int num_timezones;

	num_timezones = sizeof (tz_offsets) / sizeof ((tz_offsets) [0]);

	if (*inptr == '+' || *inptr == '-') {
		offset = atoi (inptr);
	}
	else {
		int t;

		if (*inptr == '(')
			inptr++;

		for (t = 0; t < num_timezones; t++)
			if (!strncmp (inptr, tz_offsets [t].name, strlen (tz_offsets [t].name))) {
				offset = tz_offsets [t].offset;
				break;
			}
	}

	return 60 * ((offset / 100) * 60 + (offset % 100));
}

time_t
date_parse_RFC822 (const gchar *date)
{
	struct tm tm;
	time_t t;
	time_t t2;
	char *oldlocale;
	char *pos;
	gboolean success = FALSE;

	memset (&tm, 0, sizeof (struct tm));

	/* we expect at least something like "03 Dec 12 01:38:34"
	   and don't require a day of week or the timezone

	   the most specific format we expect:  "Fri, 03 Dec 12 01:38:34 CET"
	 */

	/* skip day of week */
	pos = g_utf8_strchr (date, -1, ',');
	if (pos)
		date = ++pos;

	/* we expect English month names, so we set the locale */
	oldlocale = g_strdup (setlocale (LC_TIME, NULL));
	setlocale (LC_TIME, "C");

	/* standard format with seconds and 4 digit year */
	if (NULL != (pos = strptime ((const char*) date, " %d %b %Y %T", &tm)))
		success = TRUE;
	/* non-standard format without seconds and 4 digit year */
	else if (NULL != (pos = strptime ((const char*) date, " %d %b %Y %H:%M", &tm)))
		success = TRUE;
	/* non-standard format with seconds and 2 digit year */
	else if (NULL != (pos = strptime ((const char*) date, " %d %b %y %T", &tm)))
		success = TRUE;
	/* non-standard format without seconds 2 digit year */
	else if (NULL != (pos = strptime ((const char*) date, " %d %b %y %H:%M", &tm)))
		success = TRUE;

	while (pos && *pos != '\0' && isspace ((int) *pos))       /* skip whitespaces before timezone */
		pos++;

	if (oldlocale) {
		setlocale (LC_TIME, oldlocale);	/* and reset it again */
		g_free (oldlocale);
	}

	if (success) {
		if ((time_t)(-1) != (t = mktime (&tm))) {
			/* GMT time, with no daylight savings time
			   correction. (Usually, there is no daylight savings
			   time since the input is GMT.) */
			t = t - date_parse_rfc822_tz (pos);
			t2 = mktime (gmtime (&t));
			t = t - (t2 - t);
			return t;
		}
	}

	return 0;
}

time_t
date_parse_ISO8601 (const gchar *date)
{
	struct tm tm;
	time_t t;
	time_t t2;
	time_t offset = 0;
	gboolean success = FALSE;
	gchar *pos;

	g_assert (date != NULL);

	memset (&tm, 0, sizeof (struct tm));

	/* we expect at least something like "2003-08-07T15:28:19" and
	   don't require the second fractions and the timezone info

	   the most specific format:   YYYY-MM-DDThh:mm:ss.sTZD
	 */

	/* full specified variant */
	pos = strptime (date, "%t%Y-%m-%dT%H:%M%t", &tm);
	if (pos) {
		/* Parse seconds */
		if (*pos == ':')
			pos++;

		if (isdigit (pos [0]) && !isdigit (pos [1])) {
			tm.tm_sec = pos[0] - '0';
			pos++;
		}
		else if (isdigit (pos [0]) && isdigit (pos [1])) {
			tm.tm_sec = 10 * (pos [0] - '0') + pos [1] - '0';
			pos += 2;
		}

		/* Parse second fractions */
		if (*pos == '.') {
			while (*pos == '.' || isdigit (pos [0]))
				pos++;
		}

		/* Parse timezone */
		if (*pos == 'Z') {
			offset = 0;
		}
		else if ((*pos == '+' || *pos == '-') && isdigit (pos [1]) && isdigit (pos [2]) && strlen (pos) >= 3) {
			offset = (10 * (pos [1] - '0') + (pos [2] - '0')) * 60 * 60;

			if (pos [3] == ':' && isdigit (pos [4]) && isdigit (pos [5]))
				offset +=  (10 * (pos [4] - '0') + (pos [5] - '0')) * 60;
			else if (isdigit (pos [3]) && isdigit (pos [4]))
				offset +=  (10 * (pos [3] - '0') + (pos [4] - '0')) * 60;

			offset *= (pos [0] == '+') ? 1 : -1;
		}

		success = TRUE;
	}

	/* only date */
	else if (NULL != strptime (date, "%t%Y-%m-%d", &tm)) {
		success = TRUE;
	}

	/* there were others combinations too... */

	if (success) {
		if ((time_t)(-1) != (t = mktime (&tm))) {
			/* Correct for the local timezone*/
			struct tm tmp_tm;

			t = t - offset;
			gmtime_r (&t, &tmp_tm);
			t2 = mktime (&tmp_tm);
			t = t - (t2 - t);

			return t;
		}
	}

	return 0;
}

gboolean
address_seems_public (GInetAddress *addr)
{
	return (g_inet_address_get_is_loopback (addr) == FALSE &&
	        g_inet_address_get_is_site_local (addr) == FALSE &&
	        g_inet_address_get_is_multicast (addr) == FALSE &&
	        g_inet_address_get_is_mc_link_local (addr) == FALSE &&
	        g_inet_address_get_is_mc_node_local (addr) == FALSE &&
	        g_inet_address_get_is_mc_site_local (addr) == FALSE &&
	        g_inet_address_get_is_mc_org_local (addr) == FALSE &&
	        g_inet_address_get_is_mc_global (addr) == FALSE);
}
