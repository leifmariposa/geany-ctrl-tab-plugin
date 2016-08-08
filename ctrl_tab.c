/*
 *
 *  Copyright (C) 2016  Leif Persson <leifmariposa@hotmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <glib.h>
#include <glib/gprintf.h>
#include <geanyplugin.h>
#include <libgen.h>


#define D(x) /*x*/


/**********************************************************************/
static const char *PLUGIN_NAME = "Ctrl+Tab";
static const char *PLUGIN_DESCRIPTION = "A Ctrl+Tab replacement";
static const char *PLUGIN_VERSION = "0.1";
static const char *PLUGIN_AUTHOR = "Leif Persson <leifmariposa@hotmail.com>";
static const int   WINDOW_WIDTH = 720;
static const int   WINDOW_HEIGHT = 500;


/**********************************************************************/
GeanyPlugin *geany_plugin;
GArray *document_activation_order_array;


/**********************************************************************/
enum direction
{
	DIRECTION_UP,
	DIRECTION_DOWN,
} direction;


/**********************************************************************/
enum tool_window_location
{
	NONE,
	SIDEBAR,
	MESSAGE_WINDOW
} tool_window_location;


/**********************************************************************/
enum
{
	KB_CTRL_TAB,
	KB_COUNT
};


/**********************************************************************/
enum
{
	COL_ICON = 0,
	COL_SHORT_NAME = 1,
	COL_FILE_NAME = 2,
	COL_FILE_TYPE = 3,
	COL_REAL_PATH = 4,
	COL_FILE_TITLE = 5,
	COL_CHANGED = 6,
	COL_TOOL_WINDOW_LOCATION = 7,
	COL_TOOL_WINDOW_TAB = 8,
	COL_DOCUMENT_ID = 9,
	NUM_COLS
};


/**********************************************************************/
struct TREE_DATA
{
	GtkWidget           *tree_view;
	GtkTreeSelection    *selection;
	GtkTreeModel        *model;
	struct PLUGIN_DATA  *plugin_data;
} TREE_DATA;


/**********************************************************************/
struct PLUGIN_DATA
{
	struct TREE_DATA    tool_windows_tree;
	struct TREE_DATA    files_tree;
	GtkWidget           *main_window;
	GtkWidget           *label_short_name;
	GtkWidget           *label_file_type_desc;
	GtkWidget           *label_file_title;
	GtkWidget           *label_file_name;
	GtkWidget           *label_real_path;
	GtkWidget           *label_real_path_desc;
} PLUGIN_DATA;


/**********************************************************************/
D(static void log_debug(const gchar* s, ...)
{
	gchar* format = g_strconcat("[CTR DEBUG] : ", s, "\n", NULL);
	va_list l;
	va_start(l, s);
	g_vprintf(format, l);
	g_free(format);
	va_end(l);
})


/**********************************************************************/
static void on_document_activate(G_GNUC_UNUSED GObject *obj, GeanyDocument *doc, G_GNUC_UNUSED gpointer user_data)
{
	guint index;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	for (index = 0; index < document_activation_order_array->len; ++index)
	{
		if (doc && doc->is_valid && (guint)g_array_index(document_activation_order_array, gint, index) == doc->id)
		{
			g_array_remove_index(document_activation_order_array, index);
			break;
		}
	}
	g_array_append_val(document_activation_order_array, doc->id);
}


