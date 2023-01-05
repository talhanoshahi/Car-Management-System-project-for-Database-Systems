#include <gtk-4.0/gtk/gtk.h>
#include <glib.h>
#include <pango/pango.h>
#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <sqlite3.h>

#define USERNAME_REGEX "^[a-zA-Z(0-9_)?]{3,30}$"
#define PASSWORD_REGEX "^(?=.*[A-Z])?(?=.*[a-z])(?=.*[0-9])[(A-Z)?a-z0-9]{8,}$"

/* Comment the upper password regex and uncomment this 
 * if you want mendatory upercase letters in the psassword */
/* #define PASSWORD_REGEX "^(?=.*[A-Z])(?=.*[a-z])(?=.*[0-9])[A-Za-z0-9]{8,}$" */

#define DATABASE_FOLDER "..database"
#define DATABASE_FILENAME "../database/car_management.db"
#define LOGIN_DATA_FILENAME "../database/login_session.mdb"

enum {UNSIGNED_INT_MAX_DIGITS = 10};

struct screen_resolution 
{
    int width;
    int height;
};

struct login_data
{
    bool is_logged_in;
    gchar *logged_in_username_hash;
    gchar *logged_in_password_hash;
};

struct login_check
{
    GtkWidget *grid;
    bool *logged_in;
};

int bot_get_file_size(char const *const filename);
void bot_create_project_database_files();
void bot_initialize_database(char const *const database_filename);
gchar * bot_sqlite3_create_table(sqlite3 *database, gchar const *table_name,
	gsize const attribute_count,
	gchar const *const attribute_detail[],
	gsize const constraint_count,
	gchar const *const constraint_detail[]);
gchar *bot_sqlite3_drop_table(sqlite3 *database, gchar const *const table_name);
gchar *bot_sqlite3_insert_values(sqlite3 *database,
	gchar const *const table_name,
	gsize const column_count,
	gchar const *const column_detail[],
	gsize const values_count,
	gchar const *const values_detail[]);
bool bot_sqlite3_check_value_exist(sqlite3 *database,
	gchar const *const table_name,
	gchar const *const column_name,
	unsigned int const compare_value);
struct screen_resolution bot_get_x11_default_screen_resolution();
int bot_g_regex_compare(gchar const *const str_compare,
	gchar const *const str_regex);
static void login_button_clicked(GtkWidget * button , gpointer data);
static void set_logging_in_error (bool * signup);
static void login_menu (GtkWidget * login_window , bool * error_logging_in , bool * loggedin);
static void bot_activate_gtk(GtkApplication *app, gpointer data);
int bot_create_gtk_application(int argc, char *argv[]);
unsigned int generate_hash (gchar const * string , gsize const string_size);
void check_is_logged_in(struct login_data * data);
bool login_successful (unsigned int const username_hash , unsigned int const password_hash);

int bot_get_file_size(char const *const filename)
{
    FILE * file;
    int size;

    file = fopen(filename , "r");

    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    fclose(file);

    return size;
}

unsigned int generate_hash (gchar const * string , gsize const string_size)
{
    unsigned int hash_key = 0;

    for (gsize i = 0 ; i < string_size ; i++)
    {
	hash_key = hash_key + string[i];
	hash_key = hash_key * string[i];
    }

    return hash_key % UINT_MAX;
}

void bot_create_project_database_files()
{
    gchar const database_directory[] = DATABASE_FOLDER;
    char const database_filename[] = DATABASE_FILENAME;
    gchar const login_session_filename[] = LOGIN_DATA_FILENAME;

    if (!g_file_test(database_directory, G_FILE_TEST_EXISTS))
    {
	g_mkdir_with_parents(database_directory, 0700);
    }

    if (!g_file_test(database_filename, G_FILE_TEST_EXISTS))
    {
	bot_initialize_database(database_filename);
    }

    if (!g_file_test(login_session_filename, G_FILE_TEST_EXISTS))
    {
	g_file_set_contents(login_session_filename, "login_false" , -1 , NULL);
    }
}

