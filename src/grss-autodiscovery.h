/* grss-autodiscovery.h
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

#ifndef GRSS_AUTODISCOVERY_H
#define GRSS_AUTODISCOVERY_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GRSS_TYPE_AUTODISCOVERY (grss_autodiscovery_get_type())

G_DECLARE_FINAL_TYPE (GrssAutodiscovery, grss_autodiscovery, GRSS, AUTODISCOVERY, GObject)

GrssAutodiscovery *grss_autodiscovery_new      (const gchar        *url);
gboolean           grss_autodiscovery_fetch    (GrssAutodiscovery  *self,
                                                GError            **error);
GList             *grss_autodiscovery_discover (GrssAutodiscovery  *self);

G_END_DECLS

#endif /* GRSS_AUTODISCOVERY_H */
