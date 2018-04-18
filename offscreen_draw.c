#include <gtk/gtk.h>

static cairo_surface_t *surface = NULL;

static gboolean
configure_event_cb (GtkWidget         *widget,
                    GdkEventConfigure *event,
                    gpointer           data)
{
  cairo_t *cr;

  g_print("configure event: %d, %d\n", event->width, event->height);

  if (surface)
    cairo_surface_destroy (surface);

  surface = gdk_window_create_similar_surface (event->window,
                                               CAIRO_CONTENT_COLOR,
                                               event->width,
                                               event->height);

  cr = cairo_create(surface);
  cairo_set_source_rgb(cr, 1, 0, 0);
  cairo_paint (cr);
  cairo_destroy(cr);

  return FALSE;
}

static gboolean
draw_cb (GtkWidget *widget,
         cairo_t   *cr,
         gpointer   data)
{
  g_print("draw event\n");

  cairo_set_source_surface(cr, surface, 0, 0);
  cairo_paint(cr);

  return TRUE;
}

int
main(int    argc,
     char **argv)
{
  GtkWidget *window,
            *area;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 500, 400);
  g_signal_connect (window, "destroy", gtk_main_quit, NULL);

  area = gtk_drawing_area_new();
  g_signal_connect (area, "draw",
                    G_CALLBACK (draw_cb), NULL);
  g_signal_connect (area, "configure-event",
                    G_CALLBACK(configure_event_cb), NULL);
  gtk_container_add (GTK_CONTAINER (window), area);

  gtk_widget_show_all(window);
  gtk_main();

  return 0;
} 