void bot_initialize_database(char const *const database_filename)
{
    sqlite3 * database_file;
    sqlite3_open(database_filename, &database_file);

    gchar const accounts_table_name[] = "accounts";
    gsize const accounts_table_attribute_count = 2;
    gchar const  * const accounts_table_attribute_detail[] =
    {
	"username_hash bigint not null unique" ,
	"password_hash bigint not null unique"
    };

    bot_sqlite3_create_table(database_file , accounts_table_name ,
	    accounts_table_attribute_count , accounts_table_attribute_detail ,
	    0 , NULL);

    gchar const admin_username[] = "admin";
    gchar const admin_password[] = "admin123";
    gchar * admin_username_hash = g_strdup_printf("%u" , generate_hash(admin_username, g_utf8_strlen(admin_username , -1)));
    gchar * admin_password_hash = g_strdup_printf("%u" , generate_hash(admin_password, g_utf8_strlen(admin_password , -1)));
    gchar const * admin_login_details[2];

    admin_login_details[0] = admin_username_hash;
    admin_login_details[1] = admin_password_hash;

    bot_sqlite3_insert_values(database_file, accounts_table_name, 0, NULL, 2,
	    admin_login_details);

    sqlite3_close(database_file);

    g_free(admin_username_hash);
    g_free(admin_password_hash);
}

gchar * bot_sqlite3_create_table(sqlite3 *database, gchar const *table_name,
	gsize const attribute_count,
	gchar const *const attribute_detail[],
	gsize const constraint_count,
	gchar const *const constraint_detail[])
{
    if (database == NULL)
    {
	return "NULL database pointer";
    }

    if (table_name == NULL)
    {
	return "Empty table name";
    }

    gchar * error;
    gchar * cat_strings[2];
    bool cat_str_index = 0;

    cat_strings[0] = g_strconcat ("create table if not exists " , table_name , "(" , NULL);
    cat_str_index = !cat_str_index;

    for (gsize i = 0; i < attribute_count; i = i + 1)
    {
	if (attribute_detail[i] != NULL)
	{
	    if (i == (attribute_count - 1))
	    {
		cat_strings[cat_str_index] =
		    g_strconcat(cat_strings[!cat_str_index], attribute_detail[i], NULL);
		cat_str_index = !cat_str_index;

		g_free(cat_strings[cat_str_index]);

		continue;
	    }

	    cat_strings[cat_str_index] = g_strconcat(cat_strings[!cat_str_index],
		    attribute_detail[i], ",", NULL);
	    cat_str_index = !cat_str_index;

	    g_free(cat_strings[cat_str_index]);
	}
    }

    if (constraint_count > 0)
    {
	cat_strings[cat_str_index] = g_strconcat (cat_strings[!cat_str_index] , "," , NULL);
	cat_str_index = !cat_str_index;

	g_free (cat_strings[cat_str_index]);

	for (gsize i = 0; i < constraint_count; i = i + 1)
	{
	    if (constraint_detail[i] != NULL)
	    {
		if (i == (constraint_count - 1))
		{
		    cat_strings[cat_str_index] = g_strconcat(cat_strings[!cat_str_index],
			    constraint_detail[i], NULL);

		    cat_str_index = !cat_str_index;
		    g_free(cat_strings[cat_str_index]);

		    continue;
		}

		cat_strings[cat_str_index] = g_strconcat(
			cat_strings[!cat_str_index], constraint_detail[i], ",", NULL);
		cat_str_index = !cat_str_index;

		g_free(cat_strings[cat_str_index]);
	    }
	}
    }

    cat_strings[cat_str_index] = g_strconcat (cat_strings[!cat_str_index] , ")" , NULL);

    g_print("Query: %s\n", cat_strings[cat_str_index]);
    sqlite3_exec(database , cat_strings[cat_str_index] , NULL , NULL , &error);

    g_free (cat_strings[cat_str_index]);
    g_free (cat_strings[!cat_str_index]);

    return error;
}