/**********************************************************************/
static GtkTreeModel* get_files()
{
	GtkListStore *store;
	GtkTreeIter iter;
	gint index;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	store = gtk_list_store_new(NUM_COLS,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_BOOLEAN,
		G_TYPE_UINT,
		G_TYPE_UINT,
		G_TYPE_UINT);

	index = document_activation_order_array->len;
	while (index-- > 0)
	{
		guint i;
		guint id = g_array_index(document_activation_order_array, gint, index);
		for (i = 0; i < geany_plugin->geany_data->documents_array->len; ++i)
		{
			GeanyDocument *doc = g_ptr_array_index(geany_plugin->geany_data->documents_array, i);
			if (doc && doc->is_valid && doc->id == id)
			{
				gtk_list_store_append(store, &iter);
				if (doc->file_name)
				{
					if (g_strcmp0(doc->file_name, doc->real_path) == 0)
					{
						if (strcmp(doc->file_type->title, "None") == 0)
							gtk_list_store_set(store, &iter, COL_ICON, "", -1);
						else
							gtk_list_store_set(store, &iter, COL_ICON, GTK_STOCK_FILE, -1);
					}
					else
					{
						gtk_list_store_set(store, &iter, COL_ICON, GTK_STOCK_REDO, -1);
					}

					gtk_list_store_set(store, &iter, COL_SHORT_NAME, basename(doc->file_name), -1);
					gtk_list_store_set(store, &iter, COL_FILE_NAME, doc->file_name, -1);
				}
				else
				{
					gtk_list_store_set(store, &iter, COL_ICON, "", -1);
					gtk_list_store_set(store, &iter, COL_SHORT_NAME, "untitled", -1);
					gtk_list_store_set(store, &iter, COL_FILE_NAME, "", -1);
				}

				gtk_list_store_set(store, &iter, COL_FILE_TYPE, doc->file_type->name, -1);
				gtk_list_store_set(store, &iter, COL_REAL_PATH, doc->real_path, -1);
				gtk_list_store_set(store, &iter, COL_FILE_TITLE, doc->file_type->title, -1);
				gtk_list_store_set(store, &iter, COL_CHANGED, doc->changed, -1);
				gtk_list_store_set(store, &iter, COL_TOOL_WINDOW_LOCATION, NONE, -1);
				gtk_list_store_set(store, &iter, COL_TOOL_WINDOW_TAB, 0, -1);
				gtk_list_store_set(store, &iter, COL_DOCUMENT_ID, doc->id, -1);
			}
		}
	}

	return GTK_TREE_MODEL(store);
}


/**********************************************************************/
static GtkTreeModel* get_tool_windows()
{
	GtkListStore *store;
	GtkTreeIter iter;
	GtkNotebook *snb;
	GtkNotebook *mnb;
	gint page_count;
	gint i;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	store = gtk_list_store_new(NUM_COLS,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_BOOLEAN,
		G_TYPE_UINT,
		G_TYPE_UINT,
		G_TYPE_UINT);

	snb = GTK_NOTEBOOK(geany_plugin->geany_data->main_widgets->sidebar_notebook);
	page_count = gtk_notebook_get_n_pages(snb);
	for (i = 0; i < page_count; i++)
	{
		GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(snb), i);
		const gchar *text = gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(snb), page);

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, COL_ICON, "", -1);
		gtk_list_store_set(store, &iter, COL_SHORT_NAME, text, -1);
		gtk_list_store_set(store, &iter, COL_FILE_NAME, "", -1);
		gtk_list_store_set(store, &iter, COL_FILE_TYPE, "", -1);
		gtk_list_store_set(store, &iter, COL_REAL_PATH, "", -1);
		gtk_list_store_set(store, &iter, COL_FILE_TITLE, "Tool window", -1);
		gtk_list_store_set(store, &iter, COL_CHANGED, FALSE, -1);
		gtk_list_store_set(store, &iter, COL_TOOL_WINDOW_LOCATION, SIDEBAR, -1);
		gtk_list_store_set(store, &iter, COL_TOOL_WINDOW_TAB, i, -1);
		gtk_list_store_set(store, &iter, COL_DOCUMENT_ID, 0, -1);
	}

	mnb = GTK_NOTEBOOK(geany_plugin->geany_data->main_widgets->message_window_notebook);
	page_count = gtk_notebook_get_n_pages(mnb);
	for (i = 0; i < page_count; i++)
	{
		GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(mnb), i);
		const gchar *text = gtk_notebook_get_tab_label_text(GTK_NOTEBOOK(mnb), page);

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, COL_ICON, "", -1);
		gtk_list_store_set(store, &iter, COL_SHORT_NAME, text, -1);
		gtk_list_store_set(store, &iter, COL_FILE_NAME, "", -1);
		gtk_list_store_set(store, &iter, COL_FILE_TYPE, "", -1);
		gtk_list_store_set(store, &iter, COL_REAL_PATH, "", -1);
		gtk_list_store_set(store, &iter, COL_FILE_TITLE, "Tool window", -1);
		gtk_list_store_set(store, &iter, COL_CHANGED, FALSE, -1);
		gtk_list_store_set(store, &iter, COL_TOOL_WINDOW_LOCATION, MESSAGE_WINDOW, -1);
		gtk_list_store_set(store, &iter, COL_TOOL_WINDOW_TAB, i, -1);
		gtk_list_store_set(store, &iter, COL_DOCUMENT_ID, 0, -1);
	}

	return GTK_TREE_MODEL(store);
}


