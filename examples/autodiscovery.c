#include <glib.h>

#include <libgrss.h>

gint
main (gint   argc,
      gchar *argv[])
{
  g_autoptr(GrssAutodiscovery) discovery = grss_autodiscovery_new ("https://planet.gnome.org/");
  grss_autodiscovery_fetch (discovery, NULL);
  g_autoptr(GList) lst = grss_autodiscovery_discover (discovery);

  for (; lst != NULL; lst = lst->next)
    g_print ("%s\n", (gchar *)lst->data);

  return 0;
}