gchar *bot_sqlite3_drop_table(sqlite3 *database, gchar const *const table_name)
{
    if (database == NULL)
    {
	return "NULL database pointer";
    }

    if (table_name == NULL)
    {
	return "Empty table name";
    }

    gchar * error;
    gchar * query = g_strconcat("drop table if exists " , table_name, NULL);

    sqlite3_exec(database, query , NULL , NULL , &error);

    g_free(query);
    return error;
}

gchar *bot_sqlite3_insert_values(sqlite3 *database,
	gchar const *const table_name,
	gsize const column_count,
	gchar const *const column_detail[],
	gsize const values_count,
	gchar const *const values_detail[])
{
    if (database == NULL)
    {
	return "NULL database pointer";
    }

    if (table_name == NULL)
    {
	return "Empty table name";
    }

    if (column_count > 0 && values_count < column_count)
    {
	return "Lesser values than columns specified";
    }

    gchar * error;
    gchar * cat_strings[2];
    bool cat_str_index = 0;

    cat_strings[0] = g_strconcat ("insert into " , table_name , " " , NULL);
    cat_str_index = !cat_str_index;

    if (column_count > 0)
    {
	cat_strings[cat_str_index] = g_strconcat(cat_strings[!cat_str_index] ,"(", NULL);
	cat_str_index = !cat_str_index;
	g_free(cat_strings[cat_str_index]);

	for (gsize i = 0; i < column_count; i = i + 1)
	{
	    if (column_detail[i] != NULL)
	    {
		if (i == (column_count - 1))
		{
		    cat_strings[cat_str_index] = g_strconcat(
			    cat_strings[!cat_str_index], column_detail[i], ") ", NULL);
		    cat_str_index = !cat_str_index;

		    g_free(cat_strings[cat_str_index]);

		    continue;
		}

		cat_strings[cat_str_index] = g_strconcat(cat_strings[!cat_str_index],
			column_detail[i], ",", NULL);
		cat_str_index = !cat_str_index;

		g_free(cat_strings[cat_str_index]);
	    }
	}
    }

    cat_strings[cat_str_index] = g_strconcat(cat_strings[!cat_str_index] ,"values (", NULL);
    cat_str_index = !cat_str_index;
    g_free(cat_strings[cat_str_index]);

    for (gsize i = 0; i < values_count; i = i + 1)
    {
	if (values_detail != NULL)
	{
	    if (i == (values_count - 1))
	    {
		cat_strings[cat_str_index] = g_strconcat(cat_strings[!cat_str_index],
			values_detail[i], ")", NULL);

		continue;
	    }

	    cat_strings[cat_str_index] =
		g_strconcat(cat_strings[!cat_str_index], values_detail[i], ",", NULL);
	    cat_str_index = !cat_str_index;

	    g_free(cat_strings[cat_str_index]);
	}
    }

    sqlite3_exec(database , cat_strings[cat_str_index] , NULL , NULL , &error);

    g_free (cat_strings[cat_str_index]);
    g_free (cat_strings[!cat_str_index]);

    return error;
}

bool bot_sqlite3_check_value_exist(sqlite3 *database,
	gchar const *const table_name,
	gchar const *const column_name,
	unsigned int const compare_value)
{
    if (database == NULL)
    {
	return false;
    }

    if (table_name == NULL)
    {
	return false;
    }

    if (column_name == NULL)
    {
	return false;
    }

    g_print("Column Name: %s\n" , column_name);
    g_print("Column value: %u\n" , compare_value);
    
    int rc;
    unsigned int value;
    sqlite3_stmt *stmt;

    gchar compare_value_string[UNSIGNED_INT_MAX_DIGITS+1];
    g_snprintf(compare_value_string, UNSIGNED_INT_MAX_DIGITS+1, "%u", compare_value);

    gchar * query = g_strconcat("select " , column_name, " from ", table_name,
	    " where ", column_name, " = ", compare_value_string, NULL);
    g_print("Query: %s\n", query);

    rc = sqlite3_prepare_v2(database, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
	return false;  // error preparing statement
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW)
    {
	value = sqlite3_column_int64(stmt, 0);
	g_print("Value: %u\n" , value);
    }

    sqlite3_finalize(stmt);

    g_free (query);

    return value == compare_value;
}