/**********************************************************************/
gboolean view_selection_func(G_GNUC_UNUSED GtkTreeSelection *selection,
                             GtkTreeModel *model,
                             GtkTreePath *path,
                             gboolean path_currently_selected,
                             gpointer data)
{
	struct TREE_DATA *tree_data = data;
	GtkTreeIter iter;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	if (gtk_tree_model_get_iter(model, &iter, path))
	{
		if (!path_currently_selected)
		{
			gchar *short_name;
			gchar *file_title;
			gchar *file_name;
			gchar *real_path;
			gtk_tree_model_get(model, &iter,
				COL_SHORT_NAME, &short_name,
				COL_FILE_TITLE, &file_title,
				COL_FILE_NAME, &file_name,
				COL_REAL_PATH, &real_path,
				-1);

			gtk_label_set_text(GTK_LABEL(tree_data->plugin_data->label_short_name), short_name);

			if (file_title && strcmp(file_title, "None") == 0)
			{
				gtk_label_set_text(GTK_LABEL(tree_data->plugin_data->label_file_title), "");
				gtk_label_set_text(GTK_LABEL(tree_data->plugin_data->label_file_type_desc), "");
			}
			else
			{
				gtk_label_set_text(GTK_LABEL(tree_data->plugin_data->label_file_title), file_title);
				gtk_label_set_text(GTK_LABEL(tree_data->plugin_data->label_file_type_desc), "File Type:");
			}

			gtk_label_set_text(GTK_LABEL(tree_data->plugin_data->label_file_name), dirname(g_strdup(file_name)));
			if (real_path && strcmp(file_name, real_path) == 0)
			{
				gtk_label_set_text(GTK_LABEL(tree_data->plugin_data->label_real_path), "");
				gtk_label_set_text(GTK_LABEL(tree_data->plugin_data->label_real_path_desc), "");
			}
			else
			{
				gtk_label_set_text(GTK_LABEL(tree_data->plugin_data->label_real_path), dirname(g_strdup(real_path)));
				gtk_label_set_text(GTK_LABEL(tree_data->plugin_data->label_real_path_desc), "Real Path:");
			}
			g_free(short_name);
			g_free(file_title);
			g_free(file_name);
			g_free(real_path);
		}
	}

	return TRUE;
}


/**********************************************************************/
static void create_model(struct TREE_DATA *tree_data, GtkTreeModel *list)
{
	GtkTreeModel *filter;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	filter = gtk_tree_model_filter_new(list, NULL);
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree_data->tree_view), filter);
	g_object_unref(filter);

	tree_data->selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_data->tree_view));
	gtk_tree_selection_set_mode(tree_data->selection, GTK_SELECTION_BROWSE);
	gtk_tree_selection_set_select_function(tree_data->selection, view_selection_func, tree_data, NULL);

	tree_data->model = gtk_tree_view_get_model(GTK_TREE_VIEW(tree_data->tree_view));
}


/**********************************************************************/
void render_cell(G_GNUC_UNUSED GtkTreeViewColumn *col,
				 GtkCellRenderer *renderer,
				 GtkTreeModel *model,
				 GtkTreeIter *iter,
				 G_GNUC_UNUSED gpointer user_data)
{
	gchar *short_name;
	gboolean changed;
	gtk_tree_model_get(model, iter,
		COL_SHORT_NAME, &short_name,
		COL_CHANGED, &changed,
		-1);

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	if (changed) {
		gchar *markup = g_markup_printf_escaped("<span foreground='red'>%s</span>", short_name);
		g_object_set(renderer, "markup", markup, NULL);
		g_free(markup);
	} else {
		g_object_set(renderer, "markup", NULL, NULL);
		g_object_set(renderer, "text", short_name, NULL);
	}

	g_free(short_name);
}


