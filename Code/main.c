#include <stdio.h>
#include <stdlib.h>
#include <gtk-4.0/gtk/gtk.h>
#include <X11/Xlib.h>
#include <pango/pango.h>
#include <string.h>

struct screen_resolution
{
  int width;
  int height;
};

struct screen_resolution bot_get_x11_default_screen_resolution ()
{
  Display * display;
  Window root_window;
  XWindowAttributes window_attributes;

  // Open a connection to the X server
  display = XOpenDisplay (NULL);

  // Get the root window
  root_window = XRootWindow (display, 0);

  // Get the attributes of the root window
  XGetWindowAttributes (display, root_window, &window_attributes);

  struct screen_resolution default_screen;
  default_screen.height = window_attributes.height;
  default_screen.width = window_attributes.width;

  return default_screen;
}

void login_button_clicked(GtkWidget *button, gpointer data) {
  GtkWidget * grid;
  GtkWidget * entry_username;
  GtkWidget * entry_password;
  
  const gchar * username;
  const gchar * password;

  grid = GTK_WIDGET (data);

  entry_username= gtk_grid_get_child_at (GTK_GRID (grid) , 1 , 0);
  entry_password = gtk_grid_get_child_at (GTK_GRID (grid) , 1 , 1);
  
  username = gtk_editable_get_text (GTK_EDITABLE (entry_username));
  password = gtk_editable_get_text (GTK_EDITABLE (entry_password));

  g_print ("Username: %s , Password: %s\n" , username , password);
}
static void login_menu (GtkApplication *app, gpointer user_data)
{
  GtkWidget * window;

  GtkWidget * box_main;
  GtkWidget * box_login;
  GtkWidget * box_creator;
  
  GtkWidget * grid_credentials;
  GtkWidget * grid_buttons;
  
  GtkWidget * label_creator;
  GtkWidget * label_heading_login;
  GtkWidget * label_username;
  GtkWidget * label_password;

  GtkWidget * entry_username;
  GtkWidget * entry_password;

  GtkWidget * button_login;
  GtkWidget * button_signup;
  
  PangoAttrList * attrs;
  PangoAttribute * weight;

  GtkCssProvider * login_css_provider;
  GtkCssProvider * credentials_css_provider;

  GtkStyleContext * login_style_context;
  GtkStyleContext * credentials_style_context;

  struct screen_resolution resolution = bot_get_x11_default_screen_resolution ();
 
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Window");
  gtk_window_set_default_size(GTK_WINDOW(window), resolution.width, resolution.height);

  box_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 50);
  gtk_widget_set_halign(box_main, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box_main, GTK_ALIGN_CENTER);

  box_login = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign(box_login, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box_login, GTK_ALIGN_CENTER);
  
  box_creator = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_widget_set_halign(box_creator, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(box_creator, GTK_ALIGN_CENTER);

  grid_credentials = gtk_grid_new ();
  
  label_creator = gtk_label_new ("This project is created by: Muhammad Talha Noshahi");
  label_heading_login = gtk_label_new ("Login");
  label_username = gtk_label_new ("Username: ");
  label_password = gtk_label_new ("Password: ");

  attrs = pango_attr_list_new();
  weight = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
  pango_attr_list_insert(attrs, weight);

  gtk_label_set_attributes(GTK_LABEL(label_creator), attrs);
  gtk_label_set_attributes(GTK_LABEL(label_heading_login), attrs);
  gtk_label_set_attributes(GTK_LABEL(label_username), attrs);
  gtk_label_set_attributes(GTK_LABEL(label_password), attrs);
  
  gtk_box_append (GTK_BOX (box_creator), label_creator);
  gtk_box_append (GTK_BOX (box_login), label_heading_login);
  
  gtk_grid_attach (GTK_GRID (grid_credentials) , label_username , 0 , 0 , 1 , 1);
  gtk_grid_attach (GTK_GRID (grid_credentials) , label_password , 0 , 1 , 1 , 1);

  entry_username = gtk_entry_new ();
  entry_password = gtk_entry_new ();

  gtk_grid_attach (GTK_GRID (grid_credentials) , entry_username , 1 , 0 , 1 , 1);
  gtk_grid_attach (GTK_GRID (grid_credentials) , entry_password , 1 , 1 , 1 , 1);
  
  login_css_provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (login_css_provider, "label { font-size: 24px; }", -1);
  login_style_context = gtk_widget_get_style_context (label_heading_login);
  gtk_style_context_add_provider (login_style_context, GTK_STYLE_PROVIDER (login_css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  credentials_css_provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (credentials_css_provider, "label { font-size: 14px; }", -1);
  credentials_style_context = gtk_widget_get_style_context(label_username);
  gtk_style_context_add_provider(credentials_style_context, GTK_STYLE_PROVIDER(credentials_css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  credentials_style_context = gtk_widget_get_style_context(label_password);
  gtk_style_context_add_provider(credentials_style_context, GTK_STYLE_PROVIDER(credentials_css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  button_login = gtk_button_new_with_label ("Login");
  button_signup = gtk_button_new_with_label ("Sign Up");

  grid_buttons = gtk_grid_new ();
  
  gtk_grid_attach (GTK_GRID (grid_buttons) , button_login , 0 , 0 , 1 , 1);
  gtk_grid_attach (GTK_GRID (grid_buttons) , button_signup , 1 , 0 , 1 , 1);
    
  gtk_box_append (GTK_BOX (box_main) , box_login);
  gtk_box_append (GTK_BOX (box_main), grid_credentials);
  gtk_box_append (GTK_BOX (box_main) , grid_buttons);
  gtk_box_append (GTK_BOX (box_main), box_creator);

  gtk_window_set_child(GTK_WINDOW(window), box_main);

  g_signal_connect(button_login, "clicked", G_CALLBACK(login_button_clicked), (gpointer)grid_credentials);

  gtk_widget_show(window);

  pango_attr_list_unref(attrs);
}

int main(int argc, char *argv[])
{
    GtkApplication * app;
    int status;

    app = gtk_application_new ("bot.dbs.car_management" , G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app , "activate" , G_CALLBACK (login_menu) , NULL);
    status = g_application_run (G_APPLICATION (app) , argc , argv);
    g_object_unref (app);

    return status;
}