struct screen_resolution
bot_get_x11_default_screen_resolution() {
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

int bot_g_regex_compare(gchar const *const str_compare,
	gchar const *const str_regex)
{
    GError * error = NULL;
    GRegex * regex;
    GMatchInfo * match_info;

    regex = g_regex_new (str_regex , 0 , 0 , &error);

    if (error)
    {
	g_error_free (error);
	return 1;
    }

    g_regex_match_full (regex , str_compare , g_utf8_strlen (str_compare , -1) , 0 , 0 , &match_info , &error);
    if (g_match_info_matches(match_info))
    {
	g_match_info_free (match_info);
	g_regex_unref (regex);

	return 0;
    }

    g_regex_unref (regex);
    return -1;
}

bool login_successful(unsigned int const username_hash,
	unsigned int const password_hash)
{
    char login_session_filename[] = LOGIN_DATA_FILENAME;
    FILE * file;
    file = fopen(login_session_filename, "w+");

    if (file == NULL)
    {
	return false;
    }

    fprintf(file, "login_true\n");
    fprintf(file, "%u\n", username_hash);
    fprintf(file, "%u", password_hash);

    fclose(file);

    return true;
}

static void login_button_clicked(GtkWidget * button , gpointer data)
{  
    GtkWidget * grid;

    GtkWidget * entry_username;
    GtkWidget * entry_password;
    GtkWidget * label_login_error;

    const gchar * username;
    const gchar * password;

    grid = GTK_WIDGET(data);

    entry_username= gtk_grid_get_child_at (GTK_GRID (grid) , 1 , 0);
    entry_password = gtk_grid_get_child_at (GTK_GRID (grid) , 1 , 1);
    label_login_error = gtk_grid_get_child_at (GTK_GRID (grid) , 0 , 2);

    username = gtk_editable_get_text (GTK_EDITABLE (entry_username));
    password = gtk_editable_get_text (GTK_EDITABLE (entry_password));

    gsize username_len = g_utf8_strlen(username, -1);

    if (username_len == 0)
    {
	gtk_label_set_label (GTK_LABEL (label_login_error) , "Username field empty");
	return;
    }

    int username_regex_match_return = bot_g_regex_compare(username, USERNAME_REGEX);

    if (username_regex_match_return == -1)
    {
	gtk_label_set_label (GTK_LABEL (label_login_error) , "Username not allowed");
	return;
    }

    gsize password_len = g_utf8_strlen(password, -1);

    if (password_len == 0)
    {
	gtk_label_set_label (GTK_LABEL (label_login_error) , "Password field empty");
	return;
    }

    int password_regex_match_return = bot_g_regex_compare(password, PASSWORD_REGEX);

    if (password_regex_match_return == -1)
    {
	gtk_label_set_label (GTK_LABEL (label_login_error) , "Password not allowed");
	return;
    }

    gtk_label_set_label (GTK_LABEL (label_login_error) , "");

    char const database_filename[] = DATABASE_FILENAME;
    sqlite3 * database_file;
    sqlite3_open(database_filename, &database_file);

    gchar const table_name[] = "accounts";
    gchar const username_hash_column[] = "username_hash";
    gchar const password_hash_column[] = "password_hash";

    unsigned int username_hash = generate_hash(username, username_len);
    bool username_hash_compare = bot_sqlite3_check_value_exist(database_file , table_name, username_hash_column, username_hash);

    if (username_hash_compare == false)
    {
	gtk_label_set_label (GTK_LABEL (label_login_error) , "Username not found");
	sqlite3_close(database_file);
	return;
    }

    unsigned int password_hash = generate_hash(password, password_len);
    bool password_hash_compare = bot_sqlite3_check_value_exist(database_file , table_name, password_hash_column, password_hash);
    sqlite3_close(database_file);

    if (password_hash_compare == false)
    {
	gtk_label_set_label (GTK_LABEL (label_login_error) , "Password not found");
	return;
    }

    gtk_label_set_label (GTK_LABEL (label_login_error) , "Login Successful");

    login_successful(username_hash, password_hash);
}

static void set_logging_in_error (bool * signup)
{
    if (signup != NULL)
    {
	*signup = true;
	g_print("Signup: %d\n" , *signup);
    }
}

static void login_menu (GtkWidget * login_window , bool * error_logging_in , bool * loggedin)
{
    GtkWidget * box_main;
    GtkWidget * box_login;
    GtkWidget * box_creator;
    GtkWidget * box_buttons_and_credentials;

    GtkWidget * grid_credentials;
    GtkWidget * grid_buttons;

    GtkWidget * label_creator;
    GtkWidget * label_heading_login;
    GtkWidget * label_username;
    GtkWidget * label_password;
    GtkWidget * label_login_wrong_input_error;

    GtkWidget * entry_username;
    GtkWidget * entry_password;

    GtkWidget * button_login;
    GtkWidget * button_error_logging_in;

    PangoAttrList * attrs;
    PangoAttribute * weight;

    GtkCssProvider * login_css_provider;
    GtkCssProvider * credentials_css_provider;

    GtkStyleContext * login_style_context;
    GtkStyleContext * credentials_style_context;

    struct screen_resolution resolution = bot_get_x11_default_screen_resolution ();

    gtk_window_set_title(GTK_WINDOW(login_window), "Login Window");
    gtk_window_set_default_size(GTK_WINDOW(login_window), resolution.width, resolution.height);

    box_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 50);
    gtk_widget_set_halign(box_main, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box_main, GTK_ALIGN_CENTER);

    box_login = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(box_login, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box_login, GTK_ALIGN_CENTER);

    box_creator = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(box_creator, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box_creator, GTK_ALIGN_CENTER);

    box_buttons_and_credentials = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(box_buttons_and_credentials, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box_buttons_and_credentials, GTK_ALIGN_CENTER);

    grid_credentials = gtk_grid_new ();
    gtk_grid_set_row_homogeneous (GTK_GRID (grid_credentials) , true);
    gtk_grid_set_column_homogeneous (GTK_GRID (grid_credentials) , true);

    label_creator = gtk_label_new ("This project is created by: Muhammad Talha Noshahi");
    label_heading_login = gtk_label_new ("Login");
    label_username = gtk_label_new ("Username: ");
    label_password = gtk_label_new ("Password: ");
    label_login_wrong_input_error = gtk_label_new ("");

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
    gtk_grid_attach (GTK_GRID (grid_credentials) , label_login_wrong_input_error , 0 , 2 , 2 , 1);

    entry_username = gtk_entry_new ();
    entry_password = gtk_entry_new ();
    gtk_entry_set_visibility (GTK_ENTRY (entry_password) , false);

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
    button_error_logging_in = gtk_button_new_with_label ("Error logging in?");

    gtk_button_set_has_frame(GTK_BUTTON(button_error_logging_in), false);

    grid_buttons = gtk_grid_new ();
    gtk_widget_set_halign(grid_buttons ,GTK_ALIGN_CENTER);
    gtk_widget_set_valign(grid_buttons ,GTK_ALIGN_CENTER);
    gtk_grid_attach (GTK_GRID (grid_buttons) , button_login , 0 , 0 , 1 , 1);
    gtk_grid_attach (GTK_GRID (grid_buttons) , button_error_logging_in , 0 , 1 , 1 , 1);

    gtk_box_append (GTK_BOX (box_buttons_and_credentials) , grid_credentials);
    gtk_box_append (GTK_BOX (box_buttons_and_credentials) , grid_buttons);

    gtk_box_append (GTK_BOX (box_main) , box_login);
    gtk_box_append (GTK_BOX (box_main), box_buttons_and_credentials);
    gtk_box_append (GTK_BOX (box_main), box_creator);

    gtk_window_set_child(GTK_WINDOW(login_window), box_main);

    g_signal_connect(button_login, "clicked", G_CALLBACK(login_button_clicked), (gpointer) grid_credentials);
    g_signal_connect_swapped(button_error_logging_in, "clicked" , G_CALLBACK(gtk_window_destroy) , GTK_WINDOW(login_window));
    g_signal_connect_swapped(button_error_logging_in, "clicked" , G_CALLBACK(set_logging_in_error) , error_logging_in);

    gtk_widget_show(login_window);

    pango_attr_list_unref(attrs);
}