/**********************************************************************/
GtkWidget* find_child(GtkWidget* parent, GType type)
{
	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	D(GType t = G_TYPE_FROM_INSTANCE(parent));
	D(const gchar *name = g_type_name(t));
	D(log_debug("%s:%s type: %d, name: %s", __FILE__, __FUNCTION__, t, name));

	if (type == G_TYPE_FROM_INSTANCE(parent))
		return parent;

	if (GTK_IS_BIN(parent))
	{
		GtkWidget *child = gtk_bin_get_child(GTK_BIN(parent));
		return find_child(child, type);
	}

	if (GTK_IS_CONTAINER(parent))
	{
		GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
		while ((children = g_list_next(children)) != NULL)
		{
			GtkWidget* widget = find_child(children->data, type);
			if (widget != NULL)
				return widget;
		}
	}

	return NULL;
}


/**********************************************************************/
void activate_tool_window(GtkNotebook *notebook, guint tool_window_tab)
{
	GtkWidget *tree_view;
	GtkWidget *page;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook), tool_window_tab);
	gtk_notebook_set_current_page(notebook, tool_window_tab);
	tree_view = find_child(page, GTK_TYPE_TREE_VIEW);
	if (tree_view)
	{
		gtk_widget_grab_focus(tree_view);
	}
	else
	{
		GtkWidget *text_view = find_child(page, GTK_TYPE_TEXT_VIEW);
		if (text_view)
			gtk_widget_grab_focus(text_view);
	}
}


/**********************************************************************/
void activate_selected_function_and_quit(struct PLUGIN_DATA *plugin_data)
{
	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	if (gtk_widget_has_focus(plugin_data->tool_windows_tree.tree_view))
	{
		GtkTreePath *path = NULL;
		gtk_tree_view_get_cursor(GTK_TREE_VIEW(plugin_data->tool_windows_tree.tree_view), &path, NULL);
		if (path)
		{
			guint location;
			guint tool_window_tab;

			GtkTreeIter iter;
			gtk_tree_model_get_iter(plugin_data->tool_windows_tree.model, &iter, path);

			gtk_tree_model_get(plugin_data->tool_windows_tree.model, &iter,
				COL_TOOL_WINDOW_LOCATION, &location,
				COL_TOOL_WINDOW_TAB, &tool_window_tab,
				-1);

			if (location == SIDEBAR)
				activate_tool_window(GTK_NOTEBOOK(geany_plugin->geany_data->main_widgets->sidebar_notebook), tool_window_tab);
			else if (location == MESSAGE_WINDOW)
				activate_tool_window(GTK_NOTEBOOK(geany_plugin->geany_data->main_widgets->message_window_notebook), tool_window_tab);

			gtk_tree_path_free(path);
		}
	}
	else if (gtk_widget_has_focus(plugin_data->files_tree.tree_view))
	{
		GtkTreePath *path = NULL;
		gtk_tree_view_get_cursor(GTK_TREE_VIEW(plugin_data->files_tree.tree_view ), &path, NULL);
		if(path != NULL)
		{
			GtkTreeIter iter;
			if(gtk_tree_model_get_iter(plugin_data->files_tree.model, &iter, path))
			{
				GeanyDocument *doc;
				guint id = 0;
				gtk_tree_model_get(plugin_data->files_tree.model, &iter, COL_DOCUMENT_ID, &id, -1);
				doc = document_find_by_id(id);
				if(doc && doc->is_valid)
				{
					gtk_notebook_set_current_page(GTK_NOTEBOOK(geany_plugin->geany_data->main_widgets->notebook), document_get_notebook_page(doc));
					gtk_widget_grab_focus(GTK_WIDGET(doc->editor->sci));
				}
			}
			gtk_tree_path_free(path);
		}
	}
	gtk_widget_destroy(plugin_data->main_window);
	g_free(plugin_data);
}


/**********************************************************************/
void view_on_row_activated(G_GNUC_UNUSED GtkTreeView *treeview,
                            G_GNUC_UNUSED GtkTreePath *path,
                            G_GNUC_UNUSED GtkTreeViewColumn *col,
                            gpointer data)
{
	struct PLUGIN_DATA *plugin_data = data;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	activate_selected_function_and_quit(plugin_data);
}


