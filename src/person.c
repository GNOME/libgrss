/* person.c
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

#include "person.h"

/**
 * SECTION: person
 * @title: GrssPerson
 * @short_description: a structure representing Person
 *
 * #GrssPerson is an immutable object; once it has been created it
 * cannot be modified further. All modifiers will create a new
 * #GrssPerson. Nearly all such functions can fail %NULL will be
 * returned.
 *
 * #GrssPerson is reference counted: the reference count is increased
 * by calling grss_person_ref() and decreased by calling
 * grss_person_unref(). When the reference count drops to 0, the
 * resources allocated by the #GrssPerson structure are released.
 *
 * #GrssPerson is available since Grss 0.7.
 */

struct _GrssPerson
{
  gchar *name;
  gchar *uri;
  gchar *email;

  volatile gint ref_count;
};

static GrssPerson *
grss_person_alloc (void)
{
  GrssPerson *person;

  person = g_slice_new0 (GrssPerson);
  person->ref_count = 1;

  return person;
}

/**
 * grss_person_ref:
 * @person: a #GrssPerson
 *
 * Atomically increments the reference count of @person by one.
 *
 * Returns: the #GrssPerson with the reference count increased
 *
 * Since: 0.7
 */
GrssPerson *
grss_person_ref (GrssPerson *person)
{
  g_return_val_if_fail (person != NULL, NULL);
  g_return_val_if_fail (person->ref_count > 0, NULL);

  g_atomic_int_inc (&person->ref_count);

  return person;
}

/**
 * grss_person_unref:
 * @person: a #GrssPerson
 *
 * Atomically decrements the reference count of @person by one.
 *
 * When the reference count reaches zero, the resources allocated by
 * @person are freed
 *
 * Since: 0.7
 */
void
grss_person_unref (GrssPerson *person)
{
  g_return_if_fail (person != NULL);
  g_return_if_fail (person->ref_count > 0);

  if (g_atomic_int_dec_and_test (&person->ref_count)) {
    g_free (person->name);
    g_free (person->email);
    g_free (person->uri);
    g_slice_free (GrssPerson, person);
  }
}

/**
 * grss_person_new:
 * @name: the name of the person
 * @email: (allow-none): the email of the person, or %NULL
 * @uri: (allow-none): the homepage (uri) of the person, or %NULL
 *
 * Creates a new #GrssPerson.
 *
 * Returns: a new #GrssPerson, or %NULL
 *
 * Since: 0.7
 */
GrssPerson *
grss_person_new (const gchar *name,
                 const gchar *email,
                 const gchar *uri)
{
  GrssPerson *person;

  person = grss_person_alloc ();
  person->name = g_strdup (name);
  person->email = g_strdup (email);
  person->uri = g_strdup (uri);

  return person;
}

/**
 * grss_person_get_name:
 * @person: a #GrssPerson
 *
 * Returns: (transfer none): the name of person. The returned
 *          string is owned by #GrssPerson and it should
 *          not be modified or freed.
 *
 * Since: 0.7
 */
const gchar *
grss_person_get_name (GrssPerson *person)
{
  g_return_val_if_fail (person != NULL, NULL);

  return person->name;
}

/**
 * grss_person_get_email:
 * @person: a #GrssPerson
 *
 * Returns: (transfer none): the email of person. The returned
 *          string is owned by #GrssPerson and it should
 *          not be modified or freed.
 *
 * Since: 0.7
 */
const gchar *
grss_person_get_email (GrssPerson *person)
{
  g_return_val_if_fail (person != NULL, NULL);

  return person->email;
}

/**
 * grss_person_get_uri:
 * @person: a #GrssPerson
 *
 * Returns: (transfer none): the website (uri) of person. The returned
 *          string is owned by #GrssPerson and it should
 *          not be modified or freed.
 *
 * Since: 0.7
 */
const gchar *
grss_person_get_uri (GrssPerson *person)
{
  g_return_val_if_fail (person != NULL, NULL);

  return person->uri;
}
