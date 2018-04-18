#include <cairo.h>
#include <stdlib.h>
#include <gtk/gtk.h>
//#include <math.h>

static void do_drawing(cairo_t *, GtkWidget *);

struct {
  cairo_surface_t *image;
} glob;

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, 
    gpointer user_data)
{      
  do_drawing(cr, widget);

  return FALSE;
}

static void do_drawing(cairo_t *cr, GtkWidget *widget)
{
  static gint pos_x = 128;
  static gint pos_y = 128;
  static gint radius = 150;  
  static gint delta[] = { 3, 3 };

  GtkWidget *win = gtk_widget_get_toplevel(widget);
  
  gint width, height;
  gtk_window_get_size(GTK_WINDOW(win), &width, &height);

  if (pos_x < 0) {
      delta[0] = rand() % 4 + 5;
  } else if (pos_x > width - radius) {
      delta[0] = -(rand() % 4 + 5);
  }

  if (pos_y < 0) {
      delta[1] = rand() % 4 + 5;
  } else if (pos_y > height - radius) {
      delta[1] = -(rand() % 4 + 5);
  }

  pos_x += delta[0];
  pos_y += delta[1];

  cairo_set_source_surface(cr, glob.image, 1, 1);
  //cairo_arc(cr, pos_x, pos_y, radius, 0, 2*M_PI);
  cairo_rectangle(cr, pos_x, pos_y, radius, radius);
  cairo_clip(cr);
  cairo_paint(cr);      
}

/*
 static void do_drawing(cairo_t *cr)
{ 
  cairo_set_source_rgb(cr, 0.2, 0.3, 0.8);
  cairo_rectangle(cr, 10, 10, 30, 30);
  cairo_fill(cr);

  cairo_translate(cr, 20, 20);
  cairo_set_source_rgb(cr, 0.8, 0.3, 0.2);
  cairo_rectangle(cr, 0, 0, 30, 30);
  cairo_fill(cr);
  
  cairo_translate(cr, 30, 30);
  cairo_set_source_rgb(cr, 0.8, 0.8, 0.2);
  cairo_rectangle(cr, 0, 0, 30, 30);
  cairo_fill(cr);
  
  cairo_translate(cr, 40, 40);
  cairo_set_source_rgb(cr, 0.3, 0.8, 0.8);
  cairo_rectangle(cr, 0, 0, 30, 30);
  cairo_fill(cr);    
}
 */

static gboolean time_handler(GtkWidget *widget)
{  
  gtk_widget_queue_draw(widget);
  return TRUE;
}

int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *darea;  
  gint width, height;  

  glob.image = cairo_image_surface_create_from_png("image.png");
  width = cairo_image_surface_get_width(glob.image);
  height = cairo_image_surface_get_height(glob.image); 

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  darea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER (window), darea);

  g_signal_connect(G_OBJECT(darea), "draw", 
      G_CALLBACK(on_draw_event), NULL);  
  g_signal_connect(G_OBJECT(window), "destroy",
      G_CALLBACK(gtk_main_quit), NULL);

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), width+2, height+2); 
  gtk_window_set_title(GTK_WINDOW(window), "Clip image");

  gtk_widget_show_all(window);
  g_timeout_add(50, (GSourceFunc) time_handler, (gpointer) window);

  gtk_main();

  cairo_surface_destroy(glob.image);

  return 0;
}