/**********************************************************************/
gboolean tree_view_focus_out_event(G_GNUC_UNUSED GtkWidget *widget,
                                    G_GNUC_UNUSED GdkEvent  *event,
                                    gpointer user_data)
{
	struct TREE_DATA *tree_data = user_data;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	gtk_tree_selection_unselect_all(tree_data->selection);

	return FALSE;
}


/**********************************************************************/
gboolean view_on_button_pressed(G_GNUC_UNUSED GtkWidget *treeview,
                                 GdkEventButton *event,
                                 gpointer userdata)
{
	struct TREE_DATA *tree_data = userdata;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	/* Single click with the left mouse button */
	if (event->type == GDK_BUTTON_PRESS  &&  event->button == 1)
	{
		GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_data->tree_view));
		if (gtk_tree_selection_count_selected_rows(selection) < 1)
		{
			GtkTreePath *path = NULL;
			if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(tree_data->tree_view),
											(gint) event->x,
											(gint) event->y,
											&path, NULL, NULL, NULL))
			{
				gtk_tree_selection_unselect_all(selection);
				gtk_tree_selection_select_path(selection, path);
				gtk_tree_path_free(path);
			}
		}
	}

	return FALSE; /* we did not handle this */
}


/**********************************************************************/
static void create_tree_view(struct TREE_DATA *tree_data)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	tree_data->tree_view = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_data->tree_view ),FALSE);
	g_signal_connect(tree_data->tree_view, "row-activated", (GCallback) view_on_row_activated, tree_data->plugin_data);
	g_signal_connect(tree_data->tree_view, "focus-out-event", G_CALLBACK(tree_view_focus_out_event), tree_data);
	g_signal_connect(tree_data->tree_view, "button-press-event", (GCallback) view_on_button_pressed, tree_data);

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree_data->tree_view ), -1, "icon", renderer, "stock-id", COL_ICON, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree_data->tree_view ), -1, "file_name", renderer, "text", COL_SHORT_NAME, NULL);
	column = gtk_tree_view_get_column(GTK_TREE_VIEW(tree_data->tree_view ), COL_SHORT_NAME);
	gtk_tree_view_column_set_cell_data_func(column, renderer, render_cell, NULL, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree_data->tree_view ), -1, "full_file_name", renderer, "text", COL_FILE_NAME, NULL);
	column = gtk_tree_view_get_column(GTK_TREE_VIEW(tree_data->tree_view ), COL_FILE_NAME);
	gtk_tree_view_column_set_visible(column, FALSE);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree_data->tree_view ), -1, "file_type", renderer, "text", COL_FILE_TYPE, NULL);
	column = gtk_tree_view_get_column(GTK_TREE_VIEW(tree_data->tree_view ), COL_FILE_TYPE);
	gtk_tree_view_column_set_visible(column, FALSE);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree_data->tree_view ), -1, "real_path", renderer, "text", COL_REAL_PATH, NULL);
	column = gtk_tree_view_get_column(GTK_TREE_VIEW(tree_data->tree_view ), COL_REAL_PATH);
	gtk_tree_view_column_set_visible(column, FALSE);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree_data->tree_view ), -1, "file_title", renderer, "text", COL_FILE_TITLE, NULL);
	column = gtk_tree_view_get_column(GTK_TREE_VIEW(tree_data->tree_view ), COL_FILE_TITLE);
	gtk_tree_view_column_set_visible(column, FALSE);
}


/**********************************************************************/
GtkTreeIter get_first_iter(GtkTreeView *tree_view)
{
	GtkTreeIter iter;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	gtk_tree_model_get_iter_first(gtk_tree_view_get_model(tree_view), &iter);

	return iter;
}


/**********************************************************************/
GtkTreeIter get_last_iter(GtkTreeView *tree_view)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gint rows;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	model = gtk_tree_view_get_model(tree_view);
	rows = gtk_tree_model_iter_n_children(model, NULL);
	path = gtk_tree_path_new_from_indices(rows - 1, -1);
	gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);

	return iter;
}