void check_is_logged_in(struct login_data * data)
{
    // ".mdb" extension means my_database
    gchar const filename[] = LOGIN_DATA_FILENAME;

    FILE * file;
    size_t filesize;

    file = fopen(filename , "r");
    if (!file)
    {
	data->is_logged_in = false;
	return;
    }

    filesize = bot_get_file_size(filename);
    char logged_in[filesize+1];
    char comparison_str[] = "login_true";

    fscanf(file ,"%s" , logged_in);

    size_t logged_in_len = strlen(logged_in);
    size_t comparison_str_len = strlen(comparison_str);
    size_t comp_size = (comparison_str_len < logged_in_len) ? comparison_str_len : logged_in_len;

    if (strncmp(logged_in, comparison_str, comp_size) != 0)
    {
	data->is_logged_in = false;
	fclose(file);

	return;
    }

    data->is_logged_in = true;

    unsigned int username_hash_int;
    unsigned int password_hash_int;

    fscanf(file, "%u" , &username_hash_int);
    fscanf(file, "%u" , &password_hash_int);
    fclose(file);

    g_snprintf(data->logged_in_username_hash, filesize+1 , "%u" , username_hash_int);
    g_snprintf(data->logged_in_password_hash, filesize+1 , "%u" , password_hash_int);

}

