/* person.h
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

#ifndef GRSS_PERSON_H
#define GRSS_PERSON_H

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * GrssPerson:
 *
 * `GrssPerson` is an opaque structure whose members
 * cannot be accessed directly.
 *
 * Since: 0.7
 */
typedef struct _GrssPerson GrssPerson;

GType grss_person_get_type (void) G_GNUC_CONST;

void grss_person_unref (GrssPerson *person);
GrssPerson *grss_person_ref (GrssPerson *person);

GrssPerson *grss_person_new (const gchar *name,
                             const gchar *email,
                             const gchar *uri);

const gchar *grss_person_get_name (GrssPerson *person);
const gchar *grss_person_get_email (GrssPerson *person);
const gchar *grss_person_get_uri (GrssPerson *person);

G_END_DECLS

#endif /* GRSS_PERSON_H */