/**********************************************************************/
gboolean move_selection_to_other_tree(struct TREE_DATA *from, struct TREE_DATA *to)
{
	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	if (gtk_widget_has_focus(from->tree_view) &&
		gtk_tree_model_iter_n_children(to->model, NULL) > 0)
	{
		GtkTreePath *path = NULL;
		gtk_tree_view_get_cursor(GTK_TREE_VIEW(from->tree_view), &path, NULL);
		if (path)
		{
			gtk_tree_view_set_cursor(GTK_TREE_VIEW(to->tree_view), path, NULL, FALSE);
			gtk_tree_selection_unselect_all(from->selection);
			gtk_widget_grab_focus(to->tree_view);

			gtk_tree_path_free(path);
		}
		return TRUE;
	}
	return FALSE;
}


/**********************************************************************/
gboolean move_selection(struct PLUGIN_DATA *plugin_data, enum direction dir)
{
	struct TREE_DATA *tree_data = &plugin_data->files_tree;
	GtkTreePath *path = NULL;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	if (gtk_widget_has_focus(plugin_data->tool_windows_tree.tree_view))
		tree_data = &plugin_data->tool_windows_tree;

	if (dir == DIRECTION_DOWN)
	{
		GtkTreeIter last_iter;

		gtk_tree_view_get_cursor(GTK_TREE_VIEW(tree_data->tree_view), &path, NULL);
		if(path != NULL)
		{
			last_iter = get_last_iter(GTK_TREE_VIEW(tree_data->tree_view));
			if(gtk_tree_selection_iter_is_selected(tree_data->selection, &last_iter))
				return TRUE;

			gtk_tree_path_next(path);
		}
	}
	else if (dir == DIRECTION_UP)
	{
		GtkTreeIter first_iter;

		gtk_tree_view_get_cursor(GTK_TREE_VIEW(tree_data->tree_view), &path, NULL);
		if(path != NULL)
		{
			first_iter = get_first_iter(GTK_TREE_VIEW(tree_data->tree_view));
			if(gtk_tree_selection_iter_is_selected(tree_data->selection, &first_iter))
				return TRUE;

			gtk_tree_path_prev(path);
		}
	}

	if (path)
	{
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(tree_data->tree_view), path, NULL, FALSE);
		gtk_tree_path_free(path);
	}

	return TRUE;
}


/**********************************************************************/
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	struct PLUGIN_DATA * plugin_data = data;
	GtkWidgetClass *widget_class;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	switch(event->keyval)
	{
	case 65307: /* Escape */
		gtk_widget_destroy(plugin_data->main_window);
		g_free(plugin_data);
		break;
	case 0xff51: /* GDK_Left */
		return move_selection_to_other_tree(&plugin_data->files_tree, &plugin_data->tool_windows_tree);
		break;
	case 0xff53: /* GDK_Right */
		return move_selection_to_other_tree(&plugin_data->tool_windows_tree, &plugin_data->files_tree);
		break;
	case 0xff09: /* GDK_Tab */
	case 0xfe20: /* GDK_ISO_Left_Tab */
		if (event->state & GDK_SHIFT_MASK) {
			return move_selection(plugin_data, DIRECTION_UP);
		} else {
			return move_selection(plugin_data, DIRECTION_DOWN);
		}
		break;
	}

	event->state &= ~GDK_CONTROL_MASK;
	widget_class = GTK_WIDGET_GET_CLASS(widget);
	if (widget_class->key_press_event)
		widget_class->key_press_event(widget, event);

	return TRUE;
}


/**********************************************************************/
static gboolean on_key_release(G_GNUC_UNUSED GtkWidget *widget,
                                     G_GNUC_UNUSED GdkEventKey *event,
                                     gpointer data)
{
	struct PLUGIN_DATA * plugin_data = data;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	switch(event->keyval)
	{
	case 0xffe3: /* GDK_Control_L */
	case 0xffe4: /* GDK_Control_R */
		activate_selected_function_and_quit(plugin_data);
		break;
	}

	return FALSE;
}


/**********************************************************************/
static gboolean quit_ctr(G_GNUC_UNUSED GtkWidget *widget,
                        G_GNUC_UNUSED GdkEvent *event,
                        G_GNUC_UNUSED gpointer data)
{
	return FALSE;
}