static void bot_activate_gtk(GtkApplication *app, gpointer data)
{
    struct login_data * login = g_malloc0(sizeof(struct login_data));
    check_is_logged_in(login);
    g_print("Logged In status: %d\n" , login->is_logged_in);

    bool error_logging_in = false;

    if (login->is_logged_in == false)
    {
	GtkWidget * login_window;

	login_window = gtk_application_window_new(app);
	login_menu(login_window , &error_logging_in , &login->is_logged_in);

	while (error_logging_in != true && login->is_logged_in != true)
	{
	    check_is_logged_in(login);
	    g_main_context_iteration(NULL , true);
	}

	gtk_window_destroy(GTK_WINDOW(login_window));
    }

    if (login->is_logged_in == false && error_logging_in == true) {
	/* GtkWidget * */
	g_print("Signing up\n");
    }
}

int bot_create_gtk_application(int argc, char *argv[])
{
    GtkApplication *car_management;
    int status;

    /* bot_create_project_database_files(); */

    car_management =
	gtk_application_new("bot.dbs.car_management", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(car_management, "activate", G_CALLBACK(bot_activate_gtk), NULL);
    status = g_application_run(G_APPLICATION(car_management), argc, argv);
    g_object_unref(car_management);

    return status;
}

int main(int argc, char *argv[])
{
    int status;

    status = bot_create_gtk_application (argc , argv);

    return status;
}
