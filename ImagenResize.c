#include<gtk/gtk.h>

gboolean draw_picture(GtkWidget *da, cairo_t *cr, gpointer data)
{
  gint width=gtk_widget_get_allocated_width(da);
  gint height=gtk_widget_get_allocated_height(da);

  GdkPixbuf *temp=gdk_pixbuf_scale_simple((GdkPixbuf*)data, width, height, GDK_INTERP_BILINEAR);
  gdk_cairo_set_source_pixbuf(cr, temp, 0, 0);
  cairo_paint(cr);

  g_object_unref(temp);
  return FALSE;
}
int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Resize Picture");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  //Needs a valid picture.
  GdkPixbuf *pixbuf=gdk_pixbuf_new_from_file(argc>1 ? argv[1] : "image.jpg", NULL);

  GtkWidget *da1=gtk_drawing_area_new();
  gtk_widget_set_hexpand(da1, TRUE);
  gtk_widget_set_vexpand(da1, TRUE);
  g_signal_connect(da1, "draw", G_CALLBACK(draw_picture), pixbuf);

  GtkWidget *grid=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), da1, 0, 0, 1, 1);

  gtk_container_add(GTK_CONTAINER(window), grid);
  gtk_widget_show_all(window);

  gtk_main();

  g_object_unref(pixbuf);

  return 0;
}