/**********************************************************************/
GtkWidget *add_label(GtkWidget *grid,
                     const gchar *text,
                     guint left_attach,
                     guint right_attach,
                     guint top_attach,
                     guint bottom_attach,
                     GtkAttachOptions xoptions,
                     const gchar *markup)
{
	GtkWidget *label;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	label = gtk_label_new(text);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	if (markup)
		gtk_label_set_markup(GTK_LABEL(label), markup);
	gtk_table_attach(GTK_TABLE(grid), label, left_attach, right_attach, top_attach, bottom_attach, xoptions, 0, 0, 0);

	return label;
}


/**********************************************************************/
int launch_widget(void)
{
	struct PLUGIN_DATA *plugin_data;
	GtkWidget *scrolled_tool_windows_window;
	GtkWidget *scrolled_file_list_window;
	GtkWidget *main_grid;
	GtkWidget *top_grid;
	GtkWidget *middle_grid;
	GtkWidget *bottom_grid;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	plugin_data =  g_malloc(sizeof(PLUGIN_DATA));
	plugin_data->files_tree.plugin_data = plugin_data;
	plugin_data->tool_windows_tree.plugin_data = plugin_data;

	plugin_data->main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_modal(GTK_WINDOW(plugin_data->main_window), TRUE);
	gtk_container_set_border_width(GTK_CONTAINER(plugin_data->main_window), 6);

	create_tree_view(&plugin_data->tool_windows_tree);
	create_tree_view(&plugin_data->files_tree);
	create_model(&plugin_data->tool_windows_tree, get_tool_windows());
	create_model(&plugin_data->files_tree, get_files());

	main_grid = gtk_table_new(3, 1, FALSE);
	top_grid = gtk_table_new(3, 2, FALSE);
	middle_grid = gtk_table_new(2, 2, FALSE);
	bottom_grid = gtk_table_new(2, 2, FALSE);

	gtk_table_set_row_spacings(GTK_TABLE(main_grid), 8);
	gtk_table_set_col_spacings(GTK_TABLE(main_grid), 0);
	gtk_table_set_row_spacings(GTK_TABLE(top_grid), 6);
	gtk_table_set_col_spacings(GTK_TABLE(top_grid), 15);
	gtk_table_set_row_spacings(GTK_TABLE(middle_grid), 0);
	gtk_table_set_col_spacings(GTK_TABLE(middle_grid), 0);
	gtk_table_set_row_spacings(GTK_TABLE(bottom_grid), 6);
	gtk_table_set_col_spacings(GTK_TABLE(bottom_grid), 15);

	gtk_table_attach(GTK_TABLE(main_grid), top_grid, 0, 1, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults(GTK_TABLE(main_grid), middle_grid, 0, 1, 1, 2);
	gtk_table_attach(GTK_TABLE(main_grid), bottom_grid, 0, 1, 2, 3,GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);

	scrolled_tool_windows_window = gtk_scrolled_window_new(NULL,NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_tool_windows_window), plugin_data->tool_windows_tree.tree_view );
	gtk_table_attach_defaults(GTK_TABLE(middle_grid), scrolled_tool_windows_window, 0, 1, 1, 2);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_tool_windows_window), GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);

	scrolled_file_list_window = gtk_scrolled_window_new(NULL,NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_file_list_window), plugin_data->files_tree.tree_view );
	gtk_table_attach_defaults(GTK_TABLE(middle_grid), scrolled_file_list_window, 1, 2, 1, 2);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_file_list_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	/* Top labels */
	add_label(top_grid, "File:", 0, 1, 0, 1, GTK_FILL, NULL);
	plugin_data->label_short_name = add_label(top_grid, "", 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, NULL);
	plugin_data->label_file_type_desc = add_label(top_grid, "", 0, 1, 1, 2, GTK_FILL, NULL);
	plugin_data->label_file_title = add_label(top_grid, "", 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, NULL);

	/* Headers */
	add_label(middle_grid, "", 0, 1, 0, 1, GTK_FILL, "<b>Active tool windows</b>");
	add_label(middle_grid, "", 1, 2, 0, 1, GTK_FILL, "<b>Active files</b>");

	/* Bottom labels */
	add_label(bottom_grid, "Path:", 0, 1, 0, 1, GTK_FILL, NULL);
	plugin_data->label_file_name = add_label(bottom_grid, "", 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, NULL);
	plugin_data->label_real_path_desc = add_label(bottom_grid, "", 0, 1, 1, 2, GTK_FILL, NULL);
	plugin_data->label_real_path = add_label(bottom_grid, "", 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, NULL);

	gtk_window_set_decorated(GTK_WINDOW(plugin_data->main_window), FALSE);
	gtk_window_set_resizable(GTK_WINDOW(plugin_data->main_window), FALSE);
	gtk_widget_set_size_request(plugin_data->main_window, WINDOW_WIDTH, WINDOW_HEIGHT);
	gtk_window_set_position(GTK_WINDOW(plugin_data->main_window), GTK_WIN_POS_CENTER);
	g_signal_connect(plugin_data->main_window, "delete_event", G_CALLBACK(quit_ctr), plugin_data);
	g_signal_connect(plugin_data->main_window, "key-press-event", G_CALLBACK(on_key_press), plugin_data);
	g_signal_connect(plugin_data->main_window, "key-release-event", G_CALLBACK(on_key_release), plugin_data);

	gtk_container_add(GTK_CONTAINER(plugin_data->main_window), main_grid);
	gtk_widget_show_all(plugin_data->main_window);

	gtk_tree_selection_unselect_all(plugin_data->tool_windows_tree.selection);
	gtk_widget_grab_focus(plugin_data->files_tree.tree_view);

    if (gtk_tree_model_iter_n_children(plugin_data->files_tree.model, NULL) == 0)
    {
		/* Select first row in tool window tree */
		GtkTreePath *path = gtk_tree_path_new_from_indices(0, -1);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(plugin_data->tool_windows_tree.tree_view), path, NULL, FALSE);
		gtk_tree_path_free(path);
		gtk_widget_grab_focus(plugin_data->tool_windows_tree.tree_view);
    }
    else
    {
		/* Move one down because that was the previous used file */
		move_selection(plugin_data, DIRECTION_DOWN);
	}

	return 0;
}


/**********************************************************************/
static void keyboard_activate(G_GNUC_UNUSED guint key_id)
{
	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	launch_widget();
}


/**********************************************************************/
static gboolean ctr_init(GeanyPlugin *plugin, G_GNUC_UNUSED gpointer pdata)
{
	GeanyKeyGroup *key_group;

	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	key_group = plugin_set_key_group(plugin, "ctrl_tab", KB_COUNT, NULL);
	keybindings_set_item(key_group, KB_CTRL_TAB, keyboard_activate, 0, 0, "ctrl_tab", PLUGIN_NAME, NULL);

	document_activation_order_array = g_array_new(FALSE, FALSE, sizeof(guint));

	plugin_signal_connect(geany_plugin, NULL, "document-activate", TRUE, G_CALLBACK(on_document_activate), NULL);
	plugin_signal_connect(geany_plugin, NULL, "document-reload", TRUE, G_CALLBACK(on_document_activate), NULL);
	plugin_signal_connect(geany_plugin, NULL, "document-open", TRUE, G_CALLBACK(on_document_activate), NULL);

	return TRUE;
}


/**********************************************************************/
static void ctr_cleanup(G_GNUC_UNUSED GeanyPlugin *plugin, G_GNUC_UNUSED gpointer pdata)
{
	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	g_array_free(document_activation_order_array, TRUE);
}


/**********************************************************************/
G_MODULE_EXPORT
void geany_load_module(GeanyPlugin *plugin)
{
	D(log_debug("%s:%s", __FILE__, __FUNCTION__));

	geany_plugin = plugin;
	plugin->info->name = PLUGIN_NAME;
	plugin->info->description = PLUGIN_DESCRIPTION;
	plugin->info->version = PLUGIN_VERSION;
	plugin->info->author = PLUGIN_AUTHOR;
	plugin->funcs->init = ctr_init;
	plugin->funcs->cleanup = ctr_cleanup;
	GEANY_PLUGIN_REGISTER(plugin, 225);
}
